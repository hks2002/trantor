// Copyright 2016, Tao An.  All rights reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Tao An

#include "AresResolver.h"

#include "trantor/net/core/Channel.h"
#include <ares.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>  // inet_ntop
#include <netdb.h>
#include <netinet/in.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std::placeholders;

namespace {
/**
 * Calculate the total seconds from the given timeval structure.
 *
 * @param tv Pointer to the struct timeval containing seconds and microseconds.
 *
 * @return Total seconds as a double value.
 */
double getSeconds(struct timeval* tv) {
  if (tv)
    return double(tv->tv_sec) + double(tv->tv_usec) / 1000000.0;
  else
    return -1.0;
}

/**
 * A function that returns the type of socket based on the given type value.
 *
 * @param type The integer value representing the type of socket.
 *
 * @return The string representation of the socket type (UDP, TCP, or Unknown).
 */
const char* getSocketType(int type) {
  if (type == SOCK_DGRAM)
    return "UDP";
  else if (type == SOCK_STREAM)
    return "TCP";
  else
    return "Unknown";
}
}  // namespace

namespace trantor {
/**
 * @brief Return the global DNS cache.
 */
AresResolver::DnsCache& AresResolver::globalCache() {
  static DnsCache dnsCache_;
  return dnsCache_;
}

/**
 * @brief Return the global mutex.
 */
std::mutex& AresResolver::globalMutex() {
  static std::mutex mutex_;
  return mutex_;
}

/**
 * @brief Return event thread loop.
 */
EventLoop* AresResolver::getLoop() {
  static EventLoopThread loopThread;
  loopThread.run();
  return loopThread.getLoop();
}

AresResolver::LibraryInitializer::LibraryInitializer() {
  ares_library_init(ARES_LIB_INIT_ALL);

  hints_              = new ares_addrinfo_hints;
  hints_->ai_flags    = 0;
  hints_->ai_family   = AF_INET;
  hints_->ai_socktype = 0;
  hints_->ai_protocol = 0;
}
AresResolver::LibraryInitializer::~LibraryInitializer() {
  ares_library_cleanup();
  delete hints_;
}

AresResolver::LibraryInitializer AresResolver::libraryInitializer_;

AresResolver::AresResolver(EventLoop* loop, size_t timeout) : loop_(loop), timeout_(timeout) {
  if (!loop) {
    loop_ = getLoop();
  }
  loopValid_ = std::make_shared<bool>(true);
  loop_->runOnQuit([loopValid = loopValid_]() {
    *loopValid = false;
  });
}

AresResolver::~AresResolver() {
  if (aresChannel_) ares_destroy(aresChannel_);
}

void AresResolver::onTimer() {
  assert(timerActive_ == true);
  ares_process_fd(aresChannel_, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
  struct timeval  tv;
  struct timeval* tvp     = ares_timeout(aresChannel_, NULL, &tv);
  double          timeout = getSeconds(tvp);

  if (timeout < 0) {
    timerActive_ = false;
  } else {
    loop_->runAfter(timeout, std::bind(&AresResolver::onTimer, shared_from_this()));
  }
}

void AresResolver::onRead(int sockfd) {
  ares_process_fd(aresChannel_, sockfd, ARES_SOCKET_BAD);
}

void AresResolver::onSockCreate(int sockfd, int type) {
  (void)type;
  loop_->assertInLoopThread();
  assert(channels_.find(sockfd) == channels_.end());
  Channel* channel = new Channel(loop_, sockfd);
  channel->setReadCallback(std::bind(&AresResolver::onRead, this, sockfd));
  channel->enableReading();
  channels_[sockfd].reset(channel);
}

void AresResolver::onSockStateChange(int sockfd, bool read, bool write) {
  (void)write;
  if (read) {
    // update
    // if (write) { } else { }
  } else if (*loopValid_) {
    loop_->assertInLoopThread();
    ChannelList::iterator it = channels_.find(sockfd);
    assert(it != channels_.end());
    // remove
    it->second->disableAll();
    it->second->remove();
    channels_.erase(it);
  }
}

void AresResolver::onQueryResult(int                            status,
                                 struct ares_addrinfo*          result,
                                 const std::string&             hostname,
                                 const ResolverResultsCallback& callback) {
  LOG_TRACE << "onQueryResult " << status;
  auto inets_ptr = std::make_shared<std::vector<trantor::InetAddress>>();
  if (result) {
    auto pptr = (struct ares_addrinfo_node*)result->nodes;
    for (; pptr != NULL; pptr = pptr->ai_next) {
      trantor::InetAddress inet;
      if (pptr->ai_family == AF_INET) {
        struct sockaddr_in* addr4 = (struct sockaddr_in*)pptr->ai_addr;
        inets_ptr->emplace_back(trantor::InetAddress{*addr4});
      } else if (pptr->ai_family == AF_INET6) {
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*)pptr->ai_addr;
        inets_ptr->emplace_back(trantor::InetAddress{*addr6});
      } else {
        // TODO: Handle unknown family?
      }
    }
  }
  if (inets_ptr->empty()) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port   = 0;
    InetAddress inet(addr);
    inets_ptr->emplace_back(std::move(inet));
  }
  {
    std::lock_guard<std::mutex> lock(globalMutex());
    auto&                       addrItem = globalCache()[hostname];
    addrItem.first                       = inets_ptr;
    addrItem.second                      = trantor::Date::date();
  }
  callback(*inets_ptr);
}

#ifdef _WIN32
int AresResolver::ares_sock_createcallback_(SOCKET sockfd, int type, void* data) {
  LOG_TRACE << "sockfd=" << sockfd << " type=" << getSocketType(type);
  static_cast<AresResolver*>(data)->onSockCreate(static_cast<int>(sockfd), type);
  return 0;
}

void AresResolver::ares_sock_statecallback_(void* data, SOCKET sockfd, int read, int write) {
  LOG_TRACE << "sockfd=" << sockfd << " read=" << read << " write=" << write;
  static_cast<AresResolver*>(data)->onSockStateChange(static_cast<int>(sockfd), read, write);
}

#else
int AresResolver::ares_sock_createcallback_(int sockfd, int type, void* data) LOG_TRACE
  << "sockfd=" << sockfd << " type=" << getSocketType(type);
static_cast<AresResolver*>(data)->onSockCreate(sockfd, type);
return 0;
}

void AresResolver::ares_sock_statecallback_(void* data, int sockfd, int read, int write) {
  LOG_TRACE << "sockfd=" << sockfd << " read=" << read << " write=" << write;
  static_cast<AresResolver*>(data)->onSockStateChange(sockfd, read, write);
}

#endif

void AresResolver::ares_hostcallback_(void* data, int status, int timeouts, struct ares_addrinfo* hostent) {
  (void)timeouts;
  QueryData* query = static_cast<QueryData*>(data);

  query->owner_->onQueryResult(status, hostent, query->hostname_, query->callback_);
  delete query;
}

/**
 * @brief Initialize the AresResolver channel and sockets callbacks.
 */
void AresResolver::init() {
  if (!aresChannel_) {
    struct ares_options options;
    int                 optmask  = ARES_OPT_FLAGS;
    options.flags                = ARES_FLAG_NOCHECKRESP;
    options.flags               |= ARES_FLAG_STAYOPEN;
    options.flags               |= ARES_FLAG_IGNTC;  // UDP only
    optmask                     |= ARES_OPT_SOCK_STATE_CB;
    options.sock_state_cb        = &AresResolver::ares_sock_statecallback_;
    options.sock_state_cb_data   = this;
    optmask                     |= ARES_OPT_TIMEOUT;
    options.timeout              = 2;
    // optmask |= ARES_OPT_LOOKUPS;
    // options.lookups = lookups;

    int status = ares_init_options(&aresChannel_, &options, optmask);
    if (status != ARES_SUCCESS) {
      assert(0);
    }
    ares_set_socket_callback(aresChannel_, &AresResolver::ares_sock_createcallback_, this);
  }
}

/**
 * @brief Create a new DNS resolver.
 *
 * @param loop The event loop in which the DNS resolver runs.
 * @param timeout The timeout in seconds for DNS.
 * @return std::shared_ptr<Resolver>
 */
std::shared_ptr<Resolver> Resolver::newResolver(trantor::EventLoop* loop, size_t timeout) {
  return std::make_shared<AresResolver>(loop, timeout);
}

/**
 * Resolve the hostname asynchronously in the event loop.
 *
 * @param hostname The hostname to resolve
 * @param cb The callback function to handle the resolver results
 */
void AresResolver::resolveInLoop(const std::string& hostname, const ResolverResultsCallback& cb) {
  loop_->assertInLoopThread();
#ifdef _WIN32
  if (hostname == "localhost") {
    const static std::vector<trantor::InetAddress> localhost_{
      trantor::InetAddress{"127.0.0.1", 0}
    };
    cb(localhost_);
    return;
  }
#endif

  init();
  QueryData* queryData = new QueryData(this, cb, hostname);
  ares_getaddrinfo(aresChannel_,
                   hostname.c_str(),
                   NULL,
                   libraryInitializer_.hints_,
                   &AresResolver::ares_hostcallback_,
                   queryData);
  struct timeval  tv;
  struct timeval* tvp     = ares_timeout(aresChannel_, NULL, &tv);
  double          timeout = getSeconds(tvp);
  if (!timerActive_ && timeout >= 0.0) {
    loop_->runAfter(timeout, std::bind(&AresResolver::onTimer, shared_from_this()));
    timerActive_ = true;
  }
  return;
}

/**
 * @brief Resolve an address asynchronously.
 * @note Cache results is used, if the result is not expired.
 */
void AresResolver::resolve(const std::string& hostname, const Callback& cb) {
  {
    std::lock_guard<std::mutex> guard(globalMutex());
    auto                        iter = globalCache().find(hostname);
    if (iter != globalCache().end()) {
      auto& cachedAddr = iter->second;
      if (timeout_ == 0 || cachedAddr.second.after(static_cast<double>(timeout_)) > trantor::Date::date()) {
        cb((*cachedAddr.first)[0]);
        return;
      }
    }
  }

  if (loop_->isInLoopThread()) {
    resolveInLoop(hostname, [cb](const std::vector<trantor::InetAddress>& inets) {
      cb(inets[0]);
    });
  } else {
    loop_->queueInLoop([thisPtr = shared_from_this(), hostname, cb]() {
      thisPtr->resolveInLoop(hostname, [cb](const std::vector<trantor::InetAddress>& inets) {
        cb(inets[0]);
      });
    });
  }
}

/**
 * @brief Resolve an address asynchronously.
 * @note Cache results is used, if the result is not expired.
 */
void AresResolver::resolve(const std::string& hostname, const ResolverResultsCallback& cb) {
  {
    std::lock_guard<std::mutex> lock(globalMutex());
    auto                        iter = globalCache().find(hostname);
    if (iter != globalCache().end()) {
      auto& cachedAddr = iter->second;
      if (timeout_ == 0 || cachedAddr.second.after(static_cast<double>(timeout_)) > trantor::Date::date()) {
        cb(*cachedAddr.first);
        return;
      }
    }
  }

  if (loop_->isInLoopThread()) {
    resolveInLoop(hostname, cb);
  } else {
    loop_->queueInLoop([thisPtr = shared_from_this(), hostname, cb]() {
      thisPtr->resolveInLoop(hostname, cb);
    });
  }
}

bool Resolver::isCAresUsed() {
  return true;
}

}  // namespace trantor
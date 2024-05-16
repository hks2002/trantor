#include "NormalResolver.h"

#include "trantor/logger/Logger.h"
#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>  // memset
#include <sys/socket.h>
#endif

namespace trantor {
/**
 * @brief Return the global dns cache.
 *
 */
static DnsCache &NormalResolver::globalCache() {
  static DnsCache dnsCache_;
  return dnsCache_;
}

/**
 * @brief Return the global mutex.
 *
 */
static std::mutex &NormalResolver::globalMutex() {
  static std::mutex mutex_;
  return mutex_;
}

/**
 * @brief Return a global concurrent task queue.
 * @note The minimum value is 8, the maximum is the number of cores.
 */
static trantor::ConcurrentTaskQueue &NormalResolver::concurrentTaskQueue() {
  static trantor::ConcurrentTaskQueue queue{
    std::thread::hardware_concurrency() < 8 ? 8 : std::thread::hardware_concurrency(),
    "Dns Queue"};
  return queue;
}

/**
 * @brief Create a new resolver.
 *
 */
std::shared_ptr<Resolver> Resolver::newResolver(trantor::EventLoop *eventLoop, size_t timeout) {
  return std::make_shared<NormalResolver>(timeout);
}

/**
 * @brief Resolve an address asynchronously.
 * @note Cache results is used, if the result is not expired.
 */
void NormalResolver::resolve(const std::string &hostname, const Callback &callback) {
  {
    std::lock_guard<std::mutex> guard(globalMutex());
    auto                        iter = globalCache().find(hostname);
    if (iter != globalCache().end()) {
      auto &cachedAddr = iter->second;
      if (timeout_ == 0 || cachedAddr.second.after(static_cast<double>(timeout_)) > trantor::Date::date()) {
        callback(cachedAddr.first);
        return;
      }
    }
  }

  concurrentTaskQueue().runTaskInQueue([thisPtr = shared_from_this(), callback, hostname]() {
    {
      std::lock_guard<std::mutex> guard(thisPtr->globalMutex());
      auto                        iter = thisPtr->globalCache().find(hostname);
      if (iter != thisPtr->globalCache().end()) {
        auto &cachedAddr = iter->second;
        if (thisPtr->timeout_ == 0 ||
            cachedAddr.second.after(static_cast<double>(thisPtr->timeout_)) > trantor::Date::date()) {
          callback(cachedAddr.first);
          return;
        }
      }
    }

    struct addrinfo hints, *res = nullptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;
    auto error        = getaddrinfo(hostname.data(), nullptr, &hints, &res);
    if (error != 0 || res == nullptr) {
      LOG_SYSERR << "InetAddress::resolve";
      if (res != nullptr) {
        freeaddrinfo(res);
      }
      callback(InetAddress{});
      return;
    }

    InetAddress inet;
    if (res->ai_family == AF_INET) {
      struct sockaddr_in addr;
      memset(&addr, 0, sizeof addr);
      addr = *reinterpret_cast<struct sockaddr_in *>(res->ai_addr);
      inet = InetAddress(addr);
    } else if (res->ai_family == AF_INET6) {
      struct sockaddr_in6 addr;
      memset(&addr, 0, sizeof addr);
      addr = *reinterpret_cast<struct sockaddr_in6 *>(res->ai_addr);
      inet = InetAddress(addr);
    }

    freeaddrinfo(res);
    callback(inet);
    {
      std::lock_guard<std::mutex> guard(thisPtr->globalMutex());
      auto                       &addrItem = thisPtr->globalCache()[hostname];
      addrItem.first                       = inet;
      addrItem.second                      = trantor::Date::date();
    }
    return;
  });
}

/**
 * @brief Resolve an address asynchronously.
 * @note Cache results is used, if the result is not expired.
 */
void Resolver::resolve(const std::string &hostname, const ResolverResultsCallback &callback) {
  resolve(hostname, [callback](const trantor::InetAddress &inet) {
    callback(std::vector<trantor::InetAddress>{inet});
  });
}

/**
 * @brief Is c-ares used.
 * @return false
 */
bool Resolver::isCAresUsed() {
  return false;
}

}  // namespace trantor
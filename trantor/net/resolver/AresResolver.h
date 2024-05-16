// Copyright 2016, Tao An.  All rights reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Tao An

#ifndef TRANTOR_ARES_RESOLVER_H
#define TRANTOR_ARES_RESOLVER_H

#include "trantor/NonCopyable.h"
#include "trantor/net/core/EventLoopThread.h"
#include "trantor/net/resolver/Resolver.h"
#include <map>
#include <memory>
#include <string.h>

extern "C" {
struct ares_addrinfo;
struct ares_channeldata;
struct ares_addrinfo_hints;
using ares_channel = struct ares_channeldata*;
}

namespace trantor {
class AresResolver : public Resolver, public NonCopyable, public std::enable_shared_from_this<AresResolver> {
public:
  using ChannelList = std::map<int, std::unique_ptr<trantor::Channel>>;
  using DnsCache =
    std::unordered_map<std::string, std::pair<std::shared_ptr<std::vector<trantor::InetAddress>>, trantor::Date>>;

  AresResolver(trantor::EventLoop* loop, size_t timeout);
  ~AresResolver();

  void resolve(const std::string& hostname, const Callback& cb) override;
  void resolve(const std::string& hostname, const ResolverResultsCallback& cb) override;

private:
  struct QueryData {
    AresResolver*           owner_;
    ResolverResultsCallback callback_;
    std::string             hostname_;
    QueryData(AresResolver* o, const ResolverResultsCallback& cb, const std::string& hostname)
      : owner_(o), callback_(cb), hostname_(hostname) {}
  };

  struct LibraryInitializer {
    LibraryInitializer();
    ~LibraryInitializer();
    ares_addrinfo_hints* hints_;
  };

  const size_t              timeout_{60};
  trantor::EventLoop*       loop_;
  std::shared_ptr<bool>     loopValid_;
  bool                      timerActive_{false};
  ares_channel              aresChannel_{nullptr};
  ChannelList               channels_;

  static LibraryInitializer libraryInitializer_;

  void                      onTimer();
  void                      onRead(int sockfd);
  void                      onSockCreate(int sockfd, int type);
  void                      onSockStateChange(int sockfd, bool read, bool write);
  void                      onQueryResult(int                            status,
                                          struct ares_addrinfo*          result,
                                          const std::string&             hostname,
                                          const ResolverResultsCallback& callback);

  static DnsCache&          globalCache();
  static std::mutex&        globalMutex();
  static EventLoop*         getLoop();

#ifdef _WIN32
  static int  ares_sock_createcallback_(SOCKET sockfd, int type, void* data);
  static void ares_sock_statecallback_(void* data, SOCKET sockfd, int read, int write);
#else
  static int  ares_sock_createcallback_(int sockfd, int type, void* data);
  static void ares_sock_statecallback_(void* data, int sockfd, int read, int write);
#endif
  static void ares_hostcallback_(void* data, int status, int timeouts, struct ares_addrinfo* hostent);

  void        init();
  void        resolveInLoop(const std::string& hostname, const ResolverResultsCallback& cb);
};
}  // namespace trantor

#endif  // TRANTOR_ARES_RESOLVER_H
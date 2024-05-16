// Copyright 2016, Tao An.  All rights reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Tao An

#ifndef TRANTOR_NORMAL_RESOLVER_H
#define TRANTOR_NORMAL_RESOLVER_H

#include "Resolver.h"
#include "trantor/NonCopyable.h"
#include "trantor/net/core/ConcurrentTaskQueue.h"
#include "trantor/net/resolver/Resolver.h"
#include <memory>
#include <thread>
#include <trantor/NonCopyable.h>
#include <vector>

namespace trantor {
constexpr size_t kResolveBufferLength{16 * 1024};
class NormalResolver : public Resolver, public NonCopyable, public std::enable_shared_from_this<NormalResolver> {
public:
  virtual void resolve(const std::string& hostname, const Callback& callback) override;
  virtual void resolve(const std::string& hostname, const ResolverResultsCallback& callback) override {
    resolve(hostname, [callback](const trantor::InetAddress& inet) {
      callback(std::vector<trantor::InetAddress>{inet});
    });
  }
  explicit NormalResolver(size_t timeout) : timeout_(timeout), resolveBuffer_(kResolveBufferLength) {}
  virtual ~NormalResolver() {}

private:
  static std::unordered_map<std::string, std::pair<trantor::InetAddress, trantor::Date>>& globalCache() {
    static std::unordered_map<std::string, std::pair<trantor::InetAddress, trantor::Date>> dnsCache_;
    return dnsCache_;
  }
  static std::mutex& globalMutex() {
    static std::mutex mutex_;
    return mutex_;
  }
  static trantor::ConcurrentTaskQueue& concurrentTaskQueue() {
    static trantor::ConcurrentTaskQueue queue(
      std::thread::hardware_concurrency() < 8 ? 8 : std::thread::hardware_concurrency(),
      "Dns Queue");
    return queue;
  }
  const size_t      timeout_;
  std::vector<char> resolveBuffer_;
};
}  // namespace trantor
#endif  // TRANTOR_NORMAL_RESOLVER_H

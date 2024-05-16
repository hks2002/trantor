// Copyright 2016, Tao An.  All rights reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Tao An

#ifndef TRANTOR_NORMAL_RESOLVER_H
#define TRANTOR_NORMAL_RESOLVER_H

#include "trantor/NonCopyable.h"
#include "trantor/net/core/ConcurrentTaskQueue.h"
#include "trantor/net/resolver/Resolver.h"
#include <memory>
#include <thread>
#include <vector>

namespace trantor {
constexpr size_t kResolveBufferLength{16 * 1024};
/**
 * @brief Normal resolver.
 *
 */
class NormalResolver : public Resolver, public NonCopyable, public std::enable_shared_from_this<NormalResolver> {
public:
  using DnsCache = std::unordered_map<std::string, std::pair<trantor::InetAddress, trantor::Date>>;

  explicit NormalResolver(size_t timeout) : timeout_(timeout), resolveBuffer_(kResolveBufferLength) {}
  ~NormalResolver() {}

  void resolve(const std::string& hostname, const Callback& callback) override;
  void resolve(const std::string& hostname, const ResolverResultsCallback& callback) override;

private:
  const size_t      timeout_{60};
  std::vector<char> resolveBuffer_;

  // static functions
  static DnsCache&                     globalCache();
  static std::mutex&                   globalMutex();
  static trantor::ConcurrentTaskQueue& concurrentTaskQueue();
};
}  // namespace trantor

#endif  // TRANTOR_NORMAL_RESOLVER_H
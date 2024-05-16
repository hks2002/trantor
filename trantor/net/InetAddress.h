// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

// Taken from Muduo and modified
// Copyright 2016, Tao An.  All rights reserved.
// https://github.com/an-tao/trantor
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Tao An

#ifndef TRANTOR_INET_ADDRESS_H
#define TRANTOR_INET_ADDRESS_H

#include "trantor/exports.h"
#include "trantor/utils/Date.h"

#ifdef _WIN32
#include <ws2tcpip.h>
using sa_family_t = unsigned short;
using in_addr_t   = uint32_t;
using uint16_t    = unsigned short;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#include <mutex>
#include <string>
#include <unordered_map>

namespace trantor {
/**
 * @brief Wrapper of sockaddr_in. This is an POD interface class.
 *
 */
class TRANTOR_EXPORT InetAddress {
public:
  InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);
  InetAddress(const std::string &ip, uint16_t port, bool ipv6 = false);
  explicit InetAddress(const struct sockaddr_in &addr);
  explicit InetAddress(const struct sockaddr_in6 &addr);

  bool                   isIpV6() const;
  bool                   isIntranetIp() const;
  bool                   isLoopbackIp() const;
  bool                   isUnspecified() const;

  uint32_t               ipNetEndian() const;
  const uint32_t        *ip6NetEndian() const;
  uint16_t               portNetEndian() const;
  void                   setPortNetEndian(uint16_t port);

  sa_family_t            family() const;
  std::string            toIp() const;
  uint16_t               toPort() const;
  std::string            toIpPort() const;
  std::string            toIpNetEndian() const;
  std::string            toIpPortNetEndian() const;

  const struct sockaddr *getSockAddr() const;
  void                   setSockAddrInet6(const struct sockaddr_in6 &addr6);

private:
  union {
    struct sockaddr_in  addr_;
    struct sockaddr_in6 addr6_;
  };
  bool isIpV6_{false};
  bool isUnspecified_{true};
};

}  // namespace trantor
#endif  // TRANTOR_INET_ADDRESS_H

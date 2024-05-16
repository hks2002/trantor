/**
 *
 *  callbacks.h
 *  An Tao
 *
 *  Public header file in trantor lib.
 *
 *  Copyright 2018, An Tao.  All rights reserved.
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the License file.
 *
 *
 */

#ifndef TRANTOR_CALLBACKS_H
#define TRANTOR_CALLBACKS_H

#include <functional>
#include <memory>
namespace trantor {
enum class SSLError { kSSLHandshakeError, kSSLInvalidCertificate, kSSLProtocolError };

// the data has been read to (buf, len)
class TcpConnection;
class MsgBuffer;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
// tcp server and connection callback
using RecvMessageCallback     = std::function<void(const TcpConnectionPtr &, MsgBuffer *)>;
using ConnectionErrorCallback = std::function<void()>;
using ConnectionCallback      = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback           = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback   = std::function<void(const TcpConnectionPtr &)>;
using HighWaterMarkCallback   = std::function<void(const TcpConnectionPtr &, const size_t)>;
using SSLErrorCallback        = std::function<void(SSLError)>;
using SockOptCallback         = std::function<void(int)>;

// timer callback
using TimerCallback = std::function<void()>;
}  // namespace trantor
#endif  // TRANTOR_CALLBACKS_H
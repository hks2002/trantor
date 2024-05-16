/**
 *
 *  Acceptor.h
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

#ifndef TRANTOR_ACCEPTOR_H
#define TRANTOR_ACCEPTOR_H

#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
#include "trantor/NonCopyable.h"
#include "trantor/net/core/Channel.h"
#include <functional>

namespace trantor {
using NewConnectionCallback   = std::function<void(int fd, const InetAddress &)>;
using AcceptorSockOptCallback = std::function<void(int)>;

/**
 * @brief This class is used to accept connections on the server socket.
 *
 */
class Acceptor : NonCopyable {
public:
  Acceptor(EventLoop *loop, const InetAddress &addr, bool reUseAddr = true, bool reUsePort = true);
  ~Acceptor();
  const InetAddress &addr() const {
    return addr_;
  }
  void setNewConnectionCallback(const NewConnectionCallback &cb) {
    newConnectionCallback_ = cb;
  };
  void listen();

  void setBeforeListenSockOptCallback(AcceptorSockOptCallback cb) {
    beforeListenSetSockOptCallback_ = std::move(cb);
  }

  void setAfterAcceptSockOptCallback(AcceptorSockOptCallback cb) {
    afterAcceptSetSockOptCallback_ = std::move(cb);
  }

protected:
#ifndef _WIN32
  int idleFd_;
#endif
  Socket                  sock_;
  InetAddress             addr_;
  EventLoop              *loop_;
  NewConnectionCallback   newConnectionCallback_;
  Channel                 acceptChannel_;
  void                    readCallback();
  AcceptorSockOptCallback beforeListenSetSockOptCallback_;
  AcceptorSockOptCallback afterAcceptSetSockOptCallback_;
};

}  // namespace trantor

#endif  // TRANTOR_ACCEPTOR_H

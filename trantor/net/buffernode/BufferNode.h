/**
 *
 *  @file BufferNode.h
 *  @author An Tao
 *
 *  Public header file in trantor lib.
 *
 *  Copyright 2018, An Tao.  All rights reserved.
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the License file.
 *
 *
 */
#ifndef TRANTOR_BUFFERNODE_H
#define TRANTOR_BUFFERNODE_H

#ifdef _WIN32
#include <stdio.h>
#endif
#include "trantor/NonCopyable.h"
#include "trantor/logger/Logger.h"
#include "trantor/net/MsgBuffer.h"
#include <functional>
#include <memory>
#include <string>

namespace trantor {
class BufferNode;
using BufferNodePtr  = std::shared_ptr<BufferNode>;
using StreamCallback = std::function<std::size_t(char *, std::size_t)>;

class BufferNode : public NonCopyable {
public:
  static BufferNodePtr newMemBufferNode();
  static BufferNodePtr newStreamBufferNode(StreamCallback &&cb);
  static BufferNodePtr newAsyncStreamBufferNode();
#ifdef _WIN32
  static BufferNodePtr newFileBufferNode(const wchar_t *fileName, long long offset, long long length);
#else
  static BufferNodePtr newFileBufferNode(const char *fileName, long long offset, long long length);
#endif

  virtual ~BufferNode() = default;
  virtual bool isFile() const {
    return false;
  }
  virtual bool isStream() const {
    return false;
  }
  virtual bool isAsync() const {
    return false;
  }
  virtual bool available() const {
    return true;
  }
  void done() {
    isDone_ = true;
  }
  virtual int getFd() const {
    LOG_FATAL << "Not a file buffer node";
    return -1;
  }
  virtual void append(const char *, size_t) {
    LOG_FATAL << "Not a memory buffer node";
  }
  virtual void      getData(const char *&data, size_t &len) = 0;
  virtual void      retrieve(size_t len)                    = 0;
  virtual long long remainingBytes() const                  = 0;

protected:
  bool isDone_{false};
};

}  // namespace trantor

#endif  // TRANTOR_BUFFERNODE_H

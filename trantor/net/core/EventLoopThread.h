/**
 *
 *  @file EventLoopThread.h
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

#ifndef TRANTOR_EVENT_LOOP_THREAD_H
#define TRANTOR_EVENT_LOOP_THREAD_H

#include "EventLoop.h"
#include "trantor/NonCopyable.h"
#include "trantor/exports.h"
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <string_view>
#include <thread>

namespace trantor {
/**
 * @brief This class represents an event loop thread.
 *
 */
class TRANTOR_EXPORT EventLoopThread : NonCopyable {
public:
  explicit EventLoopThread(std::string_view threadName = "EventLoopThread");
  ~EventLoopThread();

  EventLoop *getLoop() const;
  void       run();
  void       wait();

private:
  std::string loopThreadName_;
  std::thread thread_;

  // With C++20, use std::atomic<std::shared_ptr<EventLoop>>
  std::shared_ptr<EventLoop>               loop_;
  std::mutex                               loopMutex_;

  std::promise<std::shared_ptr<EventLoop>> promiseForLoopPointer_;
  std::promise<int>                        promiseForRun_;
  std::promise<int>                        promiseForLoop_;
  std::once_flag                           once_;

  void                                     loopFuncs();
};

}  // namespace trantor

#endif  // TRANTOR_EVENT_LOOP_THREAD_H

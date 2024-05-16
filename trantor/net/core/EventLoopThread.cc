/**
 *
 *  @file EventLoopThread.cc
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

#include "EventLoopThread.h"

#include "trantor/logger/Logger.h"
#include <string_view>
#ifdef __linux__
#include <sys/prctl.h>
#endif

namespace trantor {
EventLoopThread::EventLoopThread(std::string_view threadName)
  : loop_(nullptr), loopThreadName_(threadName), thread_([this]() {
      loopFuncs();
    }) {
  auto f = promiseForLoopPointer_.get_future();
  loop_  = f.get();
}

EventLoopThread::~EventLoopThread() {
  run();
  std::shared_ptr<EventLoop> loop;
  {
    std::unique_lock<std::mutex> lk(loopMutex_);
    loop = loop_;
  }
  if (loop) {
    loop->quit();
  }
  if (thread_.joinable()) {
    thread_.join();
  }
}

/**
 * @brief The function performs a series of operations in the event loop thread.
 *
 */
void EventLoopThread::loopFuncs() {
#ifdef __linux__
  ::prctl(PR_SET_NAME, loopThreadName_.c_str());
#endif
  thread_local static std::shared_ptr<EventLoop> loop = std::make_shared<EventLoop>();
  loop->queueInLoop([this]() {
    promiseForLoop_.set_value(1);
  });
  promiseForLoopPointer_.set_value(loop);
  auto f = promiseForRun_.get_future();
  (void)f.get();
  loop->loop();
  {
    std::unique_lock<std::mutex> lk(loopMutex_);
    loop_ = nullptr;
  }
}

/**
 * @brief Get the pointer of the event loop of the thread.
 *
 * @return EventLoop*
 */
EventLoop *EventLoopThread::getLoop() const {
  return loop_.get();
}

/**
 * @brief Run the event loop of the thread. This method doesn't block the
 * current thread.
 *
 */
void EventLoopThread::run() {
  std::call_once(once_, [this]() {
    auto f = promiseForLoop_.get_future();
    promiseForRun_.set_value(1);
    // Make sure the event loop loops before returning.
    (void)f.get();
  });
}

/**
 * @brief Wait for the event loop to exit.
 * @note This method blocks the current thread until the event loop exits.
 */
void EventLoopThread::wait() {
  thread_.join();
}

}  // namespace trantor
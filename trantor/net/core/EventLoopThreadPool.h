/**
 *
 *  @file EventLoopThreadPool.h
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

#ifndef TRANTOR_EVENT_LOOP_THREAD_POOL_H
#define TRANTOR_EVENT_LOOP_THREAD_POOL_H

#include "EventLoopThread.h"
#include "trantor/exports.h"
#include <atomic>
#include <memory>
#include <string_view>
#include <vector>

namespace trantor {
/**
 * @brief This class represents a pool of EventLoopThread objects
 *
 */
class TRANTOR_EXPORT EventLoopThreadPool : NonCopyable {
public:
  EventLoopThreadPool() = delete;
  EventLoopThreadPool(size_t threadNum, std::string_view name = "EventLoopThreadPool");

  size_t                   size();
  void                     start();
  void                     wait();

  EventLoop               *getNextLoop();
  EventLoop               *getLoop(size_t id);
  std::vector<EventLoop *> getLoops() const;

private:
  std::vector<std::shared_ptr<EventLoopThread>> loopThreadVector_;
  std::atomic<size_t>                           loopIndex_{0};
};

}  // namespace trantor
#endif  // TRANTOR_EVENT_LOOP_THREAD_POOL_H

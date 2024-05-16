/**
 *
 *  @file EventLoopThreadPool.cc
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

#include "EventLoopThreadPool.h"
namespace trantor {

/**
 * @brief Construct a new event loop thread pool instance.
 *
 * @param threadNum The number of threads
 * @param name The name of the EventLoopThreadPool object.
 */
EventLoopThreadPool::EventLoopThreadPool(size_t threadNum, std::string_view name) : loopIndex_(0) {
  for (size_t i = 0; i < threadNum; ++i) {
    loopThreadVector_.emplace_back(std::make_shared<EventLoopThread>(name));
  }
}

/**
 * @brief Return the size of the event loop thread pool.
 *
 * @return size_t
 */
size_t EventLoopThreadPool::size() {
  return loopThreadVector_.size();
}

/**
 * @brief Run all event loops in the pool.
 * @note This function doesn't block the current thread.
 */
void EventLoopThreadPool::start() {
  for (unsigned int i = 0; i < loopThreadVector_.size(); ++i) {
    loopThreadVector_[i]->run();
  }
}
// void EventLoopThreadPool::stop(){
//    for(unsigned int i=0;i<loopThreadVector_.size();i++)
//    {
//        loopThreadVector_[i].stop();
//    }
//}

/**
 * @brief Wait for all event loops in the pool to quit.
 *
 * @note This function blocks the current thread.
 */
void EventLoopThreadPool::wait() {
  for (unsigned int i = 0; i < loopThreadVector_.size(); ++i) {
    loopThreadVector_[i]->wait();
  }
}

/**
 * @brief Get the next event loop in the pool.
 *
 * @return EventLoop*
 */
EventLoop *EventLoopThreadPool::getNextLoop() {
  if (loopThreadVector_.size() > 0) {
    size_t     index = loopIndex_.fetch_add(1, std::memory_order_relaxed);
    EventLoop *loop  = loopThreadVector_[index % loopThreadVector_.size()]->getLoop();
    return loop;
  }
  return nullptr;
}

/**
 * @brief Get the event loop in the `id` position in the pool.
 *
 * @param id The id of the first event loop is zero. If the id >= the number
 * of event loops, nullptr is returned.
 * @return EventLoop*
 */
EventLoop *EventLoopThreadPool::getLoop(size_t id) {
  if (id < loopThreadVector_.size()) return loopThreadVector_[id]->getLoop();
  return nullptr;
}

/**
 * @brief Get all event loops in the pool.
 *
 * @return std::vector<EventLoop *>
 */
std::vector<EventLoop *> EventLoopThreadPool::getLoops() const {
  std::vector<EventLoop *> ret;
  for (auto &loopThread : loopThreadVector_) {
    ret.push_back(loopThread->getLoop());
  }
  return ret;
}

}  // namespace trantor
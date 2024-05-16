/**
 *
 *  SerialTaskQueue.cc
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

#include "SerialTaskQueue.h"

#include "trantor/logger/Logger.h"

#ifdef __linux__
#include <sys/prctl.h>
#endif

namespace trantor {
/**
 * @brief Construct a new serial task queue instance.
 *
 * @param name
 */
SerialTaskQueue::SerialTaskQueue(const std::string &name)
  : queueName_(name.empty() ? "SerialTaskQueue" : name), loopThread_(queueName_) {
  loopThread_.run();
}

SerialTaskQueue::~SerialTaskQueue() {
  if (!stop_) stop();
  LOG_TRACE << "destruct SerialTaskQueue('" << queueName_ << "')";
}

/**
 * @brief Run a task in the queue.
 *
 * @param task
 */
void SerialTaskQueue::runTaskInQueue(const std::function<void()> &task) {
  loopThread_.getLoop()->runInLoop(task);
}

/**
 * @brief Run a task in the queue.
 *
 * @param task
 */
void SerialTaskQueue::runTaskInQueue(std::function<void()> &&task) {
  loopThread_.getLoop()->runInLoop(std::move(task));
}

/**
 * @brief Get the name of the queue.
 *
 * @return std::string
 */
std::string SerialTaskQueue::getName() const {
  return queueName_;
};

/**
 * @brief Get the number of tasks in the queue.
 *
 * @return size_t
 */
size_t SerialTaskQueue::getTaskCount() {
  // TODO:
  return 0;
}

/**
 * @brief Check whether a task is running in the queue.
 *
 * @return true
 * @return false
 */
bool SerialTaskQueue::isRunningTask() {
  return loopThread_.getLoop() ? loopThread_.getLoop()->isCallingFunctions() : false;
}

/**
 * @brief Wait until all tasks in the queue are finished.
 *
 */
void SerialTaskQueue::waitAllTasksFinished() {
  syncTaskInQueue([]() {

  });
}

/**
 * @brief Stop the queue.
 *
 */
void SerialTaskQueue::stop() {
  stop_ = true;
  loopThread_.getLoop()->quit();
  loopThread_.wait();
}

}  // namespace trantor

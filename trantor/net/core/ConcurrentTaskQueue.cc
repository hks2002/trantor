/**
 *
 *  ConcurrentTaskQueue.cc
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

#include "ConcurrentTaskQueue.h"

#include "trantor/logger/Logger.h"
#include <assert.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif

namespace trantor {

/**
 * @brief Responsible for executing tasks in a concurrent task queue.
 *
 * @param queueNum The index of the queue.
 *
 * @throws None
 */
void ConcurrentTaskQueue::queueFunc(int queueNum) {
  char tmpName[32];
  snprintf(tmpName, sizeof(tmpName), "%s%d", queueName_.c_str(), queueNum);
#ifdef __linux__
  ::prctl(PR_SET_NAME, tmpName);
#endif
  while (!stop_) {
    std::function<void()> r;
    {
      std::unique_lock<std::mutex> lock(taskMutex_);
      while (!stop_ && taskQueue_.size() == 0) {
        taskCond_.wait(lock);
      }
      if (taskQueue_.size() > 0) {
        LOG_TRACE << "got a new task!";
        r = std::move(taskQueue_.front());
        taskQueue_.pop();
      } else
        continue;
    }
    r();
  }
}

/**
 * @brief Construct a new concurrent task queue instance.
 *
 * @param threadNum The number of threads in the queue.
 * @param name The name of the queue.
 */
ConcurrentTaskQueue::ConcurrentTaskQueue(size_t threadNum, const std::string &name)
  : queueCount_(threadNum), queueName_(name), stop_(false) {
  assert(threadNum > 0);
  for (unsigned int i = 0; i < queueCount_; ++i) {
    threads_.push_back(std::thread(std::bind(&ConcurrentTaskQueue::queueFunc, this, i)));
  }
}

ConcurrentTaskQueue::~ConcurrentTaskQueue() {
  stop();
}

/**
 * @brief Run a task in the queue.
 *
 * @param task
 */
void ConcurrentTaskQueue::runTaskInQueue(const std::function<void()> &task) {
  LOG_TRACE << "copy task into queue";
  std::lock_guard<std::mutex> lock(taskMutex_);
  taskQueue_.push(task);
  taskCond_.notify_one();
}

/**
 * @brief Run a task in the queue.
 *
 * @param task
 */
void ConcurrentTaskQueue::runTaskInQueue(std::function<void()> &&task) {
  LOG_TRACE << "move task into queue";
  std::lock_guard<std::mutex> lock(taskMutex_);
  taskQueue_.push(std::move(task));
  taskCond_.notify_one();
}

/**
 * @brief Get the name of the queue.
 *
 * @return std::string
 */
std::string ConcurrentTaskQueue::getName() const {
  return queueName_;
};

/**
 * @brief Get the number of tasks to be executed in the queue.
 *
 * @return size_t
 */
size_t ConcurrentTaskQueue::getTaskCount() {
  std::lock_guard<std::mutex> guard(taskMutex_);
  return taskQueue_.size();
}

/**
 * @brief Stop all threads in the queue.
 *
 */
void ConcurrentTaskQueue::stop() {
  if (!stop_) {
    stop_ = true;
    taskCond_.notify_all();
    for (auto &t : threads_) t.join();
  }
}

}  // namespace trantor
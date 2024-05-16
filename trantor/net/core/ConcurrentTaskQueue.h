/**
 *
 *  @file ConcurrentTaskQueue.h
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

#ifndef TRANTOR_CONCURRENT_TASK_QUEUE_H
#define TRANTOR_CONCURRENT_TASK_QUEUE_H

#include "TaskQueue.h"
#include "trantor/exports.h"
#include <list>
#include <memory>
#include <queue>
#include <string>
#include <vector>

namespace trantor {
/**
 * @brief This class implements a task queue running in parallel. Basically this
 * can be called a threads pool.
 *
 */
class TRANTOR_EXPORT ConcurrentTaskQueue : public TaskQueue {
public:
  ConcurrentTaskQueue(size_t threadNum, const std::string &name);
  ~ConcurrentTaskQueue();

  virtual void        runTaskInQueue(const std::function<void()> &task);
  virtual void        runTaskInQueue(std::function<void()> &&task);
  virtual std::string getName() const;

  size_t              getTaskCount();
  void                stop();

private:
  size_t                            queueCount_;
  std::string                       queueName_;

  std::queue<std::function<void()>> taskQueue_;

  std::vector<std::thread>          threads_;
  std::mutex                        taskMutex_;
  std::condition_variable           taskCond_;
  std::atomic_bool                  stop_;
  void                              queueFunc(int queueNum);
};

}  // namespace trantor
#endif  // TRANTOR_CONCURRENT_TASK_QUEUE_H
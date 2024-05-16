#ifndef TRANTOR_SERIAL_TASK_QUEUE_H
#define TRANTOR_SERIAL_TASK_QUEUE_H

#include "TaskQueue.h"
#include "trantor/exports.h"
#include "trantor/net/core/EventLoopThread.h"
#include <atomic>
#include <mutex>
#include <queue>
#include <string>

namespace trantor {
/**
 * @brief This class represents a task queue in which all tasks are executed one
 * by one.
 *
 */
class TRANTOR_EXPORT SerialTaskQueue : public TaskQueue {
public:
  SerialTaskQueue() = delete;
  explicit SerialTaskQueue(const std::string &name);
  virtual ~SerialTaskQueue();
  virtual void        runTaskInQueue(const std::function<void()> &task);
  virtual void        runTaskInQueue(std::function<void()> &&task);
  virtual std::string getName() const;

  size_t              getTaskCount();
  bool                isRunningTask();
  void                waitAllTasksFinished();
  void                stop();

protected:
  std::string     queueName_;
  EventLoopThread loopThread_;
  bool            stop_{false};
};

}  // namespace trantor
#endif  // TRANTOR_SERIAL_TASK_QUEUE_H

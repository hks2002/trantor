// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// Taken from Muduo and modified
// Copyright 2016, Tao An.  All rights reserved.
// https://github.com/an-tao/trantor
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Tao An

#ifndef TRANTOR_EVENT_LOOP_H
#define TRANTOR_EVENT_LOOP_H

#include "trantor/NonCopyable.h"
#include "trantor/exports.h"
#include "trantor/net/core/LockFreeQueue.h"
#include "trantor/net/core/TimerQueue.h"
#include "trantor/utils/Date.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace trantor {
class Poller;
class TimerQueue;
class Channel;
using ChannelList = std::vector<Channel *>;
using Func        = std::function<void()>;
using TimerId     = uint64_t;
enum { InvalidTimerId = 0 };

/**
 * @brief As the name implies, this class represents an event loop that runs in
 * a particular thread. The event loop can handle network I/O events and timers
 * in asynchronous mode.
 * @note An event loop object always belongs to a separate thread, and there is
 * one event loop object at most in a thread. We can call an event loop object
 * the event loop of the thread it belongs to, or call that thread the thread of
 * the event loop.
 */
class TRANTOR_EXPORT EventLoop : NonCopyable {
public:
  EventLoop();
  ~EventLoop();

  size_t            index();
  void              setIndex(size_t index);

  bool              isInLoopThread() const;
  bool              isRunning();
  bool              isCallingFunctions();
  void              assertInLoopThread();

  void              updateChannel(Channel *chl);
  void              removeChannel(Channel *chl);

  static EventLoop *getEventLoopOfCurrentThread();
  void              moveToCurrentThread();

  void              loop();
  void              quit();

  void              queueInLoop(const Func &f);
  void              queueInLoop(Func &&f);

  /**
   * @brief Run the function f in the thread of the event loop.
   *
   * @param f
   * @note If the current thread is the thread of the event loop, the function
   * f is executed directly before the method exiting.
   */
  template <typename Functor>
  void runInLoop(Functor &&f) {
    if (isInLoopThread()) {
      f();
    } else {
      queueInLoop(std::forward<Functor>(f));
    }
  }

  TimerId runAt(const Date &time, const Func &cb);
  TimerId runAt(const Date &time, Func &&cb);

  TimerId runAfter(double delay, const Func &cb);
  TimerId runAfter(double delay, Func &&cb);
  TimerId runAfter(const std::chrono::duration<double> &delay, const Func &cb);
  TimerId runAfter(const std::chrono::duration<double> &delay, Func &&cb);

  TimerId runEvery(double interval, const Func &cb);
  TimerId runEvery(double interval, Func &&cb);
  TimerId runEvery(const std::chrono::duration<double> &interval, const Func &cb);
  TimerId runEvery(const std::chrono::duration<double> &interval, Func &&cb);

  void    runOnQuit(Func &&cb);
  void    runOnQuit(const Func &cb);

  void    invalidateTimer(TimerId id);
#ifdef __linux__
  void resetTimerQueue();
#endif
  void resetAfterFork();

private:
#ifdef _WIN32
  size_t index_{size_t(-1)};
#else
  size_t index_{std::numeric_limits<size_t>::max()};
#endif
  std::thread::id             threadId_{std::this_thread::get_id()};
  std::atomic<bool>           looping_{false};
  std::atomic<bool>           quit_{false};
  std::unique_ptr<Poller>     poller_{nullptr};
  ChannelList                 activeChannels_;
  Channel                    *currentActiveChannel_{nullptr};
  bool                        eventHandling_{false};
  std::unique_ptr<TimerQueue> timerQueue_{new TimerQueue(this)};
  EventLoop                 **threadLocalLoopPtr_;

#ifdef __linux__
  int                      wakeupFd_;
  std::unique_ptr<Channel> wakeupChannelPtr_;
#elif defined _WIN32
#else
  int                      wakeupFd_[2];
  std::unique_ptr<Channel> wakeupChannelPtr_;
#endif

  bool            callingFuncs_{false};
  MpscQueue<Func> funcs_;
  MpscQueue<Func> funcsOnQuit_;

  void            wakeup();
  void            wakeupRead();

  void            abortNotInLoopThread();
  void            doRunInLoopFuncs();
};

}  // namespace trantor

#endif  // TRANTOR_EVENT_LOOP_H

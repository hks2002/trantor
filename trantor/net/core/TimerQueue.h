/**
 *
 *  TimerQueue.h
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

#ifndef TRANTOR_TIMER_QUEUE_H
#define TRANTOR_TIMER_QUEUE_H

#include "Timer.h"
#include "trantor/callbacks.h"
#include <atomic>
#include <memory>
#include <queue>
#include <trantor/NonCopyable.h>
#include <unordered_set>

namespace trantor {

// class Timer;
class EventLoop;
class Channel;
using TimerPtr       = std::shared_ptr<Timer>;
using TimerPtrVector = std::vector<TimerPtr>;
struct TimerPtrComparer {
  bool operator()(const TimerPtr &x, const TimerPtr &y) const {
    return *x > *y;
  }
};
using TimerPriQueue = std::priority_queue<TimerPtr, TimerPtrVector, TimerPtrComparer>;

class TimerQueue : NonCopyable {
public:
  explicit TimerQueue(EventLoop *loop);
  ~TimerQueue();

  TimerId addTimer(const TimerCallback &cb, const TimePoint &when, const TimeInterval &interval);
  TimerId addTimer(TimerCallback &&cb, const TimePoint &when, const TimeInterval &interval);
  void    addTimerInLoop(const TimerPtr &timer);
  void    invalidateTimer(TimerId id);
#ifdef __linux__
  void reset();
#else
  int64_t getTimeout() const;
  void    processTimers();
#endif

protected:
  EventLoop *loop_;
#ifdef __linux__
  int                      timerfd_;
  std::shared_ptr<Channel> timerfdChannelPtr_;
  void                     handleRead();
#endif
  TimerPriQueue  timers_;

  bool           insert(const TimerPtr &timePtr);
  void           reset(const TimerPtrVector &expired, const TimePoint &now);
  bool           callingExpiredTimers_;
  TimerPtrVector getExpired();
  TimerPtrVector getExpired(const TimePoint &now);

private:
  std::unordered_set<uint64_t> timerIdSet_;
};

}  // namespace trantor
#endif  // TRANTOR_TIMER_QUEUE_H

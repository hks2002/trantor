/**
 *
 *  Timer.h
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

#ifndef TRANTOR_TIMER_H
#define TRANTOR_TIMER_H

#include "trantor/NonCopyable.h"
#include "trantor/callbacks.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>

namespace trantor {
using TimerId      = uint64_t;
using TimePoint    = std::chrono::steady_clock::time_point;
using TimeInterval = std::chrono::microseconds;

/**
 * @brief A timer class to run a callback function at a certain time point.
 *
 */
class Timer : public NonCopyable {
public:
  Timer(const TimerCallback &cb, const TimePoint &when, const TimeInterval &interval);
  Timer(TimerCallback &&cb, const TimePoint &when, const TimeInterval &interval);
  ~Timer() {
    //   std::cout<<"Timer deconstruct!"<<std::endl;
  }

  TimerId          id();
  const TimePoint &when() const;
  bool             isRepeat();
  void             run() const;
  void             restart(const TimePoint &now);
  bool             operator<(const Timer &t) const;
  bool             operator>(const Timer &t) const;

private:
  static std::atomic<TimerId> timersCreated_;
  const TimerId               id_;
  TimePoint                   when_;
  const TimeInterval          interval_;
  const bool                  repeat_;
  TimerCallback               callback_;
};

}  // namespace trantor
#endif  // TRANTOR_TIMER_H

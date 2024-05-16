/**
 *
 *  Timer.cc
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

#include "Timer.h"

#include "trantor/callbacks.h"
#include "trantor/logger/Logger.h"
#include <trantor/net/core/EventLoop.h>

namespace trantor {
std::atomic<TimerId> Timer::timersCreated_ = ATOMIC_VAR_INIT(InvalidTimerId);

Timer::Timer(const TimerCallback &cb, const TimePoint &when, const TimeInterval &interval)
  : callback_(cb), when_(when), interval_(interval), repeat_(interval.count() > 0), id_(++timersCreated_) {}

Timer::Timer(TimerCallback &&cb, const TimePoint &when, const TimeInterval &interval)
  : callback_(std::move(cb)), when_(when), interval_(interval), repeat_(interval.count() > 0), id_(++timersCreated_) {
  // LOG_TRACE<<"Timer move constructor";
}

/**
 * @brief Returns the ID of the Timer.
 *
 * @return the ID of the Timer
 */
TimerId Timer::id() {
  return id_;
}

/**
 * @brief Returns the time point when the timer should fire.
 *
 */
const TimePoint &Timer::when() const {
  return when_;
}

/**
 * @brief Checks if the Timer should repeat.
 *
 * @return the status of the repeat flag
 */
bool Timer::isRepeat() {
  return repeat_;
}

/**
 * @brief Runs the timer by invoking the callback function.
 */
void Timer::run() const {
  callback_();
}

/**
 * @brief Resets the time point when the timer should fire.
 *
 */
void Timer::restart(const TimePoint &now) {
  if (repeat_) {
    when_ = now + interval_;
  } else
    when_ = std::chrono::steady_clock::now();
}

/**
 * @brief Compares to the Timer.
 *
 * @param t The Timer object to compare to.
 */
bool Timer::operator<(const Timer &t) const {
  return when_ < t.when_;
}

/**
 * @brief Compares to the Timer.
 *
 * @param t The Timer object to compare to.
 */
bool Timer::operator>(const Timer &t) const {
  return when_ > t.when_;
}

}  // namespace trantor

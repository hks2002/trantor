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
#include <trantor/utils/Logger.h>
#include <trantor/net/EventLoop.h>

namespace trantor
{
std::atomic<TimerId> Timer::timersCreated_ = ATOMIC_VAR_INIT(InvalidTimerId);
Timer::Timer(const TimerCallback &cb, const Date &when, double interval)
    : callback_(cb),
      when_(when),
      interval_(interval),
      repeat_(interval > 0.0),
      id_(++timersCreated_)
{
}
Timer::Timer(TimerCallback &&cb, const Date &when, double interval)
    : callback_(std::move(cb)),
      when_(when),
      interval_(interval),
      repeat_(interval > 0.0),
      id_(++timersCreated_)
{
    // LOG_TRACE<<"Timer move contrustor";
}
void Timer::run() const
{
    callback_();
}
void Timer::restart(const Date &now)
{
    if (repeat_)
    {
        when_ = now.after(interval_);
    }
    else
        when_ = Date();
}
bool Timer::operator<(const Timer &t) const
{
    return when_ < t.when_;
}
bool Timer::operator>(const Timer &t) const
{
    return when_ > t.when_;
}
}  // namespace trantor

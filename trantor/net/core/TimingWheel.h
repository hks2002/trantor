/**
 *
 *  @file TimingWheel.h
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

#ifndef TRANTOR_TIMING_WHEEL_H
#define TRANTOR_TIMING_WHEEL_H

#include "trantor/exports.h"
#include "trantor/net/core/EventLoop.h"
#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>

constexpr auto TIMING_BUCKET_NUM_PER_WHEEL = 100;
constexpr auto TIMING_TICK_INTERVAL        = 1.0;

namespace trantor {
using EntryPtr    = std::shared_ptr<void>;
using EntryBucket = std::unordered_set<EntryPtr>;
using BucketQueue = std::deque<EntryBucket>;

/**
 * @brief This class implements a timer strategy with high performance and low
 * accuracy. This is usually used internally.
 *
 */
class TRANTOR_EXPORT TimingWheel {
public:
  class CallbackEntry {
  public:
    CallbackEntry(std::function<void()> cb) : cb_(std::move(cb)) {}
    ~CallbackEntry() {
      cb_();
    }

  private:
    std::function<void()> cb_;
  };

  TimingWheel(trantor::EventLoop *loop,
              size_t              maxTimeout,
              float               ticksInterval      = TIMING_TICK_INTERVAL,
              size_t              bucketsNumPerWheel = TIMING_BUCKET_NUM_PER_WHEEL);
  ~TimingWheel();

  EventLoop *getLoop();
  void       insertEntry(size_t delay, EntryPtr entryPtr);
  void       insertEntryInLoop(size_t delay, EntryPtr entryPtr);

private:
  std::vector<BucketQueue> wheels_;
  std::atomic<size_t>      ticksCounter_{0};

  trantor::TimerId         timerId_;
  trantor::EventLoop      *loop_;

  float                    ticksInterval_;
  size_t                   wheelsNum_;
  size_t                   bucketsNumPerWheel_;
};

}  // namespace trantor
#endif  // TRANTOR_TIMING_WHEEL_H

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

#include "EventLoop.h"

#include "trantor/logger/Logger.h"
#include "trantor/net/core/Channel.h"
#include "trantor/net/core/TimerQueue.h"
#include "trantor/net/core/poller/Poller.h"
#include <algorithm>
#include <assert.h>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <signal.h>
#include <thread>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <io.h>
#include <synchapi.h>
// clang-format on
#ifndef _SSIZE_T_DEFINED
using ssize_t = long long;
#endif
#else
#include <poll.h>
#include <unistd.h>
#endif

#ifdef __linux__
#include <sys/eventfd.h>
#endif

namespace trantor {

thread_local EventLoop *t_loopInThisThread = nullptr;

#ifdef __linux__
int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    std::cout << "Failed in eventfd" << std::endl;
    abort();
  }

  return evtfd;
}
const int kPollTimeMs = 10000;
#endif

// The event loop needs a scope exit, so here's the simplest most limited
// C++14 scope exit available (from https://stackoverflow.com/a/42506763/3173540)
//
// TODO: If this is needed anywhere else, introduce a proper on_exit from, for
// example, the GSL library
namespace {
template <typename F>
struct ScopeExit {
  ScopeExit(F &&f) : f_(std::forward<F>(f)) {}
  ~ScopeExit() {
    f_();
  }
  F f_;
};

template <typename F>
ScopeExit<F> makeScopeExit(F &&f) {
  return ScopeExit<F>(std::forward<F>(f));
};
}  // namespace

EventLoop::EventLoop()
  : looping_(false),
    threadId_(std::this_thread::get_id()),
    quit_(false),
    poller_(Poller::newPoller(this)),
    currentActiveChannel_(nullptr),
    eventHandling_(false),
    timerQueue_(new TimerQueue(this)),
#ifdef __linux__
    wakeupFd_(createEventfd()),
    wakeupChannelPtr_(new Channel(this, wakeupFd_)),
#endif
    threadLocalLoopPtr_(&t_loopInThisThread) {
  if (t_loopInThisThread) {
    LOG_FATAL << "There is already an EventLoop in this thread";
    exit(-1);
  }
  t_loopInThisThread = this;

#ifdef __linux__
  wakeupChannelPtr_->setReadCallback(std::bind(&EventLoop::wakeupRead, this));
  wakeupChannelPtr_->enableReading();
#elif !defined _WIN32
  auto r = pipe(wakeupFd_);
  (void)r;
  assert(!r);
  fcntl(wakeupFd_[0], F_SETFL, O_NONBLOCK | O_CLOEXEC);
  fcntl(wakeupFd_[1], F_SETFL, O_NONBLOCK | O_CLOEXEC);
  wakeupChannelPtr_ = std::unique_ptr<Channel>(new Channel(this, wakeupFd_[0]));
  wakeupChannelPtr_->setReadCallback(std::bind(&EventLoop::wakeupRead, this));
  wakeupChannelPtr_->enableReading();
#else
  poller_->setEventCallback([](uint64_t event) {
    assert(event == 1);
  });
#endif
}

EventLoop::~EventLoop() {
#ifdef _WIN32
  DWORD delay = 1; /* 1 msec */
#else
  struct timespec delay = {0, 1000000}; /* 1 msec */
#endif

  quit();

  // Spin waiting for the loop to exit because
  // this may take some time to complete. We
  // assume the loop thread will *always* exit.
  // If this cannot be guaranteed then one option
  // might be to abort waiting and
  // assert(!looping_) after some delay;
  while (looping_.load(std::memory_order_acquire)) {
#ifdef _WIN32
    Sleep(delay);
#else
    nanosleep(&delay, nullptr);
#endif
  }

  t_loopInThisThread = nullptr;
#ifdef __linux__
  close(wakeupFd_);
#elif defined _WIN32
#else
  close(wakeupFd_[0]);
  close(wakeupFd_[1]);
#endif
}

/**
 * @brief Return the index of the event loop.
 *
 */
size_t EventLoop::index() {
  return index_;
}

/**
 * @brief Set the index of the event loop.
 *
 */
void EventLoop::setIndex(size_t index) {
  index_ = index;
}

/**
 * @brief Return true if the current thread is the thread to which the event
 * loop belongs.
 *
 * @return true
 * @return false
 */
bool EventLoop::isInLoopThread() const {
  return threadId_ == std::this_thread::get_id();
};

/**
 * @brief Return true if the event loop is running.
 *
 * @return true
 * @return false
 */
bool EventLoop::isRunning() {
  return looping_.load(std::memory_order_acquire) && (!quit_.load(std::memory_order_acquire));
}

/**
 * @brief Check if the event loop is calling a function.
 *
 * @return true
 * @return false
 */
bool EventLoop::isCallingFunctions() {
  return callingFuncs_;
}

/**
 * @brief Assertion that the current thread is the thread to which the event
 * loop belongs. If the assertion fails, the program aborts.
 */
void EventLoop::assertInLoopThread() {
  if (!isInLoopThread()) {
    abortNotInLoopThread();
  }
};

/**
 * @brief Aborts the function execution if it is not in the event loop thread.
 *
 */
void EventLoop::abortNotInLoopThread() {
  LOG_FATAL << "It is forbidden to run loop on threads other than event-loop "
               "thread";
  exit(1);
}

/**
 * @brief Update channel status. This method is usually used internally.
 *
 */
void EventLoop::updateChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

/**
 * @brief Remove a channel from the event loop. This method is usually used
 * internally.
 *
 */
void EventLoop::removeChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->removeChannel(channel);
}

/**
 * @brief Get the event loop of the current thread. Return nullptr if there
 * is no event loop in the current thread.
 *
 * @return EventLoop*
 */
EventLoop *EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}

/**
 * @brief Move the EventLoop to the current thread, this method must be
 * called before the loop is running.
 *
 */
void EventLoop::moveToCurrentThread() {
  if (isRunning()) {
    LOG_FATAL << "EventLoop cannot be moved when running";
    exit(-1);
  }
  if (isInLoopThread()) {
    LOG_WARN << "This EventLoop is already in the current thread";
    return;
  }
  if (t_loopInThisThread) {
    LOG_FATAL << "There is already an EventLoop in this thread, you cannot "
                 "move another in";
    exit(-1);
  }
  *threadLocalLoopPtr_ = nullptr;
  t_loopInThisThread   = this;
  threadLocalLoopPtr_  = &t_loopInThisThread;
  threadId_            = std::this_thread::get_id();
}

/**
 * @brief Run the event loop. This method will be blocked until the event
 * loop exits.
 *
 */
void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();

  looping_.store(true, std::memory_order_release);
  quit_.store(false, std::memory_order_release);

  std::exception_ptr loopException;
  try {  // Scope where the loop flag is set

    auto loopFlagCleaner = makeScopeExit([this]() {
      looping_.store(false, std::memory_order_release);
    });

    while (!quit_.load(std::memory_order_acquire)) {
      activeChannels_.clear();
#ifdef __linux__
      poller_->poll(kPollTimeMs, &activeChannels_);
#else
      poller_->poll(static_cast<int>(timerQueue_->getTimeout()), &activeChannels_);
      timerQueue_->processTimers();
#endif
      // TODO sort channel by priority
      // std::cout<<"after ->poll()"<<std::endl;
      eventHandling_ = true;
      for (auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
        currentActiveChannel_ = *it;
        currentActiveChannel_->handleEvent();
      }
      currentActiveChannel_ = nullptr;
      eventHandling_        = false;
      // std::cout << "looping" << endl;
      doRunInLoopFuncs();
    }
    // loopFlagCleaner clears the loop flag here
  } catch (std::exception &e) {
    LOG_WARN << "Exception thrown from event loop, rethrowing after "
                "running functions on quit: "
             << e.what();
    loopException = std::current_exception();
  }

  // Run the quit functions even if exceptions were thrown
  // TODO: if more exceptions are thrown in the quit functions, some are left
  // un-run. Can this be made exception safe?
  Func f;
  while (funcsOnQuit_.dequeue(f)) {
    f();
  }

  // Throw the exception from the end
  if (loopException) {
    LOG_WARN << "Rethrowing exception from event loop";
    std::rethrow_exception(loopException);
  }
}

void EventLoop::wakeup() {
  // if (!looping_)
  //     return;
  uint64_t tmp = 1;
#ifdef __linux__
  int ret = write(wakeupFd_, &tmp, sizeof(tmp));
  (void)ret;
#elif defined _WIN32
  poller_->postEvent(1);
#else
  int ret = write(wakeupFd_[1], &tmp, sizeof(tmp));
  (void)ret;
#endif
}

void EventLoop::wakeupRead() {
  ssize_t ret = 0;
#ifdef __linux__
  uint64_t tmp;
  ret = read(wakeupFd_, &tmp, sizeof(tmp));
#elif defined _WIN32
#else
  uint64_t tmp;
  ret = read(wakeupFd_[0], &tmp, sizeof(tmp));
#endif
  if (ret < 0) LOG_SYSERR << "wakeup read error";
}

/**
 * @brief Let the event loop quit.
 *
 */
void EventLoop::quit() {
  quit_.store(true, std::memory_order_release);

  if (!isInLoopThread()) {
    wakeup();
  }
}

/**
 * @brief Run the function f in the thread of the event loop.
 *
 * @param f
 * @note The difference between this method and the runInLoop() method is
 * that the function f is executed after the method exiting no matter if the
 * current thread is the thread of the event loop.
 */
void EventLoop::queueInLoop(const Func &cb) {
  funcs_.enqueue(cb);
  if (!isInLoopThread() || !looping_.load(std::memory_order_acquire)) {
    wakeup();
  }
}

/**
 * @brief Run the function f in the thread of the event loop.
 *
 * @param f
 * @note The difference between this method and the runInLoop() method is
 * that the function f is executed after the method exiting no matter if the
 * current thread is the thread of the event loop.
 */
void EventLoop::queueInLoop(Func &&cb) {
  funcs_.enqueue(std::move(cb));
  if (!isInLoopThread() || !looping_.load(std::memory_order_acquire)) {
    wakeup();
  }
}

/**
 * @brief Executes all the functions in the event loop queue.
 */
void EventLoop::doRunInLoopFuncs() {
  callingFuncs_ = true;
  {
    // Assure the flag is cleared even if func throws
    auto callingFlagCleaner = makeScopeExit([this]() {
      callingFuncs_ = false;
    });
    // the destructor for the Func may itself insert a new entry into the
    // queue
    // TODO: The following is exception-unsafe. If one  of the funcs throws,
    // the remaining ones will not get run. The simplest fix is to catch any
    // exceptions and rethrow them later, but somehow that seems fishy...
    while (!funcs_.empty()) {
      Func func;
      while (funcs_.dequeue(func)) {
        func();
      }
    }
  }
}

/**
 * @brief Run a function at a time point.
 *
 * @param time The time to run the function.
 * @param cb The function to run.
 * @return TimerId The ID of the timer.
 */
TimerId EventLoop::runAt(const Date &time, const Func &cb) {
  auto microSeconds                        = time.microSecondsSinceEpoch() - Date::now().microSecondsSinceEpoch();
  std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now() + std::chrono::microseconds(microSeconds);
  return timerQueue_->addTimer(cb, tp, std::chrono::microseconds(0));
}

/**
 * @brief Run a function at a time point.
 *
 * @param time The time to run the function.
 * @param cb The function to run.
 * @return TimerId The ID of the timer.
 */
TimerId EventLoop::runAt(const Date &time, Func &&cb) {
  auto microSeconds                        = time.microSecondsSinceEpoch() - Date::now().microSecondsSinceEpoch();
  std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now() + std::chrono::microseconds(microSeconds);
  return timerQueue_->addTimer(std::move(cb), tp, std::chrono::microseconds(0));
}

/**
 * @brief Run a function after a period of time.
 *
 * @param delay Represent the period of time in seconds.
 * @param cb The function to run.
 * @return TimerId The ID of the timer.
 */
TimerId EventLoop::runAfter(double delay, const Func &cb) {
  return runAt(Date::date().after(delay), cb);
}

/**
 * @brief Run a function after a period of time.
 *
 * @param delay Represent the period of time in seconds.
 * @param cb The function to run.
 * @return TimerId The ID of the timer.
 */
TimerId EventLoop::runAfter(double delay, Func &&cb) {
  return runAt(Date::date().after(delay), std::move(cb));
}

/**
 * @brief Run a function after a period of time.
 * @note Users could use chrono literals to represent a time duration
 * For example:
 * @code
   runAfter(5s, task);
   runAfter(10min, task);
   @endcode
 */
TimerId EventLoop::runAfter(const std::chrono::duration<double> &delay, const Func &cb) {
  return runAfter(delay.count(), cb);
}

/**
 * @brief Run a function after a period of time.
 * @note Users could use chrono literals to represent a time duration
 * For example:
 * @code
   runAfter(5s, task);
   runAfter(10min, task);
   @endcode
 */
TimerId EventLoop::runAfter(const std::chrono::duration<double> &delay, Func &&cb) {
  return runAfter(delay.count(), std::move(cb));
}

/**
 * @brief Repeatedly run a function every period of time.
 *
 * @param interval The duration in seconds.
 * @param cb The function to run.
 * @return TimerId The ID of the timer.
 */
TimerId EventLoop::runEvery(double interval, const Func &cb) {
  std::chrono::microseconds dur(static_cast<std::chrono::microseconds::rep>(interval * 1000000));
  auto                      tp = std::chrono::steady_clock::now() + dur;
  return timerQueue_->addTimer(cb, tp, dur);
}
/**
 * @brief Repeatedly run a function every period of time.
 *
 * @param interval The duration in seconds.
 * @param cb The function to run.
 * @return TimerId The ID of the timer.
 */
TimerId EventLoop::runEvery(double interval, Func &&cb) {
  std::chrono::microseconds dur(static_cast<std::chrono::microseconds::rep>(interval * 1000000));
  auto                      tp = std::chrono::steady_clock::now() + dur;
  return timerQueue_->addTimer(std::move(cb), tp, dur);
}

/**
 * @brief Repeatedly run a function every period of time.
 * Users could use chrono literals to represent a time duration
 * For example:
 * @code
   runEvery(5s, task);
   runEvery(10min, task);
   runEvery(0.1h, task);
   @endcode
 */
TimerId EventLoop::runEvery(const std::chrono::duration<double> &interval, const Func &cb) {
  return runEvery(interval.count(), cb);
}

/**
 * @brief Repeatedly run a function every period of time.
 * Users could use chrono literals to represent a time duration
 * For example:
 * @code
   runEvery(5s, task);
   runEvery(10min, task);
   runEvery(0.1h, task);
   @endcode
 */
TimerId EventLoop::runEvery(const std::chrono::duration<double> &interval, Func &&cb) {
  return runEvery(interval.count(), std::move(cb));
}

/**
 * @brief Run functions when the event loop quits
 *
 * @param cb the function to run
 * @note the function runs on the thread that quits the EventLoop
 */
void EventLoop::runOnQuit(Func &&cb) {
  funcsOnQuit_.enqueue(std::move(cb));
}
/**
 * @brief Run functions when the event loop quits
 *
 * @param cb the function to run
 * @note the function runs on the thread that quits the EventLoop
 */
void EventLoop::runOnQuit(const Func &cb) {
  funcsOnQuit_.enqueue(cb);
}

/**
 * @brief Invalidate the timer identified by the given ID.
 *
 * @param id The ID of the timer.
 */
void EventLoop::invalidateTimer(TimerId id) {
  if (isRunning() && timerQueue_) timerQueue_->invalidateTimer(id);
}

#ifdef __linux__
/**
 * @brief Make the timer queue works after calling the fork() function.
 *
 */
void EventLoop::resetTimerQueue() {
  assertInLoopThread();
  assert(!looping_.load(std::memory_order_acquire));
  timerQueue_->reset();
}
#endif

/**
 * @brief Make the event loop works after calling the fork() function.
 *
 */
void EventLoop::resetAfterFork() {
  poller_->resetAfterFork();
}

}  // namespace trantor

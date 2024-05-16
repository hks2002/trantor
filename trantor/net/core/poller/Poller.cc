/**
 *
 *  Poller.cc
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

#include "Poller.h"
#ifdef __linux__
#include "EpollPoller.h"
#elif defined _WIN32
#include "EpollPoller.h"
#include "Wepoll.h"
#elif defined __FreeBSD__ || defined __OpenBSD__ || defined __APPLE__
#include "KQueue.h"
#else
#include "PollPoller.h"
#endif

namespace trantor {
/**
 * Creates a new poller based on the platform and returns it.
 *
 * @param loop Pointer to the event loop associated with the poller.
 *
 * @return Pointer to the newly created poller.
 */
Poller     *Poller::newPoller(EventLoop *loop) {
#if defined __linux__ || defined _WIN32
  return new EpollPoller(loop);
#elif defined __FreeBSD__ || defined __OpenBSD__ || defined __APPLE__
  return new KQueue(loop);
#else
  return new PollPoller(loop);
#endif
}
}  // namespace trantor
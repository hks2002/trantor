/**
 *
 *  PollPoller.h
 *  Martin Chang
 *
 *  Copyright 2021, An Tao.  All rights reserved.
 *  https://github.com/an-tao/trantor
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the License file.
 *
 *  Trantor
 *
 */
#ifndef TRANTOR_POLL_POLLER_H
#define TRANTOR_POLL_POLLER_H

#include "Poller.h"
#include <vector>

#if defined __unix__ || defined __HAIKU__
#include <poll.h>
#endif

/**
 * @brief This is used for unix and haiku platforms.
 *
 */
namespace trantor {
class PollPoller : public Poller {
public:
  PollPoller(EventLoop* loop);
  ~PollPoller() override;

  void poll(int timeoutMs, ChannelList* activeChannels) override;
  void updateChannel(Channel* channel) override;
  void removeChannel(Channel* channel) override;

private:
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

#if defined __unix__ || defined __HAIKU__
  std::vector<struct pollfd> pollfds_;
  std::map<int, Channel*>    channels_;
#endif
};

}  // namespace trantor

#endif  // TRANTOR_POLL_POLLER_H

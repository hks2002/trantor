/**
 *
 *  @file RawLogger.cc
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

#include "RawLogger.h"

#include "Logger.h"
#ifdef TRANTOR_SPDLOG_SUPPORT
#include <spdlog/spdlog.h>
#endif

namespace trantor {
RawLogger &RawLogger::setIndex(int index) {
  index_ = index;
  return *this;
}

LogStream &RawLogger::stream() {
  return logStream_;
}

RawLogger::~RawLogger() {
#ifdef TRANTOR_SPDLOG_SUPPORT
  auto logger = Logger::getRawSpdLogger(index_);
  if (logger) {
    // The only way to be fully compatible with the existing non-spdlog RAW
    // mode (dumping raw without adding a '\n') would be to store the
    // concatenated messages along the logger, and pass the complete message
    // to spdlog only when it ends with '\n'.
    // But it's overkill...
    // For now, just remove the trailing '\n', if any, since spdlog
    // automatically adds one.
    auto msglen = logStream_.bufferLength();
    if ((msglen > 0) && (logStream_.bufferData()[msglen - 1] == '\n')) msglen--;
    logger->info(spdlog::string_view_t(logStream_.bufferData(), msglen));
    return;
  }
#endif

  if (index_ < 0) {
    auto &oFunc = Logger::outputFunc_();
    if (!oFunc) return;
    oFunc(logStream_.bufferData(), logStream_.bufferLength());
  } else {
    auto &oFunc = Logger::outputFunc_(index_);
    if (!oFunc) return;
    oFunc(logStream_.bufferData(), logStream_.bufferLength());
  }
}

}  // namespace trantor
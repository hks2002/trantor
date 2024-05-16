/**
 *
 *  @file RawLogger.h
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

#ifndef TRANTOR_RAW_LOGGER_H
#define TRANTOR_RAW_LOGGER_H

#include "LogStream.h"
#include "trantor/NonCopyable.h"
#include "trantor/exports.h"

namespace trantor {
/**
 * @brief A simple logger that without level, without date.
 *
 */
class TRANTOR_EXPORT RawLogger : public NonCopyable {
public:
  RawLogger &setIndex(int index);
  LogStream &stream();
  ~RawLogger();

private:
  LogStream logStream_;
  int       index_{-1};
};

}  // namespace trantor

#endif  // TRANTOR_RAW_LOGGER_H

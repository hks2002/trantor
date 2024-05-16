/**
 *
 *  @file Fmt.h
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

#ifndef TRANTOR_FMT_H
#define TRANTOR_FMT_H

#include "trantor/exports.h"

namespace trantor {

/**
 * @brief Format the value in fmt into the buffer.
 *
 */
class TRANTOR_EXPORT Fmt {
public:
  template <typename T>
  Fmt(const char *fmt, T val);

  const char *data() const;
  int         length() const;

private:
  char buf_[48];
  int  length_;
};

}  // namespace trantor
#endif  // TRANTOR_FMT_H

/**
 *
 *  @file FixedBuffer.h
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
#ifndef TRANTOR_FIXED_BUFFER_H
#define TRANTOR_FIXED_BUFFER_H

#include "trantor/NonCopyable.h"
#include "trantor/exports.h"
#include <cstddef>
#include <string.h>  // memcpy
#include <string>

namespace trantor {
namespace detail {

static constexpr size_t kSmallBuffer{4000};
static constexpr size_t kLargeBuffer{4000 * 1000};

/**
 * @brief Constant buffer with fixed size.
 *
 */
template <size_t SIZE>
class TRANTOR_EXPORT FixedBuffer : NonCopyable {
public:
  FixedBuffer() : cur_(data_) {
    setCookie(cookieStart);
  }

  ~FixedBuffer() {
    setCookie(cookieEnd);
  }

  const char *data() const;
  size_t      length() const;
  size_t      avail() const;
  char       *current();
  void        add(size_t len);
  bool        append(const char        */*restrict*/ buf, size_t len);
  void        reset();
  void        zeroBuffer();
  void        setCookie(void (*cookie)());

  // for used by GDB
  const char *debugString();
  // for used by unit test
  std::string toString() const;

private:
  char        data_[SIZE];
  char       *cur_;

  const char *end() const;
  void (*cookie_)();
  // Must be outline function for cookies.
  static void cookieStart();
  static void cookieEnd();
};

}  // namespace detail
}  // namespace trantor

#endif  // TRANTOR_FIXED_BUFFER_H
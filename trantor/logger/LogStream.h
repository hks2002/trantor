/**
 *
 *  @file LogStream.h
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

#ifndef TRANTOR_LOG_STREAM_H
#define TRANTOR_LOG_STREAM_H

// Taken from muduo lib and modified. Classes in this file are used internally.
#include "FixedBuffer.h"
#include "Fmt.h"
#include "SourceFile.h"
#include "trantor/NonCopyable.h"
#include "trantor/exports.h"
#include <cassert>
#include <string>
#include <string_view>

namespace trantor {
/**
 * @brief LogStream with fixed buffer, for performance.
 * @note When buffer is full, will use std::string to store.
 *
 */
class TRANTOR_EXPORT LogStream : NonCopyable {
  using self = LogStream;

public:
  using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;

  const char *bufferData() const;
  size_t      bufferLength() const;
  void        resetBuffer();

  void        append(const char *data, size_t len);

  self       &operator<<(bool v);

  self       &operator<<(short);
  self       &operator<<(unsigned short);
  self       &operator<<(int);
  self       &operator<<(unsigned int);
  self       &operator<<(long);
  self       &operator<<(unsigned long);
  self       &operator<<(const long long &);
  self       &operator<<(const unsigned long long &);

  self       &operator<<(float &v);
  self       &operator<<(const double &);
  self       &operator<<(const long double &v);

  self       &operator<<(char v);
  self       &operator<<(const void *);
  self       &operator<<(char *str);
  self       &operator<<(const char *str);
  self       &operator<<(const unsigned char *str);
  self       &operator<<(const std::string &v);
  self       &operator<<(std::string_view sv);

  template <int N>
  self &operator<<(const char (&buf)[N]) {
    assert(strnlen(buf, N) == N - 1);
    append(buf, N - 1);
    return *this;
  }
  self &operator<<(const Fmt &fmt);

  self &operator<<(const SourceFile &v);

private:
  template <typename I>
  void        formatInteger(I);

  Buffer      buffer_;
  std::string exBuffer_;  // extra buffer, used when buffer_ is full or not enough
};

}  // namespace trantor

#endif  // TRANTOR_LOG_STREAM_H

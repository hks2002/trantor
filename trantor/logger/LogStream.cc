/**
 *
 *  LogStream.cc
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

// taken from muduo lib

#include "LogStream.h"

#include "SourceFile.h"
#include "trantor/utils/StringUtils.h"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdint.h>
#include <stdio.h>
#include <string.h>  // memcpy

using namespace trantor::utils;

namespace trantor {

const char *LogStream::bufferData() const {
  return exBuffer_.empty() ? buffer_.data() : exBuffer_.data();
}

size_t LogStream::bufferLength() const {
  return exBuffer_.empty() ? buffer_.length() : exBuffer_.length();
}

void LogStream::resetBuffer() {
  buffer_.reset();
  exBuffer_.clear();
}

void LogStream::append(const char *data, size_t len) {
  if (exBuffer_.empty()) {
    if (!buffer_.append(data, len)) {
      exBuffer_.append(buffer_.data(), buffer_.length());
      exBuffer_.append(data, len);
    }
  } else {
    exBuffer_.append(data, len);
  }
}

/**
 * Formats an integer value and appends it to the log stream.
 *
 * @tparam I The type of the integer value.
 * @param v The integer value to be formatted and appended.
 *
 * @throws None
 */
template <typename I>
void LogStream::formatInteger(I v) {
  constexpr static int kMaxNumericSize = std::numeric_limits<I>::digits10 + 4;

  if (exBuffer_.empty()) {
    if (buffer_.avail() >= kMaxNumericSize) {
      size_t len = utils::convertInt(buffer_.current(), v);
      buffer_.add(len);
      return;
    } else {
      exBuffer_.append(buffer_.data(), buffer_.length());
    }
  }

  auto oldLen = exBuffer_.length();
  exBuffer_.resize(oldLen + kMaxNumericSize);
  size_t len = utils::convertInt(&exBuffer_[oldLen], v);
  exBuffer_.resize(oldLen + len);
}

LogStream &LogStream::operator<<(bool v) {
  buffer_.append(v ? "1" : "0", 1);
  return *this;
}

LogStream &LogStream::operator<<(short v) {
  *this << static_cast<int>(v);
  return *this;
}

LogStream &LogStream::operator<<(unsigned short v) {
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream &LogStream::operator<<(int v) {
  formatInteger(v);
  return *this;
}

LogStream &LogStream::operator<<(unsigned int v) {
  formatInteger(v);
  return *this;
}

LogStream &LogStream::operator<<(long v) {
  formatInteger(v);
  return *this;
}

LogStream &LogStream::operator<<(unsigned long v) {
  formatInteger(v);
  return *this;
}

LogStream &LogStream::operator<<(const long long &v) {
  formatInteger(v);
  return *this;
}

LogStream &LogStream::operator<<(const unsigned long long &v) {
  formatInteger(v);
  return *this;
}

LogStream &LogStream::operator<<(float &v) {
  *this << static_cast<double>(v);
  return *this;
}

// TODO: replace this with Grisu3 by Florian Loitsch.
LogStream &LogStream::operator<<(const double &v) {
  constexpr static int kMaxNumericSize = 32;
  if (exBuffer_.empty()) {
    if (buffer_.avail() >= kMaxNumericSize) {
      int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
      buffer_.add(len);
      return *this;
    } else {
      exBuffer_.append(buffer_.data(), buffer_.length());
    }
  }

  auto oldLen = exBuffer_.length();
  exBuffer_.resize(oldLen + kMaxNumericSize);
  int len = snprintf(&(exBuffer_[oldLen]), kMaxNumericSize, "%.12g", v);
  exBuffer_.resize(oldLen + len);
  return *this;
}

LogStream &LogStream::operator<<(const long double &v) {
  constexpr static int kMaxNumericSize = 48;
  if (exBuffer_.empty()) {
    if (buffer_.avail() >= kMaxNumericSize) {
      int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12Lg", v);
      buffer_.add(len);
      return *this;
    } else {
      exBuffer_.append(buffer_.data(), buffer_.length());
    }
  }

  auto oldLen = exBuffer_.length();
  exBuffer_.resize(oldLen + kMaxNumericSize);
  int len = snprintf(&(exBuffer_[oldLen]), kMaxNumericSize, "%.12Lg", v);
  exBuffer_.resize(oldLen + len);
  return *this;
}

LogStream &LogStream::operator<<(char v) {
  append(&v, 1);
  return *this;
}

LogStream &LogStream::operator<<(const void *p) {
  uintptr_t            v               = reinterpret_cast<uintptr_t>(p);
  constexpr static int kMaxNumericSize = std::numeric_limits<uintptr_t>::digits / 4 + 4;

  if (exBuffer_.empty()) {
    if (buffer_.avail() >= kMaxNumericSize) {
      char *buf  = buffer_.current();
      buf[0]     = '0';
      buf[1]     = 'x';
      size_t len = trantor::utils::convertHex(buf + 2, v);
      buffer_.add(len + 2);
      return *this;
    } else {
      exBuffer_.append(buffer_.data(), buffer_.length());
    }
  }

  auto oldLen = exBuffer_.length();
  exBuffer_.resize(oldLen + kMaxNumericSize);
  char *buf  = &exBuffer_[oldLen];
  buf[0]     = '0';
  buf[1]     = 'x';
  size_t len = trantor::utils::convertHex(buf + 2, v);
  exBuffer_.resize(oldLen + len + 2);
  return *this;
}

LogStream &LogStream::operator<<(char *str) {
  if (str) {
    append(str, strlen(str));
  } else {
    append("(null)", 6);
  }
  return *this;
}

LogStream &LogStream::operator<<(const char *str) {
  if (str) {
    append(str, strlen(str));
  } else {
    append("(null)", 6);
  }
  return *this;
}

LogStream &LogStream::operator<<(const unsigned char *str) {
  return operator<<(reinterpret_cast<const char *>(str));
}

LogStream &LogStream::operator<<(const std::string &v) {
  append(v.c_str(), v.size());
  return *this;
}

LogStream &LogStream::operator<<(std::string_view sv) {
  append(sv.data(), sv.size());
  return *this;
}

LogStream &LogStream::operator<<(const Fmt &fmt) {
  append(fmt.data(), fmt.length());
  return *this;
}

LogStream &LogStream::operator<<(const SourceFile &v) {
  append(v.data_, v.size_);
  return *this;
}

}  // namespace trantor
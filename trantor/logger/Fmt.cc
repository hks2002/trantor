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

#include "Fmt.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

namespace trantor {

/**
 * @brief Constructs a new Fmt object by formatting the given value using the provided format string.
 *
 * @param fmt The format string to use for formatting.
 * @param val The value to format.
 *
 * @throws None
 */
template <typename T>
Fmt::Fmt(const char *fmt, T val) {
  length_ = snprintf(buf_, sizeof buf_, fmt, val);
  assert(static_cast<size_t>(length_) < sizeof buf_);
}

/**
 * @brief Get a pointer to the data in the Fmt object.
 *
 * @return const char* Pointer to the data in the Fmt object
 */
const char *Fmt::data() const {
  return buf_;
}

/**
 * @brief Get the length of the formatted string.
 *
 * @return The length of the formatted string.
 */
int Fmt::length() const {
  return length_;
}

// Explicit instantiations for common types to avoid compiler warnings
template TRANTOR_EXPORT Fmt::Fmt(const char *fmt, char);

template TRANTOR_EXPORT Fmt::Fmt(const char *fmt, short);
template TRANTOR_EXPORT Fmt::Fmt(const char *fmt, unsigned short);
template TRANTOR_EXPORT Fmt::Fmt(const char *fmt, int);
template TRANTOR_EXPORT Fmt::Fmt(const char *fmt, unsigned int);
template TRANTOR_EXPORT Fmt::Fmt(const char *fmt, long);
template TRANTOR_EXPORT Fmt::Fmt(const char *fmt, unsigned long);
template TRANTOR_EXPORT Fmt::Fmt(const char *fmt, long long);
template TRANTOR_EXPORT Fmt::Fmt(const char *fmt, unsigned long long);

template TRANTOR_EXPORT Fmt::Fmt(const char *fmt, float);
template TRANTOR_EXPORT Fmt::Fmt(const char *fmt, double);

}  // namespace trantor
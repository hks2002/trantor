/**
 *
 *  @file FixedBuffer.cc
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
#include "FixedBuffer.h"

#include <memory>

namespace trantor {
namespace detail {

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

/**
 * @brief Returns the buffer as a constant pointer.
 *
 * @return a constant pointer to buffer
 */
template <size_t SIZE>
const char *FixedBuffer<SIZE>::data() const {
  return data_;
}

/**
 * @brief Returns the end of the buffer as a constant pointer.
 *
 * @return a constant pointer to the end of the buffer
 */
template <size_t SIZE>
const char *FixedBuffer<SIZE>::end() const {
  return data_ + sizeof data_;
}

/**
 * @brief Returns the current used length of the buffer.
 *
 * @return the current used length of the buffer
 */
template <size_t SIZE>
size_t FixedBuffer<SIZE>::length() const {
  return static_cast<size_t>(cur_ - data_);
}

/**
 * @brief Returns the available length of the buffer.
 *
 * @return the available length of the buffer
 */
template <size_t SIZE>
size_t FixedBuffer<SIZE>::avail() const {
  return static_cast<size_t>(end() - cur_);
}

/**
 * @brief Returns a pointer to the current position of the buffer.
 *
 * @return a pointer to the current position of the buffer
 */
template <size_t SIZE>
char *FixedBuffer<SIZE>::current() {
  return cur_;
}

/**
 * @brief Add a specified length to the current position.
 *
 * @param len the length to add
 */
template <size_t SIZE>
void FixedBuffer<SIZE>::add(size_t len) {
  cur_ += len;
}

/**
 * @brief append data to buffer
 *
 * @param buf data to append
 * @param len data length
 * @return true if success
 */
template <size_t SIZE>
bool FixedBuffer<SIZE>::append(const char * /*restrict*/ buf, size_t len) {
  if ((size_t)(avail()) > len) {
    memcpy(cur_, buf, len);
    cur_ += len;
    return true;
  }
  return false;
}

/**
 * @brief Resets the current position to the beginning of the buffer.
 */
template <size_t SIZE>
void FixedBuffer<SIZE>::reset() {
  cur_ = data_;
}

/**
 * @brief Set whole buffer content to 0.
 */
template <size_t SIZE>
void FixedBuffer<SIZE>::zeroBuffer() {
  memset(data_, 0, sizeof(data_));
}

/**
 * @brief Set the cookie function pointer.
 *
 * @param cookie pointer to the cookie function
 *
 * @return void
 */
template <size_t SIZE>
void FixedBuffer<SIZE>::setCookie(void (*cookie)()) {
  cookie_ = cookie;
}

/**
 * @brief Returns the debug string of the buffer.for used by unit test.
 */
template <size_t SIZE>
std::string FixedBuffer<SIZE>::toString() const {
  return std::string(data_, length());
}

/**
 * @brief Returns the debug string of the buffer.for used by GDB.
 */
template <size_t SIZE>
const char *FixedBuffer<SIZE>::debugString() {
  *cur_ = '\0';
  return data_;
}

template <size_t SIZE>
void FixedBuffer<SIZE>::cookieStart() {}

template <size_t SIZE>
void FixedBuffer<SIZE>::cookieEnd() {}
}  // namespace detail
}  // namespace trantor
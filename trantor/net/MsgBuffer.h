/**
 *
 *  @file MsgBuffer.h
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

#ifndef TRANTOR_MSG_BUFFER_H
#define TRANTOR_MSG_BUFFER_H

#include "trantor/NonCopyable.h"
#include "trantor/exports.h"
#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#if defined(_WIN32) && !defined(_SSIZE_T_DEFINED)
using ssize_t = std::intptr_t;
#endif

namespace trantor {
static constexpr size_t kBufferDefaultLength{2048};
static constexpr char   CRLF[]{"\r\n"};

/**
 * @brief This class represents a memory buffer used for sending and receiving
 * data.
 *
 */
class TRANTOR_EXPORT MsgBuffer {
public:
  explicit MsgBuffer(size_t len = kBufferDefaultLength);

  const char *beginWrite() const;
  char       *beginWrite();

  size_t      readableBytes() const;
  size_t      writableBytes() const;
  void        ensureWritableBytes(size_t len);

  const char *peek() const;
  uint8_t     peekInt8() const;
  uint16_t    peekInt16() const;
  uint32_t    peekInt32() const;
  uint64_t    peekInt64() const;

  void        retrieve(size_t len);
  void        retrieveUntil(const char *end);
  void        retrieveAll();

  std::string read(size_t len);
  uint8_t     readInt8();
  uint16_t    readInt16();
  uint32_t    readInt32();
  uint64_t    readInt64();

  void        addInFront(const char *buf, size_t len);
  void        addInFrontInt8(const uint8_t b);
  void        addInFrontInt16(const uint16_t s);
  void        addInFrontInt32(const uint32_t i);
  void        addInFrontInt64(const uint64_t l);

  void        appendInt8(const uint8_t b);
  void        appendInt16(const uint16_t s);
  void        appendInt32(const uint32_t i);
  void        appendInt64(const uint64_t l);

  template <int N>
  void append(const char (&buf)[N]) {
    assert(strnlen(buf, N) == N - 1);
    append(buf, N - 1);
  }
  void        append(const char *buf, size_t len);
  void        append(const std::string &buf);
  void        append(const MsgBuffer &buf);

  void        hasWritten(size_t len);
  void        unwrite(size_t offset);

  ssize_t     readFd(int fd, int *retErrno);

  const char *findCRLF() const;
  void        swap(MsgBuffer &buf) noexcept;

  const char &operator[](size_t offset) const;
  char       &operator[](size_t offset);

private:
  size_t            head_;
  size_t            initCap_;
  std::vector<char> buffer_;
  size_t            tail_;
  const char       *begin() const;
  char             *begin();
};

/**
 * Swaps the contents of two MsgBuffer objects.
 *
 * @param one The first MsgBuffer object to swap.
 * @param two The second MsgBuffer object to swap.
 *
 * @throws None
 */
inline void swap(MsgBuffer &one, MsgBuffer &two) noexcept {
  one.swap(two);
}
}  // namespace trantor

namespace std {
/**
 * Swaps the contents of two `MsgBuffer` objects.
 *
 * @param one The first `MsgBuffer` object.
 * @param two The second `MsgBuffer` object.
 *
 * @throws None.
 */
template <>
inline void swap(trantor::MsgBuffer &one, trantor::MsgBuffer &two) noexcept {
  one.swap(two);
}
}  // namespace std

#endif  // TRANTOR_MSG_BUFFER_H

/**
 *
 *  MsgBuffer.cc
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

#include "MsgBuffer.h"

#include "trantor/utils/Encoding.h"
#include <assert.h>
#include <errno.h>
#include <string.h>

#ifndef _WIN32
#include <netinet/in.h>
#include <sys/uio.h>
#else
#include "WindowsSupport.h"
#include <winsock2.h>
#endif

namespace trantor {
static constexpr size_t kBufferOffset{8};

/**
 * @brief Construct a new message buffer instance.
 *
 * @param len The initial size of the buffer.
 */
MsgBuffer::MsgBuffer(size_t len) : head_(kBufferOffset), initCap_(len), buffer_(len + head_), tail_(head_) {}

/**
 * Returns a pointer to the beginning of the buffer.
 *
 * @return const char*
 */
const char *MsgBuffer::begin() const {
  return &buffer_[0];
}

/**
 * Returns a pointer to the beginning of the buffer.
 *
 * @return char*
 */
char *MsgBuffer::begin() {
  return &buffer_[0];
}

/**
 * @brief Get the end of the buffer where new data can be written.
 *
 * @return const char*
 */
const char *MsgBuffer::beginWrite() const {
  return begin() + tail_;
}

/**
 * @brief Get the end of the buffer where new data can be written.
 *
 * @return char*
 */
char *MsgBuffer::beginWrite() {
  return begin() + tail_;
}

/**
 * @brief Return the size of the data in the buffer.
 *
 * @return size_t
 */
size_t MsgBuffer::readableBytes() const {
  return tail_ - head_;
}

/**
 * @brief Return the size of the empty part in the buffer
 *
 * @return size_t
 */
size_t MsgBuffer::writableBytes() const {
  return buffer_.size() - tail_;
}

/**
 * @brief Make sure the buffer has enough spaces to write data.
 *
 * @param len
 */
void MsgBuffer::ensureWritableBytes(size_t len) {
  if (writableBytes() >= len) return;
  if (head_ + writableBytes() >= (len + kBufferOffset))  // move readable bytes
  {
    std::copy(begin() + head_, begin() + tail_, begin() + kBufferOffset);
    tail_ = kBufferOffset + (tail_ - head_);
    head_ = kBufferOffset;
    return;
  }
  // create new buffer
  size_t newLen;
  if ((buffer_.size() * 2) > (kBufferOffset + readableBytes() + len))
    newLen = buffer_.size() * 2;
  else
    newLen = kBufferOffset + readableBytes() + len;
  MsgBuffer newbuffer(newLen);
  newbuffer.append(*this);
  swap(newbuffer);
}

/**
 * @brief Get the beginning of the buffer.
 *
 * @return const char*
 */
const char *MsgBuffer::peek() const {
  return begin() + head_;
}

/**
 * @brief Get a byte value from the buffer.
 *
 * @return uint8_t
 */
uint8_t MsgBuffer::peekInt8() const {
  assert(readableBytes() >= 1);
  return *(static_cast<const uint8_t *>((void *)peek()));
}

/**
 * @brief Get a unsigned short value from the buffer.
 *
 * @return uint16_t
 */
uint16_t MsgBuffer::peekInt16() const {
  assert(readableBytes() >= 2);
  uint16_t rs = *(static_cast<const uint16_t *>((void *)peek()));
  return ntohs(rs);
}

/**
 * @brief Get a unsigned int value from the buffer.
 *
 * @return uint32_t
 */
uint32_t MsgBuffer::peekInt32() const {
  assert(readableBytes() >= 4);
  uint32_t rl = *(static_cast<const uint32_t *>((void *)peek()));
  return ntohl(rl);
}

/**
 * @brief Get a unsigned int64 value from the buffer.
 *
 * @return uint64_t
 */
uint64_t MsgBuffer::peekInt64() const {
  assert(readableBytes() >= 8);
  uint64_t rll = *(static_cast<const uint64_t *>((void *)peek()));
  return trantor::utils::ntoh64(rll);
}

/**
 * @brief Remove some bytes in the buffer.
 *
 * @param len
 */
void MsgBuffer::retrieve(size_t len) {
  if (len >= readableBytes()) {
    retrieveAll();
    return;
  }
  head_ += len;
}

/**
 * @brief Remove all data in the buffer.
 *
 */
void MsgBuffer::retrieveAll() {
  if (buffer_.size() > (initCap_ * 2)) {
    buffer_.resize(initCap_);
  }
  tail_ = head_ = kBufferOffset;
}

/**
 * @brief Remove the data before a certain position from the buffer.
 *
 * @param end The position.
 */
void MsgBuffer::retrieveUntil(const char *end) {
  assert(peek() <= end);
  assert(end <= beginWrite());
  retrieve(end - peek());
}

/**
 * @brief Get and remove some bytes from the buffer.
 *
 * @param len
 * @return std::string
 */
std::string MsgBuffer::read(size_t len) {
  if (len > readableBytes()) len = readableBytes();
  std::string ret(peek(), len);
  retrieve(len);
  return ret;
}

/**
 * @brief Get the remove a byte value from the buffer.
 *
 * @return uint8_t
 */
uint8_t MsgBuffer::readInt8() {
  uint8_t ret = peekInt8();
  retrieve(1);
  return ret;
}

/**
 * @brief Get and remove a unsigned short value from the buffer.
 *
 * @return uint16_t
 */
uint16_t MsgBuffer::readInt16() {
  uint16_t ret = peekInt16();
  retrieve(2);
  return ret;
}

/**
 * @brief Get and remove a unsigned int value from the buffer.
 *
 * @return uint32_t
 */
uint32_t MsgBuffer::readInt32() {
  uint32_t ret = peekInt32();
  retrieve(4);
  return ret;
}

/**
 * @brief Get and remove a unsigned int64 value from the buffer.
 *
 * @return uint64_t
 */
uint64_t MsgBuffer::readInt64() {
  uint64_t ret = peekInt64();
  retrieve(8);
  return ret;
}

/**
 * @brief Put new data to the beginning of the buffer.
 *
 * @param buf
 * @param len
 */
void MsgBuffer::addInFront(const char *buf, size_t len) {
  if (head_ >= len) {
    memcpy(begin() + head_ - len, buf, len);
    head_ -= len;
    return;
  }
  if (len <= writableBytes()) {
    std::copy(begin() + head_, begin() + tail_, begin() + head_ + len);
    memcpy(begin() + head_, buf, len);
    tail_ += len;
    return;
  }
  size_t newLen;
  if (len + readableBytes() < initCap_)
    newLen = initCap_;
  else
    newLen = len + readableBytes();
  MsgBuffer newBuf(newLen);
  newBuf.append(buf, len);
  newBuf.append(*this);
  swap(newBuf);
}

/**
 * @brief Put a byte value to the beginning of the buffer.
 *
 * @param b
 */
void MsgBuffer::addInFrontInt8(const uint8_t b) {
  addInFront(static_cast<const char *>((void *)&b), 1);
}

/**
 * @brief Put a unsigned short value to the beginning of the buffer.
 *
 * @param s
 */
void MsgBuffer::addInFrontInt16(const uint16_t s) {
  uint16_t ss = htons(s);
  addInFront(static_cast<const char *>((void *)&ss), 2);
}

/**
 * @brief Put a unsigned int value to the beginning of the buffer.
 *
 * @param i
 */
void MsgBuffer::addInFrontInt32(const uint32_t i) {
  uint32_t ii = htonl(i);
  addInFront(static_cast<const char *>((void *)&ii), 4);
}

/**
 * @brief Put a unsigned int64 value to the beginning of the buffer.
 *
 * @param l
 */
void MsgBuffer::addInFrontInt64(const uint64_t l) {
  uint64_t ll = trantor::utils::hton64(l);
  addInFront(static_cast<const char *>((void *)&ll), 8);
}

/**
 * @brief Append a byte value to the end of the buffer.
 *
 * @param b
 */
void MsgBuffer::appendInt8(const uint8_t b) {
  append(static_cast<const char *>((void *)&b), 1);
}

/**
 * @brief Append a unsigned short value to the end of the buffer.
 *
 * @param s
 */
void MsgBuffer::appendInt16(const uint16_t s) {
  uint16_t ss = htons(s);
  append(static_cast<const char *>((void *)&ss), 2);
}

/**
 * @brief Append a unsigned int value to the end of the buffer.
 *
 * @param i
 */
void MsgBuffer::appendInt32(const uint32_t i) {
  uint32_t ii = htonl(i);
  append(static_cast<const char *>((void *)&ii), 4);
}

/**
 * @brief Append a unsigned int64 value to the end of the buffer.
 *
 * @param l
 */
void MsgBuffer::appendInt64(const uint64_t l) {
  uint64_t ll = trantor::utils::hton64(l);
  append(static_cast<const char *>((void *)&ll), 8);
}

/**
 * Appends data to the message buffer.
 *
 * @param buf Pointer to the data to be appended.
 * @param len The length of the data to append.
 *
 * @throws None
 */
void MsgBuffer::append(const char *buf, size_t len) {
  ensureWritableBytes(len);
  memcpy(&buffer_[tail_], buf, len);
  tail_ += len;
}

/**
 * Appends a string to the message buffer.
 *
 * @param buf The string to append.
 *
 * @throws None
 */
void MsgBuffer::append(const std::string &buf) {
  append(buf.c_str(), buf.length());
}

/**
 * @brief Append new data to the buffer.
 *
 */
void MsgBuffer::append(const MsgBuffer &buf) {
  ensureWritableBytes(buf.readableBytes());
  memcpy(&buffer_[tail_], buf.peek(), buf.readableBytes());
  tail_ += buf.readableBytes();
}

/**
 * @brief Move the write pointer forward when the new data has been written
 * to the buffer.
 *
 * @param len
 */
void MsgBuffer::hasWritten(size_t len) {
  assert(len <= writableBytes());
  tail_ += len;
}

/**
 * @brief Move the write pointer backward to remove data in the end of the
 * buffer.
 *
 * @param offset
 */
void MsgBuffer::unwrite(size_t offset) {
  assert(readableBytes() >= offset);
  tail_ -= offset;
}

/**
 * @brief Read data from a file descriptor and put it into the buffer.˝
 *
 * @param fd The file descriptor. It is usually a socket.
 * @param retErrno The error code when reading.
 * @return ssize_t The number of bytes read from the file descriptor. -1 is
 * returned when an error occurs.
 */
ssize_t MsgBuffer::readFd(int fd, int *retErrno) {
  char         extBuffer[8192];
  struct iovec vec[2];
  size_t       writable = writableBytes();
  vec[0].iov_base       = begin() + tail_;
  vec[0].iov_len        = static_cast<int>(writable);
  vec[1].iov_base       = extBuffer;
  vec[1].iov_len        = sizeof(extBuffer);
  const int iovcnt      = (writable < sizeof extBuffer) ? 2 : 1;
  ssize_t   n           = ::readv(fd, vec, iovcnt);
  if (n < 0) {
    *retErrno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    tail_ += n;
  } else {
    tail_ = buffer_.size();
    append(extBuffer, n - writable);
  }
  return n;
}

/**
 * @brief Find the position of the buffer where the CRLF is found.
 *
 * @return const char*
 */
const char *MsgBuffer::findCRLF() const {
  const char *crlf = std::search(peek(), beginWrite(), CRLF, CRLF + 2);
  return crlf == beginWrite() ? NULL : crlf;
}

/**
 * @brief swap the buffer with another.
 *
 * @param buf
 */
void MsgBuffer::swap(MsgBuffer &buf) noexcept {
  buffer_.swap(buf.buffer_);
  std::swap(head_, buf.head_);
  std::swap(tail_, buf.tail_);
  std::swap(initCap_, buf.initCap_);
}

/**
 * @brief Access a byte in the buffer.
 *
 * @param offset
 * @return const char&
 */
const char &MsgBuffer::operator[](size_t offset) const {
  assert(readableBytes() >= offset);
  return peek()[offset];
}

/**
 * @brief Access a byte in the buffer.
 *
 * @param offset
 * @return const char&
 */
char &MsgBuffer::operator[](size_t offset) {
  assert(readableBytes() >= offset);
  return begin()[head_ + offset];
}

}  // namespace trantor
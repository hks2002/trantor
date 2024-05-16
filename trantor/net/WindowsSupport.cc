/**
 *
 *  @file WindowsSupport.cc
 *  @author An Tao
 *
 *  Implementation of Windows support functions.
 *
 *  Copyright 2018, An Tao.  All rights reserved.
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the License file.
 *
 *
 */

#include "WindowsSupport.h"

#include <cerrno>
#include <winsock2.h>

/**
 * @brief Read data from a socket.
 * from polipo
 *
 * @param fd The file descriptor of the socket.
 * @param buf The buffer to store the received data.
 * @param n The maximum number of bytes to read.
 *
 * @return The number of bytes read, or -1 if an error occurred.
 *
 * @throws None
 */
static inline int win32_read_socket(int fd, void *buf, int n) {
  int rc = recv(fd, reinterpret_cast<char *>(buf), n, 0);
  if (rc == SOCKET_ERROR) {
    _set_errno(WSAGetLastError());
  }
  return rc;
}

/**
 * @brief Read data from a file descriptor into multiple buffers in a scatter-gather fashion.
 *
 * @param fd the file descriptor to read from
 * @param vector an array of iovec structures specifying the buffers
 * @param count the number of iovec structures in the array
 *
 * @return the total number of bytes read
 *
 * @throws None
 */
int readv(int fd, const struct iovec *vector, int count) {
  int ret = 0; /* Return value */
  int i;
  for (i = 0; i < count; i++) {
    int n  = vector[i].iov_len;
    int rc = win32_read_socket(fd, vector[i].iov_base, n);
    if (rc == n) {
      ret += rc;
    } else {
      if (rc < 0) {
        ret = (ret == 0 ? rc : ret);
      } else {
        ret += rc;
      }
      break;
    }
  }
  return ret;
}

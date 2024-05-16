/**
 *
 *  @file Encoding.h
 *  @author An Tao
 *
 *  Copyright 2018, An Tao.  All rights reserved.
 *  https://github.com/an-tao/drogon
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Drogon
 *
 */

#pragma once

#include "trantor/exports.h"
#include <cstdint>
#include <string>
#include <vector>

namespace trantor {
/**
 * @brief Trantor encode helper functions.
 *
 */
namespace utils {

TRANTOR_EXPORT std::wstring fromUtf8(const std::string &str);
TRANTOR_EXPORT std::string toUtf8(const std::wstring &wstr);
TRANTOR_EXPORT std::string fromWidePath(const std::wstring &strPath);
TRANTOR_EXPORT std::wstring toWidePath(const std::string &strUtf8Path);

// Helpers for multi-platform development
// OS dependent
#if defined(_WIN32) && !defined(__MINGW32__)
/**
 * @details Convert an UTF-8 path to a native path.
 *
 * This is a helper for Windows and multi-platform projects.
 *
 * Converts the path from UTF-8 to wide string and replaces slash directory
 * separators with backslash.
 *
 * @remarks Although it accepts both slash and backslash as directory
 * separators in its API, it is better to stick to its standard.
 *
 * @param strPath UTF-8 path.
 *
 * @return Native path suitable for the Windows unicode API.
 */
inline std::wstring toNativePath(const std::string &strPath) {
  return toWidePath(strPath);
}

/**
 * @details Convert an UTF-8 path to a native path.
 *
 * This is a helper for non-Windows OSes for multi-platform projects.
 *
 * Does nothing.
 *
 * @param strPath UTF-8 path.
 *
 * @return \p strPath.
 */
inline const std::wstring &toNativePath(const std::wstring &strPath) {
  return strPath;
}
#else   // __WIN32

/**
 * @details Convert an UTF-8 path to a native path.
 *
 * This is a helper for non-Windows OSes for multi-platform projects.
 *
 * Does nothing.
 *
 * @param strPath UTF-8 path.
 *
 * @return \p strPath.
 */
inline const std::string &toNativePath(const std::string &strPath) {
  return strPath;
}

/**
 * @details Convert an wide string path to a UTF-8 path.
 *
 * This is a helper, mainly for Windows and multi-platform projects.
 *
 * @note On Windows, backslash directory separators are converted to slash to
 * keep portable paths.

 * @warning On other OSes, backslashes are not converted to slash, since they
 * are valid characters for directory/file names.
 *
 * @param strPath wide string path.
 *
 * @return Generic path, with slash directory separators
 */
inline std::string toNativePath(const std::wstring &strPath) {
  return fromWidePath(strPath);
}
#endif  // _WIN32

/**
 * @note NoOP on all OSes
 */
inline const std::string &fromNativePath(const std::string &strPath) {
  return strPath;
}

/**
 * @note fromWidePath() on all OSes
 */
// Convert on all systems
inline std::string fromNativePath(const std::wstring &strPath) {
  return fromWidePath(strPath);
}

/**
 * Converts a 64-bit unsigned integer from host byte order to network byte order.
 *
 * @param n The 64-bit unsigned integer to be converted.
 *
 * @return The converted 64-bit unsigned integer in network byte order.
 *
 * @throws None
 */
inline uint64_t hton64(uint64_t n) {
  static const int  one = 1;
  static const char sig = *(char *)&one;
  if (sig == 0) return n;  // for big endian machine just return the input
  char *ptr = reinterpret_cast<char *>(&n);
  std::reverse(ptr, ptr + sizeof(uint64_t));
  return n;
}

/**
 * Converts a 64-bit unsigned integer from network byte order to host byte
 * order.
 *
 * @param n The 64-bit unsigned integer to be converted.
 *
 * @return The converted 64-bit unsigned integer in host byte order.
 *
 * @throws None
 */
inline uint64_t ntoh64(uint64_t n) {
  return hton64(n);
}

}  // namespace utils

}  // namespace trantor

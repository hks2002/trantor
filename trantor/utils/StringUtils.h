/**
 *  @file StringUtils.h
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

#ifndef TRANTOR_STRING_UTILS_H
#define TRANTOR_STRING_UTILS_H

#include "trantor/exports.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace trantor {
/**
 * @brief provide some string functions.
 *
 */
namespace utils {
template <typename T>
size_t                          convertInt(char buf[], T value);
size_t                          convertHex(char buf[], uintptr_t value);
inline std::vector<std::string> splitString(const std::string &s,
                                            const std::string &delimiter,
                                            bool               acceptEmptyString = false) {
  if (delimiter.empty()) return std::vector<std::string>{};
  std::vector<std::string> v;
  size_t                   last = 0;
  size_t                   next = 0;
  while ((next = s.find(delimiter, last)) != std::string::npos) {
    if (next > last || acceptEmptyString) v.push_back(s.substr(last, next - last));
    last = next + delimiter.length();
  }
  if (s.length() > last || acceptEmptyString) v.push_back(s.substr(last));
  return v;
}
bool           verifySslName(std::string_view certName, std::string_view hostName);
TRANTOR_EXPORT std::string tlsBackend();
TRANTOR_EXPORT bool        secureRandomBytes(void *ptr, size_t size);

}  // namespace utils
}  // namespace trantor
#endif  // TRANTOR_STRING_UTILS_H
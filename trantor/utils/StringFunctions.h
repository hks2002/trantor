/**
 *  @file StringFunctions.h
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

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "trantor/exports.h"

namespace trantor {
/*/**
 * @brief provide same string functions.
 *
 */
namespace utils {

bool           verifySslName(std::string_view certName, std::string_view hostName);
TRANTOR_EXPORT std::string tlsBackend();
TRANTOR_EXPORT bool        secureRandomBytes(void *ptr, size_t size);
/**
 * Splits a string into a vector of substrings based on a delimiter.
 *
 * @param sv The string to be split.
 * @param delimiter The delimiter used to split the string.
 * @param acceptEmptyString (optional) Whether to accept empty strings as separate elements. Default is false.
 *
 * @return A vector of strings containing the substrings.
 *
 * @throws None
 */
inline std::vector<std::string> splitString(std::string_view sv,
                                            std::string_view delimiter,
                                            bool             acceptEmptyString = false) {
  std::vector<std::string> v;
  size_t                   last = 0;
  size_t                   next = 0;

  if (delimiter.empty()) {
    return v;
  }

  while ((next = sv.find(delimiter, last)) != std::string::npos) {
    if (next > last || acceptEmptyString) {
      v.push_back(std::string(sv.substr(last, next - last)));
    }
    last = next + delimiter.length();
  }
  if (sv.length() > last || acceptEmptyString) {
    v.push_back(std::string(sv.substr(last)));
  }
  return v;
}

}  // namespace utils
}  // namespace trantor
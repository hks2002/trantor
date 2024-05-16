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
size_t                   convertInt(char buf[], T value);
size_t                   convertHex(char buf[], uintptr_t value);
std::vector<std::string> splitString(std::string_view sv, std::string_view delimiter, bool acceptEmptyString = false);

bool                     verifySslName(std::string_view certName, std::string_view hostName);
TRANTOR_EXPORT std::string tlsBackend();
TRANTOR_EXPORT bool        secureRandomBytes(void *ptr, size_t size);

}  // namespace utils
}  // namespace trantor
#endif  // TRANTOR_STRING_UTILS_H
/**
 *
 *  @file Hash.h
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

#ifndef TRANTOR_HASH_H
#define TRANTOR_HASH_H

#include "trantor/exports.h"
#include <cstring>
#include <string>
#include <string_view>

namespace trantor {
/**
 * @brief Provide same hash functions so users don't have to provide their own.
 *
 */
namespace utils {

struct Hash128 {
  unsigned char bytes[16];
};
struct Hash160 {
  unsigned char bytes[20];
};
struct Hash256 {
  unsigned char bytes[32];
};

Hash128     md5(const void *data, size_t len);
Hash160     sha1(const void *data, size_t len);
Hash256     sha256(const void *data, size_t len);
Hash256     sha3(const void *data, size_t len);
Hash256     blake2b(const void *data, size_t len);
std::string toHexString(const void *data, size_t len);

/**
 * @brief Compute the MD5 hash of the given data
 * @note don't use MD5 for new applications. It's here only for compatibility
 */
TRANTOR_EXPORT inline Hash128 md5(std::string_view str) {
  return md5(str.data(), str.size());
}

/**
 * @brief Compute the SHA1 hash of the given data
 */
TRANTOR_EXPORT inline Hash160 sha1(std::string_view str) {
  return sha1(str.data(), str.size());
}

/**
 * @brief Compute the SHA256 hash of the given data
 */
TRANTOR_EXPORT inline Hash256 sha256(std::string_view str) {
  return sha256(str.data(), str.size());
}

/**
 * @brief Compute the SHA3 hash of the given data
 */
TRANTOR_EXPORT inline Hash256 sha3(std::string_view str) {
  return sha3(str.data(), str.size());
}

/**
 * @brief Compute the BLAKE2b hash of the given data
 * @note When in doubt, use SHA3 or BLAKE2b. Both are safe and SHA3 is faster if
 * you are using OpenSSL and it has SHA3 in hardware mode. Otherwise BLAKE2b is
 * faster in software.
 */
TRANTOR_EXPORT inline Hash256 blake2b(std::string_view str) {
  return blake2b(str.data(), str.size());
}

/**
 * @brief Convert a given hash128 to a hexadecimal string.
 */
TRANTOR_EXPORT inline std::string toHexString(const Hash128 &hash) {
  return toHexString(hash.bytes, sizeof(hash.bytes));
}

/**
 * @brief Convert a given hash160 to a hexadecimal string.
 */
TRANTOR_EXPORT inline std::string toHexString(const Hash160 &hash) {
  return toHexString(hash.bytes, sizeof(hash.bytes));
}

/**
 * @brief Convert a given hash256 to a hexadecimal string.
 */
TRANTOR_EXPORT inline std::string toHexString(const Hash256 &hash) {
  return toHexString(hash.bytes, sizeof(hash.bytes));
}

}  // namespace utils
}  // namespace trantor
#endif  // TRANTOR_HASH_H
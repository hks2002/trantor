/**
 *
 *  @file Hash.cc
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

#include "Hash.h"

#if defined(USE_OPENSSL)
#if OPENSSL_VERSION_MAJOR < 3  // openssl version < 3 using low level API
#include <crypto/blake2/blake2_local.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#else  // openssl version >= 3 using high level API
#include <openssl/evp.h>
#endif
#endif

#if defined(USE_BOTAN)
#include <botan/hash.h>
#endif

#if !defined(USE_BOTAN) && !defined(USE_OPENSSL)
#include "crypto/blake2.h"
#include "crypto/md5.h"
#include "crypto/sha1.h"
#include "crypto/sha256.h"
#include "crypto/sha3.h"
#endif

namespace trantor {
namespace utils {
/**
 * @brief Calculates the MD5 hash of the input data.
 *
 * @param data pointer to the data to be hashed
 * @param len length of the data
 *
 * @return the MD5 hash as a Hash128 object
 *
 * @throws None
 */
Hash128 md5(const void *data, size_t len) {
  Hash128 hash;

#if defined(USE_OPENSSL)
#if OPENSSL_VERSION_MAJOR < 3  // openssl version < 3 using low level API
  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx, data, len);
  MD5_Final(hash, &ctx);
#else  // openssl version >= 3 using high level API
  auto md5 = EVP_MD_fetch(nullptr, "MD5", nullptr);
  auto ctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(ctx, md5, nullptr);
  EVP_DigestFinal_ex(ctx, (unsigned char *)&hash, nullptr);
  EVP_MD_CTX_free(ctx);
  EVP_MD_free(md5);
#endif
#elif defined(USE_BOTAN)
  Hash128 md5(const void *data, size_t len) {
    auto md5 = Botan::HashFunction::create("MD5");
    md5->update((const unsigned char *)data, len);
    md5->final((unsigned char *)&hash);
  }
#else
  MD5_CTX ctx;
  trantor_md5_init(&ctx);
  trantor_md5_update(&ctx, (const unsigned char *)data, len);
  trantor_md5_final(&ctx, (unsigned char *)&hash);
#endif

  return hash;
}

/**
 * @brief Calculates the SHA1 hash of the given data.
 *
 * @param data a pointer to the data to be hashed
 * @param len the length of the data in bytes
 *
 * @return the SHA1 hash of the data
 *
 * @throws None
 */
Hash160 sha1(const void *data, size_t len) {
  Hash160 hash;

#if defined(USE_OPENSSL)
#if OPENSSL_VERSION_MAJOR < 3  // openssl version < 3 using low level API
  SHA_CTX ctx;
  SHA1_Init(&ctx);
  SHA1_Update(&ctx, data, len);
  SHA1_Final(hash, &ctx);
#else  // openssl version >= 3 using high level API
  auto sha1 = EVP_MD_fetch(nullptr, "SHA1", nullptr);
  auto ctx  = EVP_MD_CTX_new();
  EVP_DigestInit_ex(ctx, sha1, nullptr);
  EVP_DigestUpdate(ctx, data, len);
  EVP_DigestFinal_ex(ctx, (unsigned char *)&hash, nullptr);
  EVP_MD_CTX_free(ctx);
  EVP_MD_free(sha1);
#endif
#elif defined(USE_BOTAN)
  auto sha1 = Botan::HashFunction::create("SHA-1");
  sha1->update((const unsigned char *)data, len);
  sha1->final((unsigned char *)&hash);
#else
  SHA1_CTX ctx;
  trantor_sha1_init(&ctx);
  trantor_sha1_update(&ctx, (const unsigned char *)data, len);
  trantor_sha1_final((unsigned char *)&hash, &ctx);
#endif

  return hash;
}

/**
 * @brief Calculates the SHA-256 hash of the given data.
 *
 * @param data pointer to the data to be hashed
 * @param len length of the data
 *
 * @return Hash256 the resulting hash value
 */
Hash256 sha256(const void *data, size_t len) {
  Hash256 hash;

#if defined(USE_OPENSSL)
#if OPENSSL_VERSION_MAJOR < 3  // openssl version < 3 using low level API
  SHA_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, data, len);
  SHA256_Final(hash, &ctx);
#else  // openssl version >= 3 using high level API
  auto sha256 = EVP_MD_fetch(nullptr, "SHA256", nullptr);
  auto ctx    = EVP_MD_CTX_new();
  EVP_DigestInit_ex(ctx, sha256, nullptr);
  EVP_DigestUpdate(ctx, data, len);
  EVP_DigestFinal_ex(ctx, (unsigned char *)&hash, nullptr);
  EVP_MD_CTX_free(ctx);
  EVP_MD_free(sha256);
#endif
#elif defined(USE_BOTAN)
  auto sha256 = Botan::HashFunction::create("SHA-256");
  sha256->update((const unsigned char *)data, len);
  sha256->final((unsigned char *)&hash);
#else
  SHA256_CTX ctx;
  trantor_sha256_init(&ctx);
  trantor_sha256_update(&ctx, (const unsigned char *)data, len);
  trantor_sha256_final(&ctx, (unsigned char *)&hash);

#endif

  return hash;
}

/**
 * @brief Calculates the SHA3-256 hash of the given data.
 *
 * @param data A pointer to the data to be hashed.
 * @param len The length of the data in bytes.
 *
 * @return The calculated SHA3-256 hash.
 */
Hash256 sha3(const void *data, size_t len) {
  Hash256 hash;
#if defined(USE_OPENSSL)
#if OPENSSL_VERSION_MAJOR < 3  // openssl version < 3 using low level API
  trantor_sha3((const unsigned char *)data, len, &hash, sizeof(hash));
#else  // openssl version >= 3 using high level API
  auto sha3 = EVP_MD_fetch(nullptr, "SHA3-256", nullptr);
  if (sha3 != nullptr) {
    auto ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, sha3, nullptr);
    EVP_DigestUpdate(ctx, data, len);
    EVP_DigestFinal_ex(ctx, (unsigned char *)&hash, nullptr);
    EVP_MD_CTX_free(ctx);
    EVP_MD_free(sha3);
  }
#endif
#elif defined(USE_BOTAN)
  auto sha3 = Botan::HashFunction::create("SHA-3(256)");
  assert(sha3 != nullptr);
  sha3->update((const unsigned char *)data, len);
  sha3->final((unsigned char *)&hash);
#else
  trantor_sha3((const unsigned char *)data, len, &hash, sizeof(hash));
#endif

  return hash;
}

/**
 * @brief Calculates the BLAKE2b hash of the given data.
 *
 * @param data pointer to the data to be hashed
 * @param len size of the data in bytes
 *
 * @return the calculated BLAKE2b hash
 */
Hash256 blake2b(const void *data, size_t len) {
  Hash256 hash;

#if defined(USE_OPENSSL)
#if OPENSSL_VERSION_MAJOR < 3  // openssl version < 3 using low level API
  BLAKE2B ctx;
  BLAKE2b_Init(&ctx);
  BLAKE2b__Update(&ctx, data, len);
  BLAKE2b__Final(hash, &ctx);
#else  // openssl version >= 3 using high level API
  auto blake2b = EVP_MD_fetch(nullptr, "BLAKE2b-256", nullptr);
  if (blake2b != nullptr) {
    auto ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, blake2b, nullptr);
    EVP_DigestUpdate(ctx, data, len);
    EVP_DigestFinal_ex(ctx, (unsigned char *)&hash, nullptr);
    EVP_MD_CTX_free(ctx);
    EVP_MD_free(blake2b);
  }
#endif
#elif defined(USE_BOTAN)
  auto blake2b = Botan::HashFunction::create("BLAKE2b(256)");
  assert(blake2b != nullptr);
  blake2b->update((const unsigned char *)data, len);
  blake2b->final((unsigned char *)&hash);
#else
  trantor_blake2b(&hash, sizeof(hash), data, len, NULL, 0);
#endif

  return hash;
}

/**
 * @brief Converts a given data buffer to a hexadecimal string.
 *
 * @param data pointer to the data buffer
 * @param len length of the data buffer
 *
 * @return a string containing the hexadecimal representation of the data
 *
 * @throws None
 */
std::string toHexString(const void *data, size_t len) {
  std::string str;
  str.resize(len * 2);
  for (size_t i = 0; i < len; i++) {
    unsigned char c = ((const unsigned char *)data)[i];
    str[i * 2]      = "0123456789ABCDEF"[c >> 4];
    str[i * 2 + 1]  = "0123456789ABCDEF"[c & 0xf];
  }
  return str;
}

}  // namespace utils

}  // namespace trantor
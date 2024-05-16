/* ================ sha1.h ================ */
/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain

Last modified by Martin Chang for the Trantor project
*/

#ifndef TRANTOR_SHA1_H
#define TRANTOR_SHA1_H

#include <cstdint>
#include <cstring>

typedef struct {
  uint32_t      state[5];
  size_t        count[2];
  unsigned char buffer[64];
} SHA1_CTX;

void trantor_sha1_transform(uint32_t state[5], const unsigned char buffer[64]);
void trantor_sha1_init(SHA1_CTX* ctx);
void trantor_sha1_update(SHA1_CTX* ctx, const unsigned char* data, size_t len);
void trantor_sha1_final(unsigned char digest[20], SHA1_CTX* ctx);

#endif  // TRANTOR_SHA1_H
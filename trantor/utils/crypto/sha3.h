// sha3.h
// 19-Nov-11  Markku-Juhani O. Saarinen <mjos@iki.fi>

#pragma once

#include <stddef.h>
#include <stdint.h>

// state context
typedef struct {
  union {             // state:
    uint8_t  b[200];  // 8-bit bytes
    uint64_t q[25];   // 64-bit words
  } st;
  int pt, rsiz, mdlen;  // these don't overflow
} SHA3_CTX;

// Compression function.
void trantor_sha3_keccakf(uint64_t st[25]);

// OpenSSL - like interface
int trantor_sha3_init(SHA3_CTX *ctx, int mdlen);  // mdlen = hash output in bytes
int trantor_sha3_update(SHA3_CTX *ctx, const void *data, size_t len);
int trantor_sha3_final(void *md, SHA3_CTX *ctx);  // digest goes to md

// compute a sha3 hash (md) of given byte length from "in"
void *trantor_sha3(const void *in, size_t inlen, void *md, int mdlen);

// SHAKE128 and SHAKE256 extensible-output functions
#define trantor_shake128_init(c) trantor_sha3_init(c, 16)
#define trantor_shake256_init(c) trantor_sha3_init(c, 32)
#define trantor_shake_update     trantor_sha3_update

void trantor_shake_xof(SHA3_CTX *ctx);
void trantor_shake_out(SHA3_CTX *ctx, void *out, size_t len);

// sha3.c
// 19-Nov-11  Markku-Juhani O. Saarinen <mjos@iki.fi>

// Revised 07-Aug-15 to match with official release of FIPS PUB 202 "SHA3"
// Revised 03-Sep-15 for portability + OpenSSL - style API

#include "sha3.h"

#ifndef KECCAKF_ROUNDS
#define KECCAKF_ROUNDS 24
#endif

#ifndef ROTL64
#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))
#endif

/**
 * update the state with given number of rounds.Performs the Keccak-f permutation on the given state array.
 *
 * @param st The state array to be permuted.
 *
 * @return void
 *
 * @throws None
 */
void trantor_sha3_keccakf(uint64_t st[25]) {
  // constants
  const uint64_t keccakf_rndc[24] = {0x0000000000000001, 0x0000000000008082, 0x800000000000808a, 0x8000000080008000,
                                     0x000000000000808b, 0x0000000080000001, 0x8000000080008081, 0x8000000000008009,
                                     0x000000000000008a, 0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
                                     0x000000008000808b, 0x800000000000008b, 0x8000000000008089, 0x8000000000008003,
                                     0x8000000000008002, 0x8000000000000080, 0x000000000000800a, 0x800000008000000a,
                                     0x8000000080008081, 0x8000000000008080, 0x0000000080000001, 0x8000000080008008};
  const int      keccakf_rotc[24] = {1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
                                     27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44};
  const int keccakf_piln[24] = {10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4, 15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1};

  // variables
  int      i, j, r;
  uint64_t t, bc[5];

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
  uint8_t *v;

  // endianess conversion. this is redundant on little-endian targets
  for (i = 0; i < 25; i++) {
    v     = (uint8_t *)&st[i];
    st[i] = ((uint64_t)v[0]) | (((uint64_t)v[1]) << 8) | (((uint64_t)v[2]) << 16) | (((uint64_t)v[3]) << 24) |
            (((uint64_t)v[4]) << 32) | (((uint64_t)v[5]) << 40) | (((uint64_t)v[6]) << 48) | (((uint64_t)v[7]) << 56);
  }
#endif

  // actual iteration
  for (r = 0; r < KECCAKF_ROUNDS; r++) {
    // Theta
    for (i = 0; i < 5; i++) bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];

    for (i = 0; i < 5; i++) {
      t = bc[(i + 4) % 5] ^ ROTL64(bc[(i + 1) % 5], 1);
      for (j = 0; j < 25; j += 5) st[j + i] ^= t;
    }

    // Rho Pi
    t = st[1];
    for (i = 0; i < 24; i++) {
      j     = keccakf_piln[i];
      bc[0] = st[j];
      st[j] = ROTL64(t, keccakf_rotc[i]);
      t     = bc[0];
    }

    //  Chi
    for (j = 0; j < 25; j += 5) {
      for (i = 0; i < 5; i++) bc[i] = st[j + i];
      for (i = 0; i < 5; i++) st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
    }

    //  Iota
    st[0] ^= keccakf_rndc[r];
  }

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
  // endianess conversion. this is redundant on little-endian targets
  for (i = 0; i < 25; i++) {
    v    = (uint8_t *)&st[i];
    t    = st[i];
    v[0] = t & 0xFF;
    v[1] = (t >> 8) & 0xFF;
    v[2] = (t >> 16) & 0xFF;
    v[3] = (t >> 24) & 0xFF;
    v[4] = (t >> 32) & 0xFF;
    v[5] = (t >> 40) & 0xFF;
    v[6] = (t >> 48) & 0xFF;
    v[7] = (t >> 56) & 0xFF;
  }
#endif
}

/**
 * Initializes the SHA3 context with the specified message digest length.
 *
 * @param ctx the SHA3 context to be initialized
 * @param mdlen the length of the message digest
 *
 * @return 1 if the initialization was successful
 *
 * @throws None
 */
int trantor_sha3_init(SHA3_CTX *ctx, int mdlen) {
  int i;

  for (i = 0; i < 25; i++) ctx->st.q[i] = 0;
  ctx->mdlen = mdlen;
  ctx->rsiz  = 200 - 2 * mdlen;
  ctx->pt    = 0;

  return 1;
}

/**
 * Updates the SHA3 context with the input data.
 *
 * @param ctx The SHA3 context to update
 * @param data Pointer to the data to update the context with
 * @param len The length of the data in bytes
 *
 * @return 1 indicating successful update
 */
int trantor_sha3_update(SHA3_CTX *ctx, const void *data, size_t len) {
  size_t i;
  int    j;

  j = ctx->pt;
  for (i = 0; i < len; i++) {
    ctx->st.b[j++] ^= ((const uint8_t *)data)[i];
    if (j >= ctx->rsiz) {
      trantor_sha3_keccakf(ctx->st.q);
      j = 0;
    }
  }
  ctx->pt = j;

  return 1;
}

/**
 * Updates the SHA3 hash state one last time and generates the final hash value.
 *
 * @param md pointer to store the final hash value
 * @param ctx pointer to the SHA3 context
 *
 * @return 1 on success
 */
int trantor_sha3_final(void *md, SHA3_CTX *ctx) {
  int i;

  ctx->st.b[ctx->pt]       ^= 0x06;
  ctx->st.b[ctx->rsiz - 1] ^= 0x80;
  trantor_sha3_keccakf(ctx->st.q);

  for (i = 0; i < ctx->mdlen; i++) {
    ((uint8_t *)md)[i] = ctx->st.b[i];
  }

  return 1;
}

/**
 * Calculates the SHA3 hash of the input data and stores the result in the
 * provided buffer.
 *
 * @param in The input data to hash.
 * @param inlen The length of the input data.
 * @param md The buffer to store the hash result.
 * @param mdlen The length of the hash result buffer.
 *
 * @return A pointer to the hash result buffer.
 *
 * @throws None.
 */
void *trantor_sha3(const void *in, size_t inlen, void *md, int mdlen) {
  SHA3_CTX sha3;

  trantor_sha3_init(&sha3, mdlen);
  trantor_sha3_update(&sha3, in, inlen);
  trantor_sha3_final(md, &sha3);

  return md;
}

/**
 * Function to shake the input XOF. SHAKE128 and SHAKE256 extensible-output functionality
 *
 * @param ctx pointer to the SHA3 context
 */
void trantor_shake_xof(SHA3_CTX *ctx) {
  ctx->st.b[ctx->pt]       ^= 0x1F;
  ctx->st.b[ctx->rsiz - 1] ^= 0x80;
  trantor_sha3_keccakf(ctx->st.q);
  ctx->pt = 0;
}

/**
 * Generates a SHA3 hash of the given data and outputs it to the provided buffer.
 *
 * @param ctx A pointer to the SHA3_CTX struct that holds the state of the hash function.
 * @param out A pointer to the buffer where the generated hash will be stored.
 * @param len The length of the buffer.
 *
 * @throws None
 */
void trantor_shake_out(SHA3_CTX *ctx, void *out, size_t len) {
  size_t i;
  int    j;

  j = ctx->pt;
  for (i = 0; i < len; i++) {
    if (j >= ctx->rsiz) {
      trantor_sha3_keccakf(ctx->st.q);
      j = 0;
    }
    ((uint8_t *)out)[i] = ctx->st.b[j++];
  }
  ctx->pt = j;
}
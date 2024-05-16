#ifndef TRANTOR_BLAKE2_H
#define TRANTOR_BLAKE2_H

#include <cstring>
void trantor_blake2b(void* output, size_t outlen, const void* input, size_t inlen, const void* key, size_t keylen);

#endif  // TRANTOR_BLAKE2_H
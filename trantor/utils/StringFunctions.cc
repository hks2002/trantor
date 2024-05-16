/**
 *  @file StringFunctions.cc
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

#include "StringFunctions.h"

#ifdef _WIN32
// clang-format off
// windows.h must be included before ntsecapi.h
#include <windows.h>
#include <ntsecapi.h>
#include <algorithm>
// clang-format on
#else  // _WIN32
#include <unistd.h>
#endif  // _WIN32

#ifdef _MSC_VER
#include <intrin.h>
#elif defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
#endif

#include <cassert>
#include <chrono>
#include <random>
#include <string>
#include <string_view>

#include "Hash.h"
#include "trantor/exports.h"

namespace trantor {
namespace utils {

/**
 * @brief Check if the name supplied by the SSL Cert matches a FQDN
 * @param certName The name supplied by the SSL Cert
 * @param hostName The FQDN to match
 *
 * @return true if matches. false otherwise
 */
bool verifySslName(std::string_view certName, std::string_view hostname) {
  if (certName.find('*') == std::string_view::npos) {
    return certName == hostname;
  }

  size_t firstDot     = certName.find('.');
  size_t hostFirstDot = hostname.find('.');
  size_t pos, len, hostPos, hostLen;

  if (firstDot != std::string_view::npos) {
    pos = firstDot + 1;
  } else {
    firstDot = pos = certName.size();
  }

  len = certName.size() - pos;

  if (hostFirstDot != std::string_view::npos) {
    hostPos = hostFirstDot + 1;
  } else {
    hostFirstDot = hostPos = hostname.size();
  }

  hostLen = hostname.size() - hostPos;

  // *. in the beginning of the cert name
  if (certName.compare(0, firstDot, "*") == 0) {
    return certName.compare(pos, len, hostname, hostPos, hostLen) == 0;
  }
  // * in the left most. but other chars in the right
  else if (certName[0] == '*') {
    // compare if `hostname` ends with `certName` but without the leftmost
    // should be fine as domain names can't be that long
    intmax_t hostnameIdx = hostname.size() - 1;
    intmax_t certNameIdx = certName.size() - 1;
    while (hostnameIdx >= 0 && certNameIdx != 0) {
      if (hostname[hostnameIdx] != certName[certNameIdx]) {
        return false;
      }
      hostnameIdx--;
      certNameIdx--;
    }
    if (certNameIdx != 0) {
      return false;
    }
    return true;
  }
  // * in the right of the first dot
  else if (firstDot != 0 && certName[firstDot - 1] == '*') {
    if (certName.compare(pos, len, hostname, hostPos, hostLen) != 0) {
      return false;
    }
    for (size_t i = 0; i < hostFirstDot && i < firstDot && certName[i] != '*'; i++) {
      if (hostname[i] != certName[i]) {
        return false;
      }
    }
    return true;
  }
  // else there's a * in  the middle
  else {
    if (certName.compare(pos, len, hostname, hostPos, hostLen) != 0) {
      return false;
    }
    for (size_t i = 0; i < hostFirstDot && i < firstDot && certName[i] != '*'; i++) {
      if (hostname[i] != certName[i]) {
        return false;
      }
    }
    intmax_t hostnameIdx = hostFirstDot - 1;
    intmax_t certNameIdx = firstDot - 1;
    while (hostnameIdx >= 0 && certNameIdx >= 0 && certName[certNameIdx] != '*') {
      if (hostname[hostnameIdx] != certName[certNameIdx]) {
        return false;
      }
      hostnameIdx--;
      certNameIdx--;
    }
    return true;
  }

  assert(false && "This line should not be reached in verifySslName");
  // should not reach
  return certName == hostname;
}

#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)
/**
 * @brief Returns the TLS backend used by trantor. Could be "None", "OpenSSL" or
 * "Botan"
 */
TRANTOR_EXPORT std::string tlsBackend() {
  return TOSTRING(TRANTOR_TLS_PROVIDER);
}
#undef TOSTRING
#undef STRINGIFY

struct RngState {
  Hash256  secret;
  Hash256  prev;
  int64_t  time;
  uint64_t counter = 0;
};

#if !defined(USE_BOTAN) && !defined(USE_OPENSSL)
/**
 * @brief Generates `size` random bytes from the systems random source and
 * stores them into `ptr`.
 * @note We only use this we no TLS backend is available. Thus we can't piggy
 * back on the TLS backend's random source.
 */
static bool systemRandomBytes(void *ptr, size_t size) {
#if defined(__BSD__) || defined(__APPLE__)
  arc4random_buf(ptr, size);
  return true;
#elif defined(__linux__) && ((defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 25))))
  return getentropy(ptr, size) != -1;
#elif defined(_WIN32)  // Windows
  return RtlGenRandom(ptr, (ULONG)size);
  // return true;
#elif defined(__unix__) || defined(__HAIKU__)
  // fallback to /dev/urandom for other/old UNIX
  thread_local std::unique_ptr<FILE, std::function<void(FILE *)> > fptr(fopen("/dev/urandom", "rb"), [](FILE *ptr) {
    if (ptr != nullptr) fclose(ptr);
  });
  if (fptr == nullptr) {
    LOG_FATAL << "Failed to open /dev/urandom for randomness";
    abort();
  }
  if (fread(ptr, 1, size, fptr.get()) != 0) return true;
#endif
  return false;
}
#endif

/**
 * @brief Generates cryptographically secure random bytes
 * @param ptr Pointer to the buffer to fill
 * @param size Size of the buffer
 * @return true if successful, false otherwise
 *
 * @note This function really shouldn't fail, but it's possible that
 *
 *   - OpenSSL can't access /dev/urandom
 *   - Compiled with glibc that supports getentropy() but the kernel doesn't
 *
 * When using Botan or on *BSD/macOS, this function will always succeed.
 */
bool secureRandomBytes(void *data, size_t len) {
#if defined(USE_OPENSSL)
  // OpenSSL's RAND_bytes() uses int as the length parameter
  for (size_t i = 0; i < len; i += (std::numeric_limits<int>::max)()) {
    int fillSize = (int)(std::min)(len - i, (size_t)(std::numeric_limits<int>::max)());
    if (!RAND_bytes((unsigned char *)data + i, fillSize)) return false;
  }
  return true;
#elif defined(USE_BOTAN)
  thread_local Botan::AutoSeeded_RNG rng;
  rng.randomize((unsigned char *)data, len);
  return true;
#else
  // If no TLS backend is used, we use a CSPRNG of our own. This makes us use
  // up LESS system entropy. CSPRNG proposed by Dan Kaminsky in his DEFCON 22
  // talk.  With some modifications to make it suitable for trantor's
  // codebase. (RIP Dan Kaminsky. That talk was epic.)
  // https://youtu.be/xneBjc8z0DE?t=2250
  namespace chrono = std::chrono;
  static_assert(sizeof(RngState) < 128, "RngState must be less then BLAKE2b's chunk size");

  thread_local int      useCount = 0;
  thread_local RngState state;
  static const int64_t  shiftAmount = []() {
    int64_t shift = 0;
    if (!systemRandomBytes(&shift, sizeof(shift))) {
      // fallback to a random device. Not guaranteed to be secure
      // but it's better than nothing.
      shift = std::random_device{}();
    }
    return shift;
  }();
  // Update secret every 1024 calls to this function
  if (useCount == 0) {
    if (!systemRandomBytes(&state.secret, sizeof(state.secret))) return false;
  }
  useCount = (useCount + 1) % 1024;

  // use the cycle counter register to get a bit more entropy.
  // Quote from the talk: "You can at least get a timestamp. And it turns out
  // you just needs bits that are different. .... If you integrate time. It
  // tuns out imperially It's a pain in the butt to get two things to happen
  // at the exactly the same CPU nanosecond. It's not that it can't. IT'S THAT
  // IT WON'T. AND THAT'S A GOOD THING."
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
  state.time = __rdtsc();
#elif defined(__aarch64__) || defined(_M_ARM64)
  // IMPORTANT! ARMv8 cntvct_el0 is not a cycle counter. It's a free running
  // counter that increments at 1~50MHz. 20~40x slower than the CPU. But
  // hashing takes more then that. So it's still good enough.
#ifdef _MSC_VER
  state.time = _ReadStatusReg(ARM64_CNTVCT_EL0);
#else
  asm volatile("mrs %0, cntvct_el0" : "=r"(state.time));
#endif
#elif defined(__riscv) && __riscv_xlen == 64
  asm volatile("rdtime %0" : "=r"(state.time));
#elif defined(__riscv) && __riscv_xlen == 32
  uint32_t timeLo, timeHi;
  asm volatile("rdtimeh %0" : "=r"(timeHi));
  asm volatile("rdtime %0" : "=r"(timeLo));
  state.time = (uint64_t)timeHi << 32 | timeLo;
#elif defined(__s390__)  // both s390 and s390x
  asm volatile("stck %0" : "=Q"(state.time));
#else
  auto now = chrono::steady_clock::now();
  // the proposed algorithm uses the time in nanoseconds, but we don't have a
  // way to read it (yet) not C++ provided a standard way to do it. Falling
  // back to milliseconds. This along with additional entropy is hopefully
  // good enough.
  state.time = chrono::time_point_cast<chrono::milliseconds>(now).time_since_epoch().count();
  // `now` lives on the stack, so address in each call _may_ be different.
  // This code works on both 32-bit and 64-bit systems. As well as big-endian
  // and little-endian systems.
  void     *stack_ptr   = &now;
  uint32_t *stack_ptr32 = (uint32_t *)&stack_ptr;
  uint32_t  garbage     = *stack_ptr32;
  static_assert(sizeof(void *) >= sizeof(uint32_t), "pointer size too small");
  for (size_t i = 1; i < sizeof(void *) / sizeof(uint32_t); i++) garbage ^= stack_ptr32[i];
  state.time ^= garbage;
#endif
  state.time += shiftAmount;

  // generate the random data as described in the talk. We use BLAKE2b since
  // it's fast and has a good security margin.
  for (size_t i = 0; i < len / sizeof(Hash256); i++) {
    auto hash = blake2b(&state, sizeof(state));
    memcpy((char *)data + i * sizeof(hash), &hash, sizeof(hash));
    state.counter++;
    state.prev = hash;
  }
  if (len % sizeof(Hash256) != 0) {
    auto hash = blake2b(&state, sizeof(state));
    memcpy((char *)data + len - len % sizeof(hash), &hash, len % sizeof(hash));
    state.counter++;
    state.prev = hash;
  }
  return true;
#endif
}
}  // namespace utils
}  // namespace trantor
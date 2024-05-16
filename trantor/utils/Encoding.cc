/**
 *
 *  @file Utilities.cc
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

#include "Encoding.h"

#ifdef _WIN32
#include <algorithm>
#include <windows.h>
#else  // _WIN32
#include <string.h>
#include <unistd.h>
#if __cplusplus < 201103L || __cplusplus >= 201703L
#include <locale.h>
#include <stdlib.h>
#else  // __cplusplus
#include <codecvt>
#include <locale>
#endif  // __cplusplus
#endif  // _WIN32

namespace trantor {
namespace utils {

// (codecvt is deprecated after c++17), so define it ourselves.
#if __cplusplus < 201103L || __cplusplus >= 201703L
/**
 * Convert a UTF-8 encoded string to a UTF-16 encoded string.
 *
 * @param utf8Str the UTF-8 string to convert
 *
 * @return the converted UTF-16 string
 *
 * @throws None
 */
static std::wstring utf8ToUtf16(const std::string &utf8Str) {
  std::wstring utf16Str;
  utf16Str.reserve(utf8Str.length());  // Reserve space to avoid reallocations

  for (size_t i = 0; i < utf8Str.length();) {
    wchar_t unicode_char;

    // Check the first byte
    if ((utf8Str[i] & 0b10000000) == 0) {
      // Single-byte character (ASCII)
      unicode_char = utf8Str[i++];
    } else if ((utf8Str[i] & 0b11100000) == 0b11000000) {
      if (i + 1 >= utf8Str.length()) {
        // Invalid UTF-8 sequence
        // Handle the error as needed
        return L"";
      }
      // Two-byte character
      unicode_char  = ((utf8Str[i] & 0b00011111) << 6) | (utf8Str[i + 1] & 0b00111111);
      i            += 2;
    } else if ((utf8Str[i] & 0b11110000) == 0b11100000) {
      if (i + 2 >= utf8Str.length()) {
        // Invalid UTF-8 sequence
        // Handle the error as needed
        return L"";
      }
      // Three-byte character
      unicode_char =
        ((utf8Str[i] & 0b00001111) << 12) | ((utf8Str[i + 1] & 0b00111111) << 6) | (utf8Str[i + 2] & 0b00111111);
      i += 3;
    } else {
      // Invalid UTF-8 sequence
      // Handle the error as needed
      return L"";
    }

    utf16Str.push_back(unicode_char);
  }

  return utf16Str;
}

/**
 * Converts a UTF-16 encoded string to a UTF-8 encoded string.
 *
 * @param utf16Str The UTF-16 encoded string to be converted.
 *
 * @return The UTF-8 encoded string.
 *
 * @throws None
 */
static std::string utf16ToUtf8(const std::wstring &utf16Str) {
  std::string utf8Str;
  utf8Str.reserve(utf16Str.length() * 3);

  for (size_t i = 0; i < utf16Str.length(); ++i) {
    wchar_t unicode_char = utf16Str[i];

    if (unicode_char <= 0x7F) {
      // Single-byte character (ASCII)
      utf8Str.push_back(static_cast<char>(unicode_char));
    } else if (unicode_char <= 0x7FF) {
      // Two-byte character
      utf8Str.push_back(static_cast<char>(0xC0 | ((unicode_char >> 6) & 0x1F)));
      utf8Str.push_back(static_cast<char>(0x80 | (unicode_char & 0x3F)));
    } else {
      // Three-byte character
      utf8Str.push_back(static_cast<char>(0xE0 | ((unicode_char >> 12) & 0x0F)));
      utf8Str.push_back(static_cast<char>(0x80 | ((unicode_char >> 6) & 0x3F)));
      utf8Str.push_back(static_cast<char>(0x80 | (unicode_char & 0x3F)));
    }
  }

  return utf8Str;
}
#endif  // __cplusplus

/**
 * @brief Convert a UTF-8 string to a wide string.
 * @details UCS2 on Windows, UTF-32 on Linux & Mac
 *
 * @param str String to convert
 *
 * @return converted string.
 */
std::wstring fromUtf8(const std::string &str) {
  if (str.empty()) return {};
  std::wstring wstrTo;
#ifdef _WIN32
  int nSizeNeeded = ::MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
  wstrTo.resize(nSizeNeeded, 0);
  ::MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], nSizeNeeded);
#elif __cplusplus < 201103L || __cplusplus >= 201703L
  wstrTo = utf8ToUtf16(str);
#else  // c++11 to c++14 (codecvt is deprecated after c++17)
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> utf8conv;
  try {
    wstrTo = utf8conv.from_bytes(str);
  } catch (...)  // Should never fail if str valid UTF-8
  {
  }
#endif
  return wstrTo;
}

/**
 * @brief Convert a wide string to a UTF-8.
 * @details UCS2 on Windows, UTF-32 on Linux & Mac
 *
 * @param wstr String to convert
 *
 * @return converted string.
 */
std::string toUtf8(const std::wstring &wstr) {
  if (wstr.empty()) return {};

  std::string strTo;
#ifdef _WIN32
  int nSizeNeeded = ::WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
  strTo.resize(nSizeNeeded, 0);
  ::WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], nSizeNeeded, NULL, NULL);
#elif __cplusplus < 201103L || __cplusplus >= 201703L
  strTo = utf16ToUtf8(wstr);
#else  // c++11 to c++14 (codecvt is deprecated after c++17)
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> utf8conv;
  strTo = utf8conv.to_bytes(wstr);
#endif
  return strTo;
}

/**
 * @details Convert a wide string path with arbitrary directory separators
 * to a UTF-8 portable path for use with trantor.
 *
 * This is a helper, mainly for Windows and multi-platform projects.
 *
 * @note On Windows, backslash directory separators are converted to slash to
 * keep portable paths.
 *
 * @remarks On other OSes, backslashes are not converted to slash, since they
 * are valid characters for directory/file names.
 *
 * @param strPath Wide string path.
 *
 * @return std::string UTF-8 path, with slash directory separator.
 */
std::string fromWidePath(const std::wstring &wstrPath) {
#ifdef _WIN32
  auto srcPath{wstrPath};
  // Not needed: to portable path (just replaces '\' with '/')
  std::replace(srcPath.begin(), srcPath.end(), L'\\', L'/');
#else   // _WIN32
  auto &srcPath{wstrPath};
#endif  // _WIN32
  return toUtf8(srcPath);
}

/**
 * @details Convert a UTF-8 path with arbitrary directory separator to a wide
 * string path.
 *
 * This is a helper, mainly for Windows and multi-platform projects.
 *
 * @note On Windows, slash directory separators are converted to backslash.
 * Although it accepts both slash and backslash as directory separator in its
 * API, it is better to stick to its standard.

 * @remarks On other OSes, slashes are not converted to backslashes, since they
 * are not interpreted as directory separators and are valid characters for
 * directory/file names.
 *
 * @param strUtf8Path Ascii path considered as being UTF-8.
 *
 * @return std::wstring path with, on windows, standard backslash directory
 * separator to stick to its standard.
 */
std::wstring toWidePath(const std::string &strUtf8Path) {
  auto wstrPath{fromUtf8(strUtf8Path)};
#ifdef _WIN32
  // Not needed: normalize path (just replaces '/' with '\')
  std::replace(wstrPath.begin(), wstrPath.end(), L'/', L'\\');
#endif  // _WIN32
  return wstrPath;
}

}  // namespace utils
}  // namespace trantor

/**
 *
 *  @file SourceFile.h
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
#ifndef TRANTOR_SOURCEFILE_H
#define TRANTOR_SOURCEFILE_H

#include <cstring>
namespace trantor {

/**
 * @brief Calculate of basename of source files in compile time.
 *
 */
class SourceFile {
public:
  /**
   * @brief Construct a new Source File object
   *
   * @tparam N
   */
  template <int N>
  inline SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
    // std::cout<<data_<<std::endl;
#ifndef _MSC_VER
    const char *slash = strrchr(data_, '/');  // builtin function
#else
    const char *slash = strrchr(data_, '\\');
#endif
    if (slash) {
      data_  = slash + 1;
      size_ -= static_cast<int>(data_ - arr);
    }
  }

  /**
   * @brief Construct a SourceFile with filename
   * @param filename
   */
  explicit SourceFile(const char *filename = nullptr) : data_(filename) {
    if (!filename) {
      size_ = 0;
      return;
    }
#ifndef _MSC_VER
    const char *slash = strrchr(filename, '/');
#else
    const char *slash = strrchr(filename, '\\');
#endif
    if (slash) {
      data_ = slash + 1;
    }
    size_ = static_cast<int>(strlen(data_));
  }

  const char *data_;
  int         size_;
};

}  // namespace trantor

#endif  // TRANTOR_SOURCEFILE_H

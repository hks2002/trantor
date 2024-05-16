/**
 *
 *  @file LoggerFile.h
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

#ifndef TRANTOR_LOGGER_FILE_H
#define TRANTOR_LOGGER_FILE_H

#include "trantor/NonCopyable.h"
#include "trantor/utils/Date.h"
#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <string_view>

namespace trantor {

using StringPtr = std::shared_ptr<std::string>;

/**
 * @brief This class implements utility functions for writing logs to files.
 *
 */
class LoggerFile : NonCopyable {
public:
  LoggerFile(std::string_view filePath,
             std::string_view fileBaseName,
             std::string_view fileExtName,
             bool             switchOnLimitOnly = false,
             size_t           maxFiles          = 0);

  ~LoggerFile();
  void     open();
  void     writeLog(const StringPtr buf);
  void     switchLog(bool openNewOne);
  uint64_t getLength();
  void     flush();
  explicit operator bool() const {
    return fp_ != nullptr;
  }

protected:
  FILE         *fp_{nullptr};
  Date          creationDate_;
  std::string   fileFullName_;
  std::string   filePath_;
  std::string   fileBaseName_;
  std::string   fileExtName_;
  static size_t fileSeq_;
  bool          switchOnLimitOnly_{false};  // by default false, will generate new
                                            // file name on each destroy
private:
  size_t maxFiles_{0};
  // store generated filenames
  std::deque<std::string> filenameQueue_;

  void                    initFilenameQueue();
  void                    deleteOldFiles();
};

}  // namespace trantor

#endif  // TRANTOR_LOGGER_FILE_H

/**
 *
 *  @file AsyncFileLogger.h
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

#ifndef TRANTOR_ASYNC_FILE_LOGGER_H
#define TRANTOR_ASYNC_FILE_LOGGER_H

#include "LoggerFile.h"
#include "trantor/NonCopyable.h"
#include "trantor/exports.h"
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>

namespace trantor {
using StringPtr      = std::shared_ptr<std::string>;
using StringPtrQueue = std::queue<StringPtr>;

/**
 * @brief This class implements utility functions for writing logs to files
 * asynchronously.
 *
 */
class TRANTOR_EXPORT AsyncFileLogger : NonCopyable {
public:
  AsyncFileLogger();
  ~AsyncFileLogger();

  void setFileName(std::string_view baseName, std::string_view extName = ".log", std::string_view path = "./");
  void setFileSizeLimit(uint64_t limit);
  void setMaxFiles(size_t maxFiles);
  void setSwitchOnLimitOnly(bool flag = true);
  void output(const char *msg, const size_t len);
  void flush();
  void startLogging();

protected:
  std::string filePath_{"./"};
  std::string fileBaseName_{"trantor"};
  std::string fileExtName_{".log"};
  uint64_t    sizeLimit_{20 * 1024 * 1024};
  size_t      maxFiles_{0};
  bool        switchOnLimitOnly_{false};  // by default false, will generate new
                                          // file name on each destroy.
  size_t                       lostCounter_{0};

  StringPtr                    logBufferPtr_;
  StringPtr                    nextBufferPtr_;
  std::unique_ptr<LoggerFile>  loggerFilePtr_;

  std::mutex                   mutex_;
  std::condition_variable      cond_;
  StringPtrQueue               writeBuffers_;
  StringPtrQueue               tmpBuffers_;
  std::unique_ptr<std::thread> threadPtr_;
  bool                         stopFlag_{false};

  void                         swapBuffer();
  void                         logThreadFunc();
  void                         writeLogToFile(const StringPtr buf);
};

}  // namespace trantor

#endif  // TRANTOR_ASYNC_FILE_LOGGER_H

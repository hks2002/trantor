/**
 *
 *  AsyncFileLogger.cc
 *  An Tao
 *
 *  Public header file in trantor lib.
 *
 *  Copyright 2018, An Tao.  All rights reserved.
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the License file.
 *
 *
 */

#include "AsyncFileLogger.h"

#include "trantor/utils/Date.h"
#include "trantor/utils/Encoding.h"
#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <string.h>
#include <thread>

#if !defined(_WIN32) || defined(__MINGW32__)
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif
#else
#include <windows.h>
#endif

namespace trantor {

extern const char                    *strerror_tl(int savedErrno);

static constexpr std::chrono::seconds kLogFlushTimeout{1};
static constexpr size_t               kMemBufferSize{4 * 1024 * 1024};

AsyncFileLogger::AsyncFileLogger() : logBufferPtr_(new std::string), nextBufferPtr_(new std::string) {
  logBufferPtr_->reserve(kMemBufferSize);
  nextBufferPtr_->reserve(kMemBufferSize);
}

AsyncFileLogger::~AsyncFileLogger() {
  // std::cout << "~AsyncFileLogger" << std::endl;
  stopFlag_ = true;
  if (threadPtr_) {
    cond_.notify_all();
    threadPtr_->join();
  }
  // std::cout << "thread exit" << std::endl;
  {
    std::lock_guard<std::mutex> guard_(mutex_);
    if (logBufferPtr_->length() > 0) {
      writeBuffers_.push(logBufferPtr_);
    }
    while (!writeBuffers_.empty()) {
      StringPtr tmpPtr = (StringPtr &&)writeBuffers_.front();
      writeBuffers_.pop();
      writeLogToFile(tmpPtr);
    }
  }
}

/**
 * @brief Set the log file name.
 *
 * @param baseName The base name of the log file.
 * @param extName The extended name of the log file.
 * @param path The location where the log file is stored.
 */
void AsyncFileLogger::setFileName(std::string_view baseName, std::string_view extName, std::string_view path) {
  fileBaseName_ = baseName;
  extName[0] == '.' ? fileExtName_ = extName : fileExtName_ = std::string(".") + extName.data();
  filePath_ = path;
  if (filePath_.length() == 0) filePath_ = "./";
  if (filePath_[filePath_.length() - 1] != '/') filePath_ = filePath_ + "/";
}

/**
 * @brief Set the size limit of log files. When the log file size reaches
 * the limit, the log file is switched.
 *
 * @param limit
 */
void AsyncFileLogger::setFileSizeLimit(uint64_t limit) {
  sizeLimit_ = limit;
}

/**
 * @brief Set the max number of log files. When the number exceeds the
 * limit, the oldest log file will be deleted.
 *
 * @param maxFiles
 */
void AsyncFileLogger::setMaxFiles(size_t maxFiles) {
  maxFiles_ = maxFiles;
}

/**
 * @brief Set whether to switch the log file when the AsyncFileLogger object
 * is destroyed. If this flag is set to true, the log file is not switched
 * when the AsyncFileLogger object is destroyed.
 *
 * @param flag
 */
void AsyncFileLogger::setSwitchOnLimitOnly(bool flag) {
  switchOnLimitOnly_ = flag;
}

/**
 * @brief Swaps the current log buffer with the next buffer or creates a new buffer if there is no next buffer.
 *
 * @throws None
 */
void AsyncFileLogger::swapBuffer() {
  writeBuffers_.push(logBufferPtr_);
  if (nextBufferPtr_) {
    logBufferPtr_ = nextBufferPtr_;
    nextBufferPtr_.reset();
    logBufferPtr_->clear();
  } else {
    logBufferPtr_ = std::make_shared<std::string>();
    logBufferPtr_->reserve(kMemBufferSize);
  }
}

/**
 * @brief Writes the given log message to the log file.
 *
 * @param buf The log message to be written.
 *
 * @throws None
 */
void AsyncFileLogger::writeLogToFile(const StringPtr buf) {
  if (!loggerFilePtr_) {
    loggerFilePtr_ = std::unique_ptr<LoggerFile>(
      new LoggerFile(filePath_, fileBaseName_, fileExtName_, switchOnLimitOnly_, maxFiles_));
  }
  loggerFilePtr_->writeLog(buf);
  if (loggerFilePtr_->getLength() > sizeLimit_) {
    loggerFilePtr_->switchLog(true);
  }
}

/**
 * @brief Write the message to the log file.
 *
 * @param msg
 * @param len
 */
void AsyncFileLogger::output(const char *msg, const size_t len) {
  std::lock_guard<std::mutex> guard_(mutex_);
  if (len > kMemBufferSize) return;

  if (logBufferPtr_->capacity() - logBufferPtr_->length() < len) {
    swapBuffer();
    cond_.notify_one();
  }
  if (writeBuffers_.size() > 25)  // 100M bytes logs in buffer
  {
    ++lostCounter_;
    return;
  }

  if (lostCounter_ > 0) {
    char logErr[128];
    auto strlen  = snprintf(logErr,
                           sizeof(logErr),
                           "%llu log information is lost\n",
                           static_cast<long long unsigned int>(lostCounter_));
    lostCounter_ = 0;
    logBufferPtr_->append(logErr, strlen);
  }
  logBufferPtr_->append(msg, len);
}

/**
 * @brief Flush data from memory buffer to the log file.
 *
 */
void AsyncFileLogger::flush() {
  std::lock_guard<std::mutex> guard_(mutex_);
  if (logBufferPtr_->length() > 0) {
    // std::cout<<"flush log buffer
    // len:"<<logBufferPtr_->length()<<std::endl;
    swapBuffer();
    cond_.notify_one();
  }
}

/**
 * @brief The logThreadFunc function is responsible for executing the logging thread.
 *
 * @throws ErrorType description of error
 */
void AsyncFileLogger::logThreadFunc() {
#ifdef __linux__
  prctl(PR_SET_NAME, "AsyncFileLogger");
#endif
  while (!stopFlag_) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      while (writeBuffers_.size() == 0 && !stopFlag_) {
        if (cond_.wait_for(lock, kLogFlushTimeout) == std::cv_status::timeout) {
          if (logBufferPtr_->length() > 0) {
            swapBuffer();
          }
          break;
        }
      }
      tmpBuffers_.swap(writeBuffers_);
    }

    while (!tmpBuffers_.empty()) {
      StringPtr tmpPtr = (StringPtr &&)tmpBuffers_.front();
      tmpBuffers_.pop();
      writeLogToFile(tmpPtr);
      tmpPtr->clear();
      {
        std::unique_lock<std::mutex> lock(mutex_);
        nextBufferPtr_ = tmpPtr;
      }
    }
    if (loggerFilePtr_) loggerFilePtr_->flush();
  }
}

/**
 * @brief Start writing log files.
 *
 */
void AsyncFileLogger::startLogging() {
  threadPtr_ = std::unique_ptr<std::thread>(new std::thread(std::bind(&AsyncFileLogger::logThreadFunc, this)));
}

}  // namespace trantor

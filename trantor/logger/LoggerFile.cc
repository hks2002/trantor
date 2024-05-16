/**
 *
 *  @file LoggerFile.cc
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

#include "LoggerFile.h"

#include "Logger.h"
#include "trantor/utils/Encoding.h"
#include <algorithm>
#include <deque>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>

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

extern const char *strerror_tl(int savedErrno);

size_t             LoggerFile::fileSeq_{0};

/**
 * @brief Initialize the filename queue by walking through the directory and finding all files that match the specified
 * pattern. maxFiles_ is used to limit the number of files in the queue.
 */
void LoggerFile::initFilenameQueue() {
  if (maxFiles_ <= 0) {
    return;
  }

  // walk through the directory and file all files
#if !defined(_WIN32) || defined(__MINGW32__)
  DIR           *dp;
  struct dirent *dirp;
  struct stat    st;

  if ((dp = opendir(filePath_.c_str())) == nullptr) {
    fprintf(stderr, "Can't open dir %s: %s\n", filePath_.c_str(), strerror_tl(errno));
    return;
  }

  while ((dirp = readdir(dp)) != nullptr) {
    std::string name = dirp->d_name;
    // <base>.yymmdd-hhmmss.000000<ext>
    // NOTE: magic number 21: the length of middle part of generated name
    if (name.size() != fileBaseName_.size() + 21 + fileExtName_.size() ||
        name.compare(0, fileBaseName_.size(), fileBaseName_) != 0 ||
        name.compare(name.size() - fileExtName_.size(), fileExtName_.size(), fileExtName_) != 0) {
      continue;
    }
    std::string fullname = filePath_ + name;
    if (stat(fullname.c_str(), &st) == -1) {
      fprintf(stderr, "Can't stat file %s: %s\n", fullname.c_str(), strerror_tl(errno));
      continue;
    }
    if (!S_ISREG(st.st_mode)) {
      continue;
    }
    filenameQueue_.push_back(fullname);
    std::push_heap(filenameQueue_.begin(), filenameQueue_.end(), std::greater<>());
    if (filenameQueue_.size() > maxFiles_) {
      std::pop_heap(filenameQueue_.begin(), filenameQueue_.end(), std::greater<>());
      auto fileToRemove = std::move(filenameQueue_.back());
      filenameQueue_.pop_back();
      remove(fileToRemove.c_str());
    }
  }
  closedir(dp);
#else
  // TODO: windows implementation
#endif
  std::sort(filenameQueue_.begin(), filenameQueue_.end(), std::less<>());
}

/**
 * @brief Deletes old files from the filenameQueue_ until the size is less than or equal to maxFiles_.
 */
void LoggerFile::deleteOldFiles() {
  while (filenameQueue_.size() > maxFiles_) {
    std::string filename = std::move(filenameQueue_.front());
    filenameQueue_.pop_front();

#if !defined(_WIN32) || defined(__MINGW32__)
    int r = remove(filename.c_str());
#else
    // Convert UTF-8 file to UCS-2
    auto wName{utils::toNativePath(filename)};
    int  r = _wremove(wName.c_str());
#endif
    if (r != 0) {
      fprintf(stderr, "Failed to remove file %s: %s\n", filename.c_str(), strerror_tl(errno));
    }
  }
}

LoggerFile::LoggerFile(std::string_view filePath,
                       std::string_view fileBaseName,
                       std::string_view fileExtName,
                       bool             switchOnLimitOnly,
                       size_t           maxFiles)
  : creationDate_(Date::date()),
    filePath_(filePath),
    fileBaseName_(fileBaseName),
    fileExtName_(fileExtName),
    switchOnLimitOnly_(switchOnLimitOnly),
    maxFiles_(maxFiles) {
  open();

  if (maxFiles_ > 0) {
    initFilenameQueue();
  }
}

/**
 * @brief Open file for append logs, Always write to file with base name.
 */
void LoggerFile::open() {
  fileFullName_ = filePath_ + fileBaseName_ + fileExtName_;
#ifndef _MSC_VER
  fp_ = fopen(fileFullName_.c_str(), "a");
#else
  // Convert UTF-8 file to UCS-2
  auto wFullName{utils::toNativePath(fileFullName_)};
  fp_ = _wfsopen(wFullName.c_str(), L"a+", _SH_DENYWR);
#endif
  if (fp_ == nullptr) {
    std::cout << strerror_tl(errno) << std::endl;
  }
}

/**
 * @brief Writes the given log message to the log file.
 *
 * @param buf a pointer to the log message to be written
 */
void LoggerFile::writeLog(const StringPtr buf) {
  if (fp_) {
    // std::cout<<"write "<<buf->length()<<" bytes to file"<<std::endl;
    fwrite(buf->c_str(), 1, buf->length(), fp_);
  }
}

/**
 * @brief Force store the current file (with the base name) with the newly generated name by adding time point.
 *
 * @param openNewOne - true for keeping log file opened and continuing logging
 */
void LoggerFile::switchLog(bool openNewOne) {
  if (fp_) {
    fclose(fp_);
    fp_ = nullptr;

    char seq[12];
    snprintf(seq, sizeof(seq), ".%06llu", static_cast<long long unsigned int>(fileSeq_ % 1000000));
    ++fileSeq_;
    // NOTE: Remember to update initFilenameQueue() if name format changes
    std::string newName = filePath_ + fileBaseName_ + "." + creationDate_.toCustomFormattedString("%y%m%d-%H%M%S") +
                          std::string(seq) + fileExtName_;

#if !defined(_WIN32) || defined(__MINGW32__)
    rename(fileFullName_.c_str(), newName.c_str());
#else
    // Convert UTF-8 file to UCS-2
    auto wFullName{utils::toNativePath(fileFullName_)};
    auto wNewName{utils::toNativePath(newName)};
    _wrename(wFullName.c_str(), wNewName.c_str());
#endif

    if (maxFiles_ > 0) {
      filenameQueue_.push_back(newName);
      if (filenameQueue_.size() > maxFiles_) {
        deleteOldFiles();
      }
    }
    if (openNewOne)
      open();  // continue logging with base name until next renaming will
               // be required
  }
}

/**
 * @brief Get length of current log file.
 *
 */
uint64_t LoggerFile::getLength() {
  if (fp_) return ftell(fp_);
  return 0;
}

/**
 * @brief Use fflush to force write to disk.
 *
 */
void LoggerFile::flush() {
  if (fp_) {
    fflush(fp_);
  }
}

LoggerFile::~LoggerFile() {
  if (!switchOnLimitOnly_)  // rename on each destroy
    switchLog(false);
  if (fp_) fclose(fp_);
}

}  // namespace trantor

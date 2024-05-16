/**
 *
 *  @file Logger.h
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

#ifndef TRANTOR_LOGGER_H
#define TRANTOR_LOGGER_H

#include "LogStream.h"
#include "RawLogger.h"
#include "SourceFile.h"
#include "trantor/NonCopyable.h"
#include "trantor/exports.h"
#include "trantor/utils/Date.h"
#include <functional>
#include <memory>

#ifdef TRANTOR_SPDLOG_SUPPORT
#include <map>
#include <mutex>
#include <spdlog/spdlog.h>
#endif

namespace spdlog {
class logger;
}

namespace trantor {
const char *strerror_tl(int savedErrno);

/**
 * @brief This class implements log functions.
 *
 */
class TRANTOR_EXPORT Logger : public NonCopyable {
public:
  enum LogLevel { kTrace = 0, kDebug, kInfo, kWarn, kError, kFatal, kNumberOfLogLevels };

public:
  static LogLevel logLevel();
  static void     setLogLevel(LogLevel level);
  static bool     displayLocalTime();
  static void     setDisplayLocalTime(bool showLocalTime);

  static void     defaultFlushFunction();
  static void     defaultOutputFunction(const char *msg, const size_t len);
  static void     setOutputFunction(std::function<void(const char *msg, const size_t len)> outputFunc,
                                    std::function<void()>                                  flushFunc,
                                    int                                                    index = -1);

  static bool     hasSpdLogSupport();
#ifdef TRANTOR_SPDLOG_SUPPORT
  static void enableSpdLog(std::shared_ptr<spdlog::logger> logger = {});
  static void enableSpdLog(int index, std::shared_ptr<spdlog::logger> logger = {});
  static void disableSpdLog();
  static void disableSpdLog(int index = -1);
#endif

  // LOG_COMPACT only <time><ThreadID><Level>
  Logger();
  Logger(LogLevel level);
  Logger(bool isSysErr);

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, bool isSysErr);
  Logger(SourceFile file, int line, LogLevel level, const char *func);

  Logger    &setIndex(int index);
  LogStream &stream();

  ~Logger();

protected:
  static LogLevel                                               &defaultLogLevel_();
  static bool                                                   &displayLocalTime_();
  static std::function<void()>                                  &flushFunc_();
  static std::function<void()>                                  &flushFunc_(int index);
  static std::function<void(const char *msg, const size_t len)> &outputFunc_();
  static std::function<void(const char *msg, const size_t len)> &outputFunc_(int index);
#ifdef TRANTOR_SPDLOG_SUPPORT
  static std::shared_ptr<spdlog::logger> getDefaultSpdLogger(int index);
  static std::shared_ptr<spdlog::logger> getSpdLogger(int index = -1);
  static std::shared_ptr<spdlog::logger> getRawSpdLogger(int index);
#endif

protected:
  friend class RawLogger;

  LogLevel   level_;
  LogStream  logStream_;
  Date       date_{Date::now()};
  SourceFile sourceFile_;
  int        fileLine_;
  int        index_{-1};

  void       formatTime();

#ifdef TRANTOR_SPDLOG_SUPPORT
  const char *func_{nullptr};  // to hold function name
  std::size_t spdLogMessageOffset_{0};
#endif
};

/**
 * @brief Macro for conditional logging.
 *
 * @note Using if() instead of for(;;) for better performance
 * #define TRANTOR_IF_(cond) for (int _r = 0; _r == 0 && (cond); _r = 1)
 */
#define TRANTOR_IF_(cond)        if (cond)
#define COMPARE_LOG_LEVEL(level) trantor::Logger::logLevel() <= trantor::Logger::level

/* clang-format off */
 
// log raw begin
#define LOG_TRACE_RAW             trantor::Logger(__FILE__, __LINE__, trantor::Logger::kTrace, __func__).stream()
#define LOG_TRACE_RAW_TO(index)   trantor::Logger(__FILE__, __LINE__, trantor::Logger::kTrace, __func__).setIndex(index).stream()
#define LOG_DEBUG_RAW             trantor::Logger(__FILE__, __LINE__, trantor::Logger::kDebug, __func__).stream()
#define LOG_DEBUG_RAW_TO(index)   trantor::Logger(__FILE__, __LINE__, trantor::Logger::kDebug, __func__).setIndex(index).stream()
#define LOG_INFO_RAW              trantor::Logger(__FILE__, __LINE__, trantor::Logger::kInfo).stream()
#define LOG_INFO_RAW_TO(index)    trantor::Logger(__FILE__, __LINE__, trantor::Logger::kInfo).setIndex(index).stream()
#define LOG_WARN_RAW              trantor::Logger(__FILE__, __LINE__, trantor::Logger::kWarn).stream()
#define LOG_WARN_RAW_TO(index)    trantor::Logger(__FILE__, __LINE__, trantor::Logger::kWarn).setIndex(index).stream()
#define LOG_ERROR_RAW             trantor::Logger(__FILE__, __LINE__, trantor::Logger::kError).stream()
#define LOG_ERROR_RAW_TO(index)   trantor::Logger(__FILE__, __LINE__, trantor::Logger::kError).setIndex(index).stream()
#define LOG_FATAL_RAW             trantor::Logger(__FILE__, __LINE__, trantor::Logger::kFatal).stream()
#define LOG_FATAL_RAW_TO(index)   trantor::Logger(__FILE__, __LINE__, trantor::Logger::kFatal).setIndex(index).stream()
#define LOG_SYSERR_RAW            trantor::Logger(__FILE__, __LINE__, true).stream()
#define LOG_SYSERR_RAW_TO(index)  trantor::Logger(__FILE__, __LINE__, true).setIndex(index).stream()

#define LOG_COMPACT_DEBUG_RAW            trantor::Logger(trantor::Logger::kDebug).stream()
#define LOG_COMPACT_DEBUG_RAW_TO(index)  trantor::Logger(trantor::Logger::kDebug).setIndex(index).stream()
#define LOG_COMPACT_INFO_RAW             trantor::Logger(trantor::Logger::kInfo).stream()
#define LOG_COMPACT_INFO_RAW_TO(index)   trantor::Logger(trantor::Logger::kInfo).setIndex(index).stream()
#define LOG_COMPACT_WARN_RAW             trantor::Logger(trantor::Logger::kWarn).stream()
#define LOG_COMPACT_WARN_RAW_TO(index)   trantor::Logger(trantor::Logger::kWarn).setIndex(index).stream()
#define LOG_COMPACT_ERROR_RAW            trantor::Logger(trantor::Logger::kError).stream()
#define LOG_COMPACT_ERROR_RAW_TO(index)  trantor::Logger(trantor::Logger::kError).setIndex(index).stream()
#define LOG_COMPACT_FATAL_RAW            trantor::Logger(trantor::Logger::kFatal).stream()
#define LOG_COMPACT_FATAL_RAW_TO(index)  trantor::Logger(trantor::Logger::kFatal).setIndex(index).stream()
#define LOG_COMPACT_SYSERR_RAW           trantor::Logger(true).stream()
#define LOG_COMPACT_SYSERR_RAW_TO(index) trantor::Logger(true).setIndex(index).stream()

#define LOG_RAW           trantor::RawLogger().stream()
#define LOG_RAW_TO(index) trantor::RawLogger().setIndex(index).stream()
// log raw end

// log begin
#ifdef NDEBUG
#define LOG_TRACE TRANTOR_IF_(0) LOG_TRACE_RAW
#else
#define LOG_TRACE            TRANTOR_IF_(COMPARE_LOG_LEVEL(kTrace)) LOG_TRACE_RAW
#define LOG_TRACE_TO(index)  TRANTOR_IF_(COMPARE_LOG_LEVEL(kTrace)) LOG_TRACE_RAW_TO(index)
#endif

#define LOG_DEBUG            TRANTOR_IF_(COMPARE_LOG_LEVEL(kDebug)) LOG_DEBUG_RAW
#define LOG_DEBUG_TO(index)  TRANTOR_IF_(COMPARE_LOG_LEVEL(kDebug)) LOG_DEBUG_RAW_TO(index)
#define LOG_INFO             TRANTOR_IF_(COMPARE_LOG_LEVEL(kInfo))  LOG_INFO_RAW
#define LOG_INFO_TO(index)   TRANTOR_IF_(COMPARE_LOG_LEVEL(kInfo))  LOG_INFO_RAW_TO(index)

#define LOG_WARN             LOG_WARN_RAW
#define LOG_WARN_TO(index)   LOG_WARN_RAW_TO(index)
#define LOG_ERROR            LOG_ERROR_RAW
#define LOG_ERROR_TO(index)  LOG_ERROR_RAW_TO(index)
#define LOG_FATAL            LOG_FATAL_RAW
#define LOG_FATAL_TO(index)  LOG_FATAL_RAW_TO(index)
#define LOG_SYSERR           LOG_SYSERR_RAW
#define LOG_SYSERR_TO(index) LOG_SYSERR_RAW_TO(index)
// log end

// log compact begin
#define LOG_COMPACT_DEBUG            TRANTOR_IF_(COMPARE_LOG_LEVEL(kDebug)) LOG_COMPACT_DEBUG_RAW
#define LOG_COMPACT_DEBUG_TO(index)  TRANTOR_IF_(COMPARE_LOG_LEVEL(kDebug)) LOG_COMPACT_DEBUG_RAW_TO(index)
#define LOG_COMPACT_INFO             TRANTOR_IF_(COMPARE_LOG_LEVEL(kInfo))  LOG_COMPACT_INFO_RAW
#define LOG_COMPACT_INFO_TO(index)   TRANTOR_IF_(COMPARE_LOG_LEVEL(kInfo))  LOG_COMPACT_INFO_RAW_TO(index)

#define LOG_COMPACT_WARN             LOG_COMPACT_WARN_RAW
#define LOG_COMPACT_WARN_TO(index)   LOG_COMPACT_WARN_RAW_TO(index)
#define LOG_COMPACT_ERROR            LOG_COMPACT_ERROR_RAW
#define LOG_COMPACT_ERROR_TO(index)  LOG_COMPACT_ERROR_RAW_TO(index)
#define LOG_COMPACT_FATAL            LOG_COMPACT_FATAL_RAW
#define LOG_COMPACT_FATAL_TO(index)  LOG_COMPACT_FATAL_RAW_TO(index)
#define LOG_COMPACT_SYSERR           LOG_COMPACT_SYSERR_RAW
#define LOG_COMPACT_SYSERR_TO(index) LOG_COMPACT_SYSERR_RAW_TO(index)
// log compact end

// dlog begin
#ifdef NDEBUG
#define DLOG_TRACE          TRANTOR_IF_(0) LOG_TRACE_RAW
#define DLOG_DEBUG          TRANTOR_IF_(0) LOG_DEBUG_RAW
#define DLOG_INFO           TRANTOR_IF_(0) LOG_INFO_RAW
#define DLOG_WARN           TRANTOR_IF_(0) LOG_WARN_RAW
#define DLOG_ERROR          TRANTOR_IF_(0) LOG_ERROR_RAW
#define DLOG_FATAL          TRANTOR_IF_(0) LOG_FATAL_RAW

#define DLOG_TRACE_IF(cond) TRANTOR_IF_(0) DLOG_TRACE_RAW
#define DLOG_DEBUG_IF(cond) TRANTOR_IF_(0) DLOG_DEBUG_RAW
#define DLOG_INFO_IF(cond)  TRANTOR_IF_(0) DLOG_INFO_RAW
#define DLOG_WARN_IF(cond)  TRANTOR_IF_(0) DLOG_WARN_RAW
#define DLOG_ERROR_IF(cond) TRANTOR_IF_(0) DLOG_ERROR_RAW
#define DLOG_FATAL_IF(cond) TRANTOR_IF_(0) LOG_FATAL_RAW
#else
#define DLOG_TRACE          TRANTOR_IF_(COMPARE_LOG_LEVEL(kTrace)) LOG_TRACE_RAW
#define DLOG_DEBUG          TRANTOR_IF_(COMPARE_LOG_LEVEL(kDebug)) LOG_DEBUG_RAW
#define DLOG_INFO           TRANTOR_IF_(COMPARE_LOG_LEVEL(kInfo)) LOG_INFO_RAW
#define DLOG_WARN           LOG_WARN_RAW
#define DLOG_ERROR          LOG_ERROR_RAW
#define DLOG_FATAL          LOG_FATAL_RAW

#define DLOG_TRACE_IF(cond) TRANTOR_IF_((COMPARE_LOG_LEVEL(kTrace)) && (cond)) LOG_TRACE_RAW
#define DLOG_DEBUG_IF(cond) TRANTOR_IF_((COMPARE_LOG_LEVEL(kDebug)) && (cond)) LOG_DEBUG_RAW
#define DLOG_INFO_IF(cond)  TRANTOR_IF_((COMPARE_LOG_LEVEL(kInfo)) && (cond))  LOG_INFO_RAW
#define DLOG_WARN_IF(cond)  TRANTOR_IF_(cond) LOG_WARN_RAW
#define DLOG_ERROR_IF(cond) TRANTOR_IF_(cond) LOG_ERROR_RAW
#define DLOG_FATAL_IF(cond) TRANTOR_IF_(cond) LOG_FATAL_RAW
#endif
// dlog end

/* clang-format on */

}  // namespace trantor

#endif  // TRANTOR_LOGGER_H

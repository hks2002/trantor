/**
 *
 *  Logger.cc
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

#include "Logger.h"

#include "LogStream.h"
#include "RawLogger.h"
#include "SourceFile.h"
#include <algorithm>
#include <cassert>
#include <stdio.h>
#include <thread>

#ifdef __unix__
#include <sstream>
#include <sys/syscall.h>
#include <unistd.h>
#elif defined __HAIKU__
#include <sstream>
#include <unistd.h>
#elif defined _WIN32
#include <sstream>
#endif

#if defined __FreeBSD__
#include <pthread_np.h>
#endif

namespace trantor {
/**
 * @brief helper class for known string length at compile time
 */
class T {
public:
  T(const char *str, unsigned len) : str_(str), len_(len) {
    assert(strlen(str) == len_);
  }

  const char    *str_;
  const unsigned len_;
};

/**
 * @brief Extend the LogStream << operator for class T
 *
 * @param s class LogStream
 * @param v class T
 * @return LogStream&
 */
inline LogStream &operator<<(LogStream &s, T v) {
  s.append(v.str_, v.len_);
  return s;
}

/**
 * @brief Get the error message by errno.
 *
 * @param savedErrno
 * @return
 */
const char *strerror_tl(int savedErrno) {
#ifndef _MSC_VER
  return strerror(savedErrno);
#else
  static thread_local char errMsg[64];
  (void)strerror_s<sizeof errMsg>(errMsg, savedErrno);
  return errMsg;
#endif
}

/**
 * @brief Return the log level string fixed with 7 characters, pad with space, e.g. " TRACE ".
 *
 */
static const char *logLevelStr[Logger::LogLevel::kNumberOfLogLevels] = {
  " TRACE ",
  " DEBUG ",
  " INFO  ",
  " WARN  ",
  " ERROR ",
  " FATAL ",
};

/**
 * @brief Return the default log level.
 *
 * @note If RELEASE is defined, it is set to Info. Otherwise it is set to Debug.
 *
 * @return Logger::LogLevel
 */
Logger::LogLevel &Logger::defaultLogLevel_() {
#ifdef RELEASE
  static LogLevel logLevel = LogLevel::kInfo;
#else
  static LogLevel logLevel = LogLevel::kDebug;
#endif
  return logLevel;
}

/**
 * @brief Get the current log level.
 *
 * @note If RELEASE is defined, it is set to Info. Otherwise it is set to Debug.
 * The level could be changed by Logger::setLogLevel.
 *
 * @return LogLevel
 */
Logger::LogLevel Logger::logLevel() {
  return defaultLogLevel_();
}

/**
 * @brief Set the log level. Logs below the level are not printed.
 *
 * @param level
 */
void Logger::setLogLevel(LogLevel level) {
  defaultLogLevel_() = level;
}

/**
 * @brief Display local time is false by default.
 *
 * @return false
 */
bool &Logger::displayLocalTime_() {
  static bool showLocalTime = false;
  return showLocalTime;
}

/**
 * @brief Check whether it shows local time or UTC time currently.
 *
 * @note It is false by default. But it could be changed by Logger::setDisplayLocalTime.
 */
bool Logger::displayLocalTime() {
  return displayLocalTime_();
}

/**
 * @brief Set whether it shows local time or UTC time. the default is UTC.
 */
void Logger::setDisplayLocalTime(bool showLocalTime) {
  displayLocalTime_() = showLocalTime;
}

/**
 * @brief set the default flush function.
 *
 * @note By default, it flushes to stdout.
 *
 */
void Logger::defaultFlushFunction() {
  fflush(stdout);
}

/**
 * @brief Set the default output function.
 *
 * @note By default, it outputs to stdout.
 *
 * @param msg
 * @param len
 */
void Logger::defaultOutputFunction(const char *msg, const size_t len) {
  fwrite(msg, 1, len, stdout);
}

/**
 * @brief Get the default flush function.
 *
 */
std::function<void()> &Logger::flushFunc_() {
  static std::function<void()> flushFunc = Logger::defaultFlushFunction;
  return flushFunc;
}

/**
 * @brief Get the flush function by index.
 *
 */
std::function<void()> &Logger::flushFunc_(int index) {
  static std::vector<std::function<void()>> flushFuncs;
  if (index < flushFuncs.size()) {
    return flushFuncs[index];
  }
  while (index >= flushFuncs.size()) {
    flushFuncs.emplace_back(flushFunc_());
  }
  return flushFuncs[index];
}

/**
 * @brief Get the default output function.
 *
 */
std::function<void(const char *msg, const size_t len)> &Logger::outputFunc_() {
  static std::function<void(const char *msg, const size_t len)> outputFunc = Logger::defaultOutputFunction;
  return outputFunc;
}

/**
 * @brief Get the output function by index.
 *
 */
std::function<void(const char *msg, const size_t len)> &Logger::outputFunc_(int index) {
  static std::vector<std::function<void(const char *msg, const size_t len)>> outputFuncs;
  if (index < outputFuncs.size()) {
    return outputFuncs[index];
  }
  while (index >= outputFuncs.size()) {
    outputFuncs.emplace_back(outputFunc_());
  }
  return outputFuncs[index];
}

/**
 * @brief Set specific flush and output function.
 *
 * @param outputFunc The function to output a log message.
 * @param flushFunc The function to flush.
 * @param index The channel index.
 * @note Logs are output to the standard output by default.
 */
void Logger::setOutputFunction(std::function<void(const char *msg, const size_t len)> outputFunc,
                               std::function<void()>                                  flushFunc,
                               int                                                    index) {
  if (index < 0) {
    outputFunc_() = outputFunc;
    flushFunc_()  = flushFunc;
  } else {
    outputFunc_(index) = outputFunc;
    flushFunc_(index)  = flushFunc;
  }
}

// Some static variables
static thread_local uint64_t lastSecond_{0};
static thread_local char     lastTimeString_[32] = {0};
#ifdef __linux__
static thread_local pid_t threadId_{0};
#else
static thread_local uint64_t threadId_{0};
#endif
//   static thread_local LogStream logStream_;
/**
 * @brief Output formatted timestamp.
 *
 */
void Logger::formatTime() {
  uint64_t now = static_cast<uint64_t>(date_.secondsSinceEpoch());
  uint64_t microSec =
    static_cast<uint64_t>(date_.microSecondsSinceEpoch() - date_.roundSecond().microSecondsSinceEpoch());

  if (now != lastSecond_) {
    lastSecond_ = now;
    if (displayLocalTime_()) {
#ifndef _MSC_VER
      strncpy(lastTimeString_, date_.toFormattedStringLocal(false).c_str(), sizeof(lastTimeString_) - 1);
#else
      strncpy_s<sizeof lastTimeString_>(lastTimeString_,
                                        date_.toFormattedStringLocal(false).c_str(),
                                        sizeof(lastTimeString_) - 1);
#endif
    } else {
#ifndef _MSC_VER
      strncpy(lastTimeString_, date_.toFormattedString(false).c_str(), sizeof(lastTimeString_) - 1);
#else
      strncpy_s<sizeof lastTimeString_>(lastTimeString_,
                                        date_.toFormattedString(false).c_str(),
                                        sizeof(lastTimeString_) - 1);
#endif
    }
  }

  logStream_ << T(lastTimeString_, 17);
  char tmp[32];
  if (displayLocalTime_()) {
    snprintf(tmp, sizeof(tmp), ".%06llu ", static_cast<long long unsigned int>(microSec));
    logStream_ << T(tmp, 8);
  } else {
    snprintf(tmp, sizeof(tmp), ".%06llu UTC ", static_cast<long long unsigned int>(microSec));
    logStream_ << T(tmp, 12);
  }
#ifdef __linux__
  if (threadId_ == 0) threadId_ = static_cast<pid_t>(::syscall(SYS_gettid));
#elif defined __FreeBSD__
  if (threadId_ == 0) {
    threadId_ = pthread_getthreadid_np();
  }
#elif defined __OpenBSD__
  if (threadId_ == 0) {
    threadId_ = getthrid();
  }
#elif defined __sun
  if (threadId_ == 0) {
    threadId_ = static_cast<uint64_t>(pthread_self());
  }
#elif defined _WIN32 || defined __HAIKU__
  if (threadId_ == 0) {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    threadId_ = std::stoull(ss.str());
  }
#else
  if (threadId_ == 0) {
    pthread_threadid_np(NULL, &threadId_);
  }
#endif
  logStream_ << threadId_;
}

// LOG_COMPACT only <time><ThreadID><Level>
/**
 * @brief Construct a new Logger object, with Info level
 *
 */
Logger::Logger() : level_(kInfo) {
  formatTime();
  logStream_ << T(logLevelStr[level_], 7);
#ifdef TRANTOR_SPDLOG_SUPPORT
  spdLogMessageOffset_ = logStream_.bufferLength();
#endif
}

/**
 * @brief Construct a new Logger object, with specified log level
 *
 * @param level The log level
 */
Logger::Logger(LogLevel level) : level_(std::clamp(level, kTrace, kFatal)) {
  formatTime();
  logStream_ << T(logLevelStr[level_], 7);
#ifdef TRANTOR_SPDLOG_SUPPORT
  spdLogMessageOffset_ = logStream_.bufferLength();
#endif
}

/**
 * @brief Construct a new Logger object, with Fatal level
 *
 */
Logger::Logger(bool) : level_(kFatal) {
  formatTime();
  logStream_ << T(logLevelStr[level_], 7);
#ifdef TRANTOR_SPDLOG_SUPPORT
  spdLogMessageOffset_ = logStream_.bufferLength();
#endif
  if (errno != 0) {
    logStream_ << strerror_tl(errno) << " (errno=" << errno << ") ";
  }
}

/**
 * @brief Construct a new Logger object, with Info level
 * @param file The source file
 * @param line The line in the source file
 */
Logger::Logger(SourceFile file, int line) : sourceFile_(file), fileLine_(line), level_(kInfo) {
  formatTime();
  logStream_ << T(logLevelStr[level_], 7);
#ifdef TRANTOR_SPDLOG_SUPPORT
  spdLogMessageOffset_ = logStream_.bufferLength();
#endif
}

/**
 * @brief Construct a new Logger object, with specified log level
 *
 * @param file The source file
 * @param line The line in the source file
 * @param level The log level
 */
Logger::Logger(SourceFile file, int line, LogLevel level)
  : sourceFile_(file), fileLine_(line), level_(std::clamp(level, kTrace, kFatal)) {
  formatTime();
  logStream_ << T(logLevelStr[level_], 7);
#ifdef TRANTOR_SPDLOG_SUPPORT
  spdLogMessageOffset_ = logStream_.bufferLength();
#endif
}

/**
 * @brief Construct a new Logger object, with Fatal level
 *
 * @param file The source file
 * @param line The line in the source file
 */
Logger::Logger(SourceFile file, int line, bool) : sourceFile_(file), fileLine_(line), level_(kFatal) {
  formatTime();
  logStream_ << T(logLevelStr[level_], 7);
#ifdef TRANTOR_SPDLOG_SUPPORT
  spdLogMessageOffset_ = logStream_.bufferLength();
#endif
  if (errno != 0) {
    logStream_ << strerror_tl(errno) << " (errno=" << errno << ") ";
  }
}

/**
 * @brief Construct a new Logger object, with specified log level
 *
 * @param file The source file
 * @param line The line in the source file
 * @param level The log level
 * @param func SpdLog function
 */
Logger::Logger(SourceFile file, int line, LogLevel level, const char *func)
  : sourceFile_(file),
    fileLine_(line),
    level_(std::clamp(level, kTrace, kFatal))
#ifdef TRANTOR_SPDLOG_SUPPORT
    ,
    func_(func)
#endif
{
  formatTime();
  logStream_ << T(logLevelStr[level_], 7) << "[" << func << "] ";
#ifdef TRANTOR_SPDLOG_SUPPORT
  spdLogMessageOffset_ = logStream_.bufferLength();
#endif
}

/**
 * @brief Set the logger index.
 *
 * @param index The index
 */
Logger &Logger::setIndex(int index) {
  index_ = index;
  return *this;
}

/**
 * @brief Return the log stream.
 *
 */
LogStream &Logger::stream() {
  return logStream_;
}

/**
 * @brief Check whether trantor was build with spdlog support
 * @retval true if yes
 * @retval false if not - in this case, all the spdlog functions are noop
 *         functions
 */
bool Logger::hasSpdLogSupport() {
#ifdef TRANTOR_SPDLOG_SUPPORT
  return true;
#else
  return false;
#endif
}

#ifdef TRANTOR_SPDLOG_SUPPORT
// a map with int keys is more efficient than spdlog internal registry based on strings (logger name)
static std::map<int, std::shared_ptr<spdlog::logger>> spdLoggers;
// same sinks, but the format pattern is only "%v", for LOG_RAW[_TO]
static std::map<int, std::shared_ptr<spdlog::logger>> rawSpdLoggers;
static std::mutex                                     spdLoggersMtx;

/**
 * @brief Helper for uniform naming
 */
static std::string defaultSpdLoggerName(int index) {
  using namespace std::literals::string_literals;
  std::string loggerName = "trantor"s;
  if (index >= 0) loggerName.append(std::to_string(index));
  return loggerName;
}

/**
 * @brief Get a default spdlog::logger for the specified channel.
 * @details This helper function provides a default spdlog::logger with a
 *          similar output format as the existing non-spdlog trantor::Logger
 *          format.
 *
 *          If a default logger was already created for the channel, it is
 *          returned as-is.
 *
 *          Otherwise, a new spdlog::logger object named "trantor" (for
 *          index < 0) or "trantor<channel>" is created, registered with
 *          spdlog, and configured as follows:
 *          - it has the same sinks as the lowest (index) enabled channel,
 *            or those of the spdlog::default_logger(), which by defaults
 *            outputs to stdout (spdlog::sinks::stdout_color_mt),
 *          - its format pattern is set to resemble to the existing
 *            non-spdlog trantor::Logger format
 *            ("%Y%m%d %T.%f %6t %^%=8l%$ [%!] %v - %s:%#"),
 *          - the logging level is set to unfiltered (spdlog::level::trace)
 *            since the internal trantor/drogon level filtering is still
 *            managed by trantor:::Logger,
 *          - the flush level is set to spdlog::level::error.
 * @note To add custom sinks to all the channels, you can do that this way:
 *        -# (optional) add your sinks to spdlog::default_logger(),
 *        -# create the default logger for the default channel using
 *           getDefaultSpdLogger(-1),
 *        -# if not done at step 1., add your sinks to this logger,
 *        -# enable the logger with enableSpdLog(),
 *        -# for the other channels, invoke enableSpdLog(index).
 * @remarks The created spdlog::logger is automatically registered
 *          with the spdlog logger registry.
 * @param[in] channel index (-1 = default channel).
 * @return the default spdlog logger for the channel.
 */
std::shared_ptr<spdlog::logger> Logger::getDefaultSpdLogger(int index) {
  auto loggerName = defaultSpdLoggerName(index);
  auto logger     = spdlog::get(loggerName);
  if (logger) return logger;

  // Create a new spdlog logger with the same sinks as the current default
  // Logger or spdlog logger
  auto &sinks =
    ((spdLoggers.begin() != spdLoggers.end() ? spdLoggers.begin()->second : spdlog::default_logger()))->sinks();
  logger = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());
  // keep a log format similar to the existing one, but with coloured
  // level on console since it's nice :)
  // see reference: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  logger->set_pattern("%Y%m%d %T.%f %6t %^%=8l%$ [%!] %v - %s:%#");
  // the filtering is done at Logger level, so no need to filter here
  logger->set_level(spdlog::level::trace);
  logger->flush_on(spdlog::level::err);
  spdlog::register_logger(logger);

  return logger;
}

/**
 * @brief Get the spdlog::logger set on the specified channel.
 * @param[in] channel index (-1 = default channel).
 * @return the logger, if set, else a null pointer.
 */
std::shared_ptr<spdlog::logger> Logger::getSpdLogger(int index) {
  std::lock_guard<std::mutex> lck(spdLoggersMtx);
  auto                        it = spdLoggers.find((index < 0) ? -1 : index);
  return (it == spdLoggers.end()) ? std::shared_ptr<spdlog::logger>() : it->second;
}

std::shared_ptr<spdlog::logger> Logger::getRawSpdLogger(int index) {
  // Create/delete RAW logger on-the fly
  // drawback: changes to the main logger's level or sinks won't be
  //           reflected in the raw logger once it's created
  if (index < -1) index = -1;
  std::lock_guard<std::mutex> lck(spdLoggersMtx);
  auto                        itMain = spdLoggers.find(index);
  auto                        itRaw  = rawSpdLoggers.find(index);

  if (itMain == spdLoggers.end()) {
    if (itRaw != rawSpdLoggers.end()) {
      spdlog::drop(itRaw->second->name());
      rawSpdLoggers.erase(itRaw);
    }
    return {};
  }
  auto mainLogger = itMain->second;
  if (itRaw != rawSpdLoggers.end()) return itRaw->second;
  auto rawLogger = std::make_shared<spdlog::logger>(mainLogger->name() + "_raw",
                                                    mainLogger->sinks().begin(),
                                                    mainLogger->sinks().end());
  rawLogger->set_pattern("%v");
  rawLogger->set_level(mainLogger->level());
  rawLogger->flush_on(mainLogger->flush_level());
  rawSpdLoggers[index] = rawLogger;
  spdlog::register_logger(rawLogger);
  return rawLogger;
}

/**
 * @brief Enable logging with spdlog for the default channel.
 * @param logger spdlog::logger object to use.
 *               If none given, defaults to getDefaultSpdLogger().
 * @remarks If provided, it is not registered with the spdlog logger
 *          registry, it's up to you to register/drop it.
 */
void Logger::enableSpdLog(std::shared_ptr<spdlog::logger> logger) {
  enableSpdLog(-1, logger);
}

/**
 * @brief Enable logging with spdlog for the specified channel.
 * @param index channel index (-1 = default channel).
 * @param logger spdlog::logger object to use.
 *               If none given, defaults to getDefaultSpdLogger(@p index).
 * @remarks If provided, it is not registered with the spdlog logger
 *          registry, it's up to you to register/drop it.
 */
void Logger::enableSpdLog(int index, std::shared_ptr<spdlog::logger> logger) {
  if (index < -1) index = -1;
  std::lock_guard<std::mutex> lck(spdLoggersMtx);
  spdLoggers[index] = logger ? logger : getDefaultSpdLogger(index);
}

/**
 * @brief Disable logging with spdlog for the default channel
 * @remarks The spdlog::logger object is unregistered and
 *          destroyed only if it was created by getDefaultSpdLogger().
 *          Custom loggers are only unset.
 */
void Logger::disableSpdLog() {
  disableSpdLog(-1);
}

/**
 * @brief Disable logging with spdlog for the specified channel.
 * @param[in] channel index (-1 = default channel).
 * @remarks The spdlog::logger object is unregistered and
 *          destroyed only if it was created by
 *          getDefaultSpdLogger(@p index).
 *          Custom loggers are only unset.
 */
void Logger::disableSpdLog(int index) {
  std::lock_guard<std::mutex> lck(spdLoggersMtx);
  if (index < -1) index = -1;
  auto it = spdLoggers.find(index);
  if (it == spdLoggers.end()) return;
  // auto-unregister
  if (it->second->name() == defaultSpdLoggerName(index)) spdlog::drop(it->second->name());
  spdLoggers.erase(it);
}

#endif  // TRANTOR_SPDLOG_SUPPORT

Logger::~Logger() {
#ifdef TRANTOR_SPDLOG_SUPPORT
  auto spdLogger = getSpdLogger(index_);
  if (spdLogger) {
    spdlog::source_loc spdLocation;
    if (sourceFile_.data_) spdLocation = {sourceFile_.data_, fileLine_, func_ ? func_ : ""};
    spdlog::string_view_t message(logStream_.bufferData(), logStream_.bufferLength());
    message.remove_prefix(spdLogMessageOffset_);

#if defined(SPDLOG_VERSION) && (SPDLOG_VERSION >= 10600)
    spdLogger->log(
      std::chrono::system_clock::time_point(std::chrono::duration<int64_t, std::micro>(date_.microSecondsSinceEpoch())),
      spdLocation,
      spdlog::level::level_enum(level_),
      message);
#else  // very old version, cannot specify time
    spdLogger->log(spdLocation, spdlog::level::level_enum(level_), message);
#endif
    return;
  }
#endif  // TRANTOR_SPDLOG_SUPPORT

  if (sourceFile_.data_)
    logStream_ << T(" - ", 3) << sourceFile_ << ':' << fileLine_ << '\n';
  else
    logStream_ << '\n';

  if (index_ < 0) {
    auto &oFunc = Logger::outputFunc_();
    if (!oFunc) return;
    oFunc(logStream_.bufferData(), logStream_.bufferLength());
    if (level_ >= kError) Logger::flushFunc_()();
  } else {
    auto &oFunc = Logger::outputFunc_(index_);
    if (!oFunc) return;
    oFunc(logStream_.bufferData(), logStream_.bufferLength());
    if (level_ >= kError) Logger::flushFunc_(index_)();
  }
}

}  // namespace trantor
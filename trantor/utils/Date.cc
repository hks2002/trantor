/**
 *
 *  Date.cc
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

#include "Date.h"

#ifndef _WIN32
#include <sys/time.h>
#else
#include <time.h>
#include <winsock2.h>
#endif

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#include "StringFunctions.h"

namespace trantor {
/**
 * @brief define microseconds per second
 *
 */
const int64_t MICRO_SECONDS_PRE_SEC = 1000000LL;

/**
 * @brief Construct a new Date instance.
 */
Date::Date() : microSecondsSinceEpoch_(0){};

/**
 * @brief Construct a new Date instance.
 *
 * @param microSec The microseconds from 1970-01-01 00:00:00.
 */

Date::Date(int64_t microSec) : microSecondsSinceEpoch_(microSec){};
/**
 * @brief Constructor for Date class that initializes the date and time based on the provided parameters.
 *
 * @param year The year value for the date, such as 2018
 * @param month The month value for the date, range 1-12
 * @param day The day value for the date, range 1-31
 * @param hour The hour value for the time, range 0-23
 * @param minute The minute value for the time, range 0-59
 * @param second The second value for the time, range 0-59
 * @param microSecond The microsecond value for the time
 */
Date::Date(unsigned int year,
           unsigned int month,
           unsigned int day,
           unsigned int hour,
           unsigned int minute,
           unsigned int second,
           unsigned int microSecond) {
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  tm.tm_isdst = -1;
  time_t epoch;
  tm.tm_year              = year - 1900;
  tm.tm_mon               = month - 1;
  tm.tm_mday              = day;
  tm.tm_hour              = hour;
  tm.tm_min               = minute;
  tm.tm_sec               = second;
  epoch                   = mktime(&tm);
  microSecondsSinceEpoch_ = static_cast<int64_t>(epoch) * MICRO_SECONDS_PRE_SEC + microSecond;
}

/**
 * @brief Get the tm struct for the time point.
 *
 * @return struct tm
 */
struct tm Date::tmStruct() const {
  time_t    seconds = static_cast<time_t>(microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC);
  struct tm tm_time;
#ifndef _WIN32
  gmtime_r(&seconds, &tm_time);
#else
  gmtime_s(&tm_time, &seconds);
#endif
  return tm_time;
}

#ifdef _WIN32
/**
 * @brief Retrieves the current time and timezone information.
 *
 * @param tp pointer to a timeval structure to store the time
 * @param tzp pointer to timezone information (not used in this implementation)
 *
 * @return 0 indicating success
 *
 * @throws None
 */
int gettimeofday(timeval *tp, void *tzp) {
  time_t     clock;
  struct tm  tm;
  SYSTEMTIME wtm;

  GetLocalTime(&wtm);
  tm.tm_year  = wtm.wYear - 1900;
  tm.tm_mon   = wtm.wMonth - 1;
  tm.tm_mday  = wtm.wDay;
  tm.tm_hour  = wtm.wHour;
  tm.tm_min   = wtm.wMinute;
  tm.tm_sec   = wtm.wSecond;
  tm.tm_isdst = -1;
  clock       = mktime(&tm);
  tp->tv_sec  = static_cast<long>(clock);
  tp->tv_usec = wtm.wMilliseconds * 1000;

  return (0);
}
#endif

/**
 * @brief Create a Date object that represents the current time.
 *
 * @return const Date
 */
const Date Date::date() {
#ifndef _WIN32
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Date(seconds * MICRO_SECONDS_PRE_SEC + tv.tv_usec);
#else
  timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Date(seconds * MICRO_SECONDS_PRE_SEC + tv.tv_usec);
#endif
}

/**
 * @brief Same as the date() method.
 *
 * @return const Date
 */
const Date Date::now() {
  return Date::date();
}

/**
 * @brief Get the number of milliseconds since 1970-01-01 00:00.
 *
 * @return int64_t
 */
int64_t Date::microSecondsSinceEpoch() const {
  return microSecondsSinceEpoch_;
}

/**
 * @brief Get the number of seconds since 1970-01-01 00:00.
 *
 * @return int64_t
 */
int64_t Date::secondsSinceEpoch() const {
  return microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC;
}

/**
 * @brief Return true if the time point is in a same second as another.
 *
 * @param date
 * @return true
 * @return false
 */
bool Date::isSameSecond(const Date &date) const {
  return microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC == date.microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC;
}

/**
 * @brief Return true if the time point is in a same second as another.
 *
 * @param date
 * @return true
 * @return false
 */
bool Date::isSameSecond(Date &&date) const {
  return microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC == date.microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC;
}

/**
 * @brief Swap the time point with another.
 *
 * @param that
 */
void Date::swap(Date &that) {
  std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
}

/**
 * @brief Returns the timezone offset for the Date.
 *
 * @return the timezone offset
 */
int64_t Date::timezoneOffset() {
  static int64_t offset = -(Date(1970, 1, 3).secondsSinceEpoch() - 2LL * 3600LL * 24LL);
  return offset;
}

/**
 * @brief From DB string to trantor UTC time.
 *
 * Inverse of toDbString()
 */
Date Date::fromDbString(std::string_view datetime) {
  return fromDbStringLocal(datetime).after(static_cast<double>(timezoneOffset()));
}

/**
 * @brief From DB string to trantor local time zone.
 *
 * Inverse of toDbStringLocal()
 */
Date Date::fromDbStringLocal(std::string_view datetime) {
  unsigned int year = {0}, month = {0}, day = {0}, hour = {0}, minute = {0}, second = {0}, microSecond = {0};
  std::vector<std::string> &&v = trantor::utils::splitString(datetime, " ");
  if (2 == v.size()) {
    // date
    std::vector<std::string> date = trantor::utils::splitString(v[0], "-");
    if (3 == date.size()) {
      year  = std::stol(date[0]);
      month = std::stol(date[1]);
      day   = std::stol(date[2]);
      // time
      std::vector<std::string> time = trantor::utils::splitString(v[1], ":");
      if (2 < time.size()) {
        hour         = std::stol(time[0]);
        minute       = std::stol(time[1]);
        auto seconds = trantor::utils::splitString(time[2], ".");
        second       = std::stol(seconds[0]);
        if (1 < seconds.size()) {
          if (seconds[1].length() > 6) {
            seconds[1].resize(6);
          } else if (seconds[1].length() < 6) {
            seconds[1].append(6 - seconds[1].length(), '0');
          }
          microSecond = std::stol(seconds[1]);
        }
      }
    }
  }
  return Date(year, month, day, hour, minute, second, microSecond);
}

/**
 * @brief Return a new Date instance that represents the time after some seconds from *this.
 *
 * @param second
 * @return const Date
 */
const Date Date::after(double second) const {
  return Date(static_cast<int64_t>(microSecondsSinceEpoch_ + second * MICRO_SECONDS_PRE_SEC));
}

/**
 * @brief Return a new Date instance that equals to *this, but with zero
 * microseconds.
 *
 * @return const Date
 */
const Date Date::roundSecond() const {
  return Date(microSecondsSinceEpoch_ - (microSecondsSinceEpoch_ % MICRO_SECONDS_PRE_SEC));
}

/**
 * @brief Return a new Date instance that equals to * this, but with zero
 * hours, minutes, seconds and microseconds.
 *
 * @return const Date
 */
const Date Date::roundDay() const {
  struct tm t;
  time_t    seconds = static_cast<time_t>(microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC);

#ifndef _WIN32
  localtime_r(&seconds, &t);
#else
  localtime_s(&t, &seconds);
#endif

  t.tm_hour = 0;
  t.tm_min  = 0;
  t.tm_sec  = 0;
  return Date(mktime(&t) * MICRO_SECONDS_PRE_SEC);
}

/**
 * @brief Generate a UTC time string for database.
 */
std::string Date::toDbString() const {
  return after(static_cast<double>(-timezoneOffset())).toDbStringLocal();
}

/**
 * @brief Generate a local time zone string for database.
 * @note Examples:
 *  - "2018-01-01" if hours, minutes, seconds and microseconds are zero
 *  - "2018-01-01 10:10:25" if the microsecond is zero
 *  - "2018-01-01 10:10:25:102414" if the microsecond is not zero
 */
std::string Date::toDbStringLocal() const {
  char      buf[128] = {0};
  time_t    seconds  = static_cast<time_t>(microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC);
  struct tm tm_time;

#ifndef _WIN32
  localtime_r(&seconds, &tm_time);
#else
  localtime_s(&tm_time, &seconds);
#endif

  bool showMicroseconds = (microSecondsSinceEpoch_ % MICRO_SECONDS_PRE_SEC != 0);
  if (showMicroseconds) {
    int microseconds = static_cast<int>(microSecondsSinceEpoch_ % MICRO_SECONDS_PRE_SEC);
    snprintf(buf,
             sizeof(buf),
             "%4d-%02d-%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour,
             tm_time.tm_min,
             tm_time.tm_sec,
             microseconds);
  } else {
    if (*this == roundDay()) {
      snprintf(buf, sizeof(buf), "%4d-%02d-%02d", tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday);
    } else {
      snprintf(buf,
               sizeof(buf),
               "%4d-%02d-%02d %02d:%02d:%02d",
               tm_time.tm_year + 1900,
               tm_time.tm_mon + 1,
               tm_time.tm_mday,
               tm_time.tm_hour,
               tm_time.tm_min,
               tm_time.tm_sec);
    }
  }
  return buf;
}

/**
 * @brief Generate a UTC time string.
 * Converts the date to a formatted string with the option to include microseconds.
 *
 * @param showMicroseconds boolean flag indicating whether to include microseconds in the formatted string
 *
 * @return formatted string representing the date
 * @note Examples:
 *  - "20180101 10:10:25" if the @p showMicroseconds is false
 *  - "20180101 10:10:25:102414" if the @p showMicroseconds is true
 * @throws None
 */
std::string Date::toFormattedString(bool showMicroseconds) const {
  //  std::cout<<"toFormattedString"<<std::endl;
  char      buf[128] = {0};
  time_t    seconds  = static_cast<time_t>(microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC);
  struct tm tm_time;

#ifndef _WIN32
  gmtime_r(&seconds, &tm_time);
#else
  gmtime_s(&tm_time, &seconds);
#endif

  if (showMicroseconds) {
    int microseconds = static_cast<int>(microSecondsSinceEpoch_ % MICRO_SECONDS_PRE_SEC);
    snprintf(buf,
             sizeof(buf),
             "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour,
             tm_time.tm_min,
             tm_time.tm_sec,
             microseconds);
  } else {
    snprintf(buf,
             sizeof(buf),
             "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour,
             tm_time.tm_min,
             tm_time.tm_sec);
  }
  return buf;
}

/**
 * @brief Generate a local time zone string, the format of the string is
 * same as the method toFormattedString
 *
 * @param showMicroseconds
 * @return std::string
 */
std::string Date::toFormattedStringLocal(bool showMicroseconds) const {
  //  std::cout<<"toFormattedString"<<std::endl;
  char      buf[128] = {0};
  time_t    seconds  = static_cast<time_t>(microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC);
  struct tm tm_time;

#ifndef _WIN32
  localtime_r(&seconds, &tm_time);
#else
  localtime_s(&tm_time, &seconds);
#endif

  if (showMicroseconds) {
    int microseconds = static_cast<int>(microSecondsSinceEpoch_ % MICRO_SECONDS_PRE_SEC);
    snprintf(buf,
             sizeof(buf),
             "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour,
             tm_time.tm_min,
             tm_time.tm_sec,
             microseconds);
  } else {
    snprintf(buf,
             sizeof(buf),
             "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour,
             tm_time.tm_min,
             tm_time.tm_sec);
  }
  return buf;
}

/**
 * @brief Generate a UTC time string formatted by the @p fmtStr
 * @param fmtStr is the format string for the function strftime()
 * @param showMicroseconds whether the microseconds are returned.
 * @note Examples:
 *  - "2018-01-01 10:10:25" if the @p fmtStr is "%Y-%m-%d %H:%M:%S" and the
 *    @p showMicroseconds is false
 *  - "2018-01-01 10:10:25:102414" if the @p fmtStr is "%Y-%m-%d %H:%M:%S"
 *    and the @p showMicroseconds is true
 */
std::string Date::toCustomizedFormattedString(std::string_view fmtStr, bool showMicroseconds) const {
  char      buf[256] = {0};
  time_t    seconds  = static_cast<time_t>(microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC);
  struct tm tm_time;

#ifndef _WIN32
  gmtime_r(&seconds, &tm_time);
#else
  gmtime_s(&tm_time, &seconds);
#endif

  strftime(buf, sizeof(buf), fmtStr.data(), &tm_time);
  if (!showMicroseconds) return std::string(buf);

  char decimals[12] = {0};
  int  microseconds = static_cast<int>(microSecondsSinceEpoch_ % MICRO_SECONDS_PRE_SEC);
  snprintf(decimals, sizeof(decimals), ".%06d", microseconds);
  return std::string(buf) + decimals;
}

/**
 * @brief Generate a local time zone string formatted by the @p fmtStr
 *
 * @param fmtStr
 * @param showMicroseconds
 * @return std::string
 */
std::string Date::toCustomizedFormattedStringLocal(std::string_view fmtStr, bool showMicroseconds) const {
  char      buf[256] = {0};
  time_t    seconds  = static_cast<time_t>(microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC);
  struct tm tm_time;

#ifndef _WIN32
  localtime_r(&seconds, &tm_time);
#else
  localtime_s(&tm_time, &seconds);
#endif

  strftime(buf, sizeof(buf), fmtStr.data(), &tm_time);
  if (!showMicroseconds) return std::string(buf);

  char decimals[12] = {0};
  int  microseconds = static_cast<int>(microSecondsSinceEpoch_ % MICRO_SECONDS_PRE_SEC);
  snprintf(decimals, sizeof(decimals), ".%06d", microseconds);
  return std::string(buf) + decimals;
}

/**
 * @brief Generate a UTC time string.
 *
 * @param fmtStr The format string.
 * @param str The string buffer for the generated time string.
 * @param len The length of the string buffer.
 */
void Date::toCustomizedFormattedString(std::string_view fmtStr, char *str, size_t len) const {
  // not safe
  time_t    seconds = static_cast<time_t>(microSecondsSinceEpoch_ / MICRO_SECONDS_PRE_SEC);
  struct tm tm_time;

#ifndef _WIN32
  gmtime_r(&seconds, &tm_time);
#else
  gmtime_s(&tm_time, &seconds);
#endif
  strftime(str, len, fmtStr.data(), &tm_time);
}

/**
 * @brief Return true if the time point is equal to another.
 *
 */
bool Date::operator==(const Date &date) const {
  return microSecondsSinceEpoch_ == date.microSecondsSinceEpoch_;
}

/**
 * @brief Return true if the time point is not equal to another.
 *
 */
bool Date::operator!=(const Date &date) const {
  return microSecondsSinceEpoch_ != date.microSecondsSinceEpoch_;
}

/**
 * @brief Return true if the time point is earlier than another.
 *
 */
bool Date::operator<(const Date &date) const {
  return microSecondsSinceEpoch_ < date.microSecondsSinceEpoch_;
}

/**
 * @brief Return true if the time point is later than another.
 *
 */
bool Date::operator>(const Date &date) const {
  return microSecondsSinceEpoch_ > date.microSecondsSinceEpoch_;
}

/**
 * @brief Return true if the time point is not earlier than another.
 *
 */
bool Date::operator>=(const Date &date) const {
  return microSecondsSinceEpoch_ >= date.microSecondsSinceEpoch_;
}

/**
 * @brief Return true if the time point is not later than another.
 *
 */
bool Date::operator<=(const Date &date) const {
  return microSecondsSinceEpoch_ <= date.microSecondsSinceEpoch_;
}

}  // namespace trantor

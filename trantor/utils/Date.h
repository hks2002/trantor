/**
 *
 *  @file Date.h
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

#pragma once

#include "trantor/exports.h"
#include <cstdint>
#include <string>

namespace trantor {
/**
 * @brief This class represents a time point.
 *
 */
class TRANTOR_EXPORT Date {
public:
  Date();
  explicit Date(int64_t microSec);
  Date(unsigned int year,
       unsigned int month,
       unsigned int day,
       unsigned int hour        = 0,
       unsigned int minute      = 0,
       unsigned int second      = 0,
       unsigned int microSecond = 0);

  static const Date date();
  static const Date now();

  int64_t           microSecondsSinceEpoch() const;
  int64_t           secondsSinceEpoch() const;
  bool              isSameSecond(const Date &date) const;
  bool              isSameSecond(Date &&date) const;
  void              swap(Date &that);

  static int64_t    timezoneOffset();
  static Date       fromDbString(std::string_view datetime);
  static Date       fromDbStringLocal(std::string_view datetime);
  const Date        after(double second) const;
  const Date        roundSecond() const;
  const Date        roundDay() const;

  std::string       toDbString() const;
  std::string       toDbStringLocal() const;
  std::string       toFormattedString(bool showMicroseconds) const;
  std::string       toFormattedStringLocal(bool showMicroseconds) const;
  std::string       toCustomizedFormattedString(std::string_view fmtStr, bool showMicroseconds = false) const;
  std::string       toCustomizedFormattedStringLocal(std::string_view fmtStr, bool showMicroseconds = false) const;
  void              toCustomizedFormattedString(std::string_view fmtStr, char *str, size_t len) const;  // UTC

  // Comparators
  bool operator==(const Date &date) const;
  bool operator!=(const Date &date) const;
  bool operator<(const Date &date) const;
  bool operator>(const Date &date) const;
  bool operator>=(const Date &date) const;
  bool operator<=(const Date &date) const;

private:
  struct tm tmStruct() const;
  int64_t   microSecondsSinceEpoch_{0};
};
}  // namespace trantor

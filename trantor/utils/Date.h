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

#ifndef TRANTOR_DATE_H
#define TRANTOR_DATE_H

#include "trantor/exports.h"
#include <cstdint>
#include <string_view>

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
  static Date       fromDbString(std::string_view datetime);
  static Date       fromDbStringLocal(std::string_view datetime);
  static int64_t    timezoneOffset();

  int64_t           microSecondsSinceEpoch() const;
  int64_t           secondsSinceEpoch() const;
  bool              isSameSecond(const Date &date) const;
  bool              isSameSecond(Date &&date) const;

  void              swap(Date &that);

  const Date        after(double second) const;
  const Date        roundSecond() const;
  const Date        roundDay() const;

  std::string       toDbString() const;
  std::string       toDbStringLocal() const;
  std::string       toFormattedString(bool showMicroseconds) const;
  std::string       toFormattedStringLocal(bool showMicroseconds) const;
  std::string       toCustomFormattedString(std::string_view fmtStr, bool showMicroseconds = false) const;
  std::string       toCustomFormattedStringLocal(std::string_view fmtStr, bool showMicroseconds = false) const;
  void              toCustomFormattedString(std::string_view fmtStr, char *str, size_t len) const;  // UTC

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
#endif  // TRANTOR_DATE_H
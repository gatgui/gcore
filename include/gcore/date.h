/*

Copyright (C) 2010  Gaetan Guidet

This file is part of gcore.

gcore is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

gcore is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#ifndef __gcore_date_h_
#define __gcore_date_h_

#include <gcore/string.h>
#include <gcore/platform.h>

namespace gcore {
  
  GCORE_API String GetDate();
  
  class GCORE_API Date {
    public:
      
      // NOTE: in diffs, a month a always 30 days and a year 365 days
      //       you'll notice that 30 * 12 != 365, so avoid those kind
      //       of computation for diffs
      
      enum Month {
        UnknownMonth = -1,
        January,
        February,
        March,
        April,
        May,
        June,
        July,
        August,
        September,
        October,
        November,
        December
      };
      
      enum Day {
        UnknownDay = -1,
        Sunday,
        Monday,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday
      };
      
      Date();
      Date(Int64 t, bool asDiff=false);
      Date(const Date &rhs);
      ~Date();
      
      Date& operator=(const Date &rhs);
      
      bool operator==(const Date &rhs) const;
      bool operator<(const Date &rhs) const;
      
      Date& operator-=(const Date &rhs);
      Date& operator+=(const Date &rhs);
      
      // this methods will return emtpy strings for difference time
      String format(const String &fmt) const;
      String strftime(const String &fmt) const;
      
      String toString() const;
      
      inline bool operator<=(const Date &rhs) const {
        return (operator==(rhs) || operator<(rhs));
      }
      
      inline bool operator>=(const Date &rhs) const {
        return (operator==(rhs) || !operator<(rhs));
      }
      
      inline bool operator>(const Date &rhs) const {
        return (!operator==(rhs) && !operator<(rhs));
      }
      
      inline int year() const {
        return mDateTime.tm_year + (mIsDiff ? 0 : 1900);
      }
      
      inline Month month() const {
        return Month(mDateTime.tm_mon);
      }
      
      inline int dayOfMonth() const {
        return mDateTime.tm_mday;
      }
      
      inline Day dayOfWeek() const {
        return Day(mDateTime.tm_wday);
      }
      
      inline int dayOfYear() const {
        return mDateTime.tm_yday;
      }
      
      inline int hour() const {
        return mDateTime.tm_hour;
      }
      
      inline int minute() const {
        return mDateTime.tm_min;
      }
      
      inline int second() const {
        return mDateTime.tm_sec;
      }
      
      inline bool isDiff() const {
        return mIsDiff;
      }
      
      void setYear(int year);
      void setMonth(int m);
      void setDayOfWeek(int d);
      void setDayOfMonth(int d);
      void setDayOfYear(int d);
      void setHour(int h);
      void setMinute(int m);
      void setSecond(int s);
      
      void set(Int64 t, bool asDiff=false);
      Int64 get(bool forceDiff=false) const;
      
      static Date Days(int n);
      static Date Weeks(int n);
      static Date Months(int n);
      static Date Years(int n);
      static Date Hours(int n);
      static Date Minutes(int n);
      static Date Seconds(int n);
      
      bool IsLeapYear(int year);
      int DaysInYear(int year);
      int DaysInMonth(int month, bool leapYear=false);
      
    protected:
      
      struct tm mDateTime;
      bool mIsDiff;
  };
  
  /*
  Format string
  
  %a
      The abbreviated weekday name according to the current locale. 
  %A
      The full weekday name according to the current locale. 
  %b
      The abbreviated month name according to the current locale. 
  %B
      The full month name according to the current locale. 
  %c
      The preferred date and time representation for the current locale. 
  %C
      The century number (year/100) as a 2-digit integer. (SU) 
  %d
      The day of the month as a decimal number (range 01 to 31). 
  %D
      Equivalent to %m/%d/%y. (Yecch -- for Americans only. Americans should note that in other countries %d/%m/%y is rather common. This means that in international context this format is ambiguous and should not be used.) (SU) 
  %e
      Like %d, the day of the month as a decimal number, but a leading zero is replaced by a space. (SU) 
  %F
      Equivalent to %Y-%m-%d (the ISO 8601 date format). (C99) 
  %h
      Equivalent to %b. (SU) 
  %H
      The hour as a decimal number using a 24-hour clock (range 00 to 23). 
  %I
      The hour as a decimal number using a 12-hour clock (range 01 to 12). 
  %j
      The day of the year as a decimal number (range 001 to 366). 
  %k
      The hour (24-hour clock) as a decimal number (range 0 to 23); single digits are preceded by a blank. (See also %H.) (TZ) 
  %l
      The hour (12-hour clock) as a decimal number (range 1 to 12); single digits are preceded by a blank. (See also %I.) (TZ) 
  %m
      The month as a decimal number (range 01 to 12). 
  %M
      The minute as a decimal number (range 00 to 59). 
  %n
      A newline character. (SU) 
  %p
      Either 'AM' or 'PM' according to the given time value, or the corresponding strings for the current locale. Noon is treated as 'pm' and midnight as 'am'. 
  %P
      Like %p but in lowercase: 'am' or 'pm' or a corresponding string for the current locale. (GNU) 
  %r
      The time in a.m. or p.m. notation. In the POSIX locale this is equivalent to '%I:%M:%S %p'. (SU) 
  %R
      The time in 24-hour notation (%H:%M). (SU) For a version including the seconds, see %T below. 
  %s
      The number of seconds since the Epoch, i.e., since 1970-01-01 00:00:00 UTC. (TZ) 
  %S
      The second as a decimal number (range 00 to 60). (The range is up to 60 to allow for occasional leap seconds.) 
  %t
      A tab character. (SU) 
  %T
      The time in 24-hour notation (%H:%M:%S). (SU) 
  %u
      The day of the week as a decimal, range 1 to 7, Monday being 1. See also %w. (SU) 
  %U
      The week number of the current year as a decimal number, range 00 to 53, starting with the first Sunday as the first day of week 01. See also %V and %W. 
  %w
      The day of the week as a decimal, range 0 to 6, Sunday being 0. See also %u. 
  %W
      The week number of the current year as a decimal number, range 00 to 53, starting with the first Monday as the first day of week 01. 
  %x
      The preferred date representation for the current locale without the time. 
  %X
      The preferred time representation for the current locale without the date. 
  %y
      The year as a decimal number without a century (range 00 to 99). 
  %Y
      The year as a decimal number including the century. 
  %z
      The time-zone as hour offset from GMT. Required to emit RFC 822-conformant dates (using "%a, %d %b %Y %H:%M:%S %z"). (GNU) 
  %Z
      The time zone or name or abbreviation. 
  %%
      A literal '%' character.
*/

}

inline std::ostream& operator<<(std::ostream &os, const gcore::Date &d) {
  os << d.toString();
  return os;
}

inline gcore::Date operator+(const gcore::Date &d0, const gcore::Date &d1) {
  gcore::Date rv(d0);
  rv += d1;
  return rv;
}

inline gcore::Date operator-(const gcore::Date &d0, const gcore::Date &d1) {
  gcore::Date rv(d0);
  rv -= d1;
  return rv;
}

#endif

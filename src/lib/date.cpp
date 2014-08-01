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

#include <gcore/date.h>

static const char *gsShortMonths[] = {
  "Jan",
  "Feb",
  "Mar",
  "Apr",
  "May",
  "Jun",
  "Jul",
  "Aug",
  "Sep",
  "Oct",
  "Nov",
  "Dec"
};

static const char *gsLongMonths[] = {
  "January",
  "February",
  "March",
  "April",
  "May",
  "June",
  "July",
  "August",
  "September",
  "October",
  "November",
  "December"
};

static const char *gsShortDays[] = {
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat"
};

static const char *gsLongDays[] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

// time_t is a signed value
// negative values might not be handled well on some system though 
static time_t gsSecsPerMinute =  60;
static time_t gsSecsPerHour   =  60 * gsSecsPerMinute;
static time_t gsSecsPerDay    =  24 * gsSecsPerHour;
static time_t gsSecsPerWeek   =   7 * gsSecsPerDay;
static time_t gsSecsPerMonth  =  30 * gsSecsPerDay;
static time_t gsSecsPerYear   = 365 * gsSecsPerDay;

namespace gcore {

String GetDate() {
  char buffer[256];
  time_t curt = time(NULL);
  struct tm *t = 0;
  t = localtime(&curt);
  sprintf(buffer, "%04d/%02d/%02d %02d:%02d:%02d", 1900+t->tm_year, 1+t->tm_mon, t->tm_mday,
                                                   t->tm_hour, t->tm_min, t->tm_sec);
  return buffer;
}

// ---

Date Date::Days(int n) {
  return Date(n * gsSecsPerDay, true);
}
  
Date Date::Weeks(int n) {
  return Date(n * gsSecsPerWeek, true);
}

Date Date::Months(int n) {
  return Date(n * gsSecsPerMonth, true);
}

Date Date::Years(int n) {
  return Date(n * gsSecsPerYear, true);
}

Date Date::Hours(int n) {
  return Date(n * gsSecsPerHour, true);
}

Date Date::Minutes(int n) {
  return Date(n * gsSecsPerMinute, true);
}

Date Date::Seconds(int n) {
  return Date(n, true);
}

bool Date::IsLeapYear(int year) {
  return (((year % 4) == 0 && (year % 100) != 0) || (year % 400) == 0);
}

int Date::DaysInYear(int year) {
  return (IsLeapYear(year) ? 366 : 365);
}

int Date::DaysInMonth(int month, bool leapYear) {
  static int sDaysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  if (month < 0 || month > 11) {
    return 0;
  }
  
  int ndays = sDaysInMonth[month];
  
  if (month == 1 && leapYear) {
    ndays += 1;
  }
  
  return ndays;
}

Date::Date() {
  set(time(NULL), false);
}

Date::Date(Int64 t, bool asDiff)
  : mIsDiff(asDiff) {
  set(t, asDiff);
}

Date::Date(const Date &rhs)
  : mDateTime(rhs.mDateTime), mIsDiff(rhs.mIsDiff) {
}

Date::~Date() {
}

void Date::set(struct tm &timedate) {
  // use mktime?
  mDateTime = timedate;
  mIsDiff = false;
}

void Date::set(Int64 t, bool asDiff) {
  mIsDiff = asDiff;
  
  if (!asDiff) {
    time_t tt = time_t(t);
    struct tm *ts = localtime(&tt);
    mDateTime = *ts;
    
  } else {
    mDateTime.tm_year = int(t / gsSecsPerYear);
    t = t % gsSecsPerYear;
    mDateTime.tm_yday = int(t / gsSecsPerDay);
    mDateTime.tm_mon  = int(t / gsSecsPerMonth);
    mDateTime.tm_mday = int((t % gsSecsPerMonth) / gsSecsPerDay);
    t = t % gsSecsPerDay;
    mDateTime.tm_hour = int(t / gsSecsPerHour);
    t = t % gsSecsPerHour;
    mDateTime.tm_min = int(t / gsSecsPerMinute);
    t = t % gsSecsPerMinute;
    mDateTime.tm_sec = int(t);
    mDateTime.tm_wday = -1;
  }
}

Int64 Date::get() const {
  Int64 r;
  if (mIsDiff) {
    r = mDateTime.tm_sec;
    r += gsSecsPerMinute * mDateTime.tm_min;
    r += gsSecsPerHour * mDateTime.tm_hour;
    r += gsSecsPerDay * mDateTime.tm_yday;
    r += gsSecsPerYear * mDateTime.tm_year;
    return r;
    
  } else {
    struct tm tmp = mDateTime;
    r = mktime(&tmp);
  }
  return r;
}

String Date::toString() const {
  
  char buffer[256];
  
  if (mIsDiff) {
    sprintf(buffer, "%d year(s), %d month(s), %d day(s), %d hour(s), %d minute(s), %d second(s)",
            mDateTime.tm_year, mDateTime.tm_mon, mDateTime.tm_mday,
            mDateTime.tm_hour, mDateTime.tm_min, mDateTime.tm_sec);
    
  } else {
    sprintf(buffer, "%04d/%02d/%02d %02d:%02d:%02d", 1900+mDateTime.tm_year,
                                                     1+mDateTime.tm_mon,
                                                     mDateTime.tm_mday,
                                                     mDateTime.tm_hour,
                                                     mDateTime.tm_min,
                                                     mDateTime.tm_sec);
  }
  return buffer;
}

String Date::strftime(const String &fmt) const {
  
  if (mIsDiff) {
    // allow only a subset of the formats?
    return "";
  }
  
  char buffer[4096];
  if (::strftime(buffer, 4096, fmt.c_str(), &mDateTime) == 0) {
    // too long for buffer
    return "";
  }
  return buffer;
}

String Date::format(const String &fmt) const {
  
  if (mIsDiff) {
    // allow only a subset of the formats?
    return "";
  }
  
  char buffer[256];
  
  String dt;
  
  size_t p0 = 0;
  size_t p1 = fmt.find('%', p0);
  
  while (p1 != String::npos) {
    dt.append(fmt, p0, p1-p0);
    if (p1+1 < fmt.length()) {
      ++p1;
      switch (fmt[p1]) {
      case 'a':
        dt.append(gsShortDays[mDateTime.tm_wday]);
        break;
      case 'A':
        dt.append(gsLongDays[mDateTime.tm_wday]);
        break;
      case 'b':
      case 'h':
        dt.append(gsShortMonths[mDateTime.tm_mon]);
        break;
      case 'B':
        dt.append(gsLongMonths[mDateTime.tm_mon]);
        break;
      case 'c':
        ::strftime(buffer, 256, "%c", &mDateTime);
        dt.append(buffer);
        break;
      case 'C': {
          int th = (1900 + mDateTime.tm_year) / 100;
          sprintf(buffer, "%02d", th);
          dt.append(buffer);
        }
        break;
      case 'd':
        sprintf(buffer, "%02d", mDateTime.tm_mday);
        dt.append(buffer);
        break;
      case 'D':
        sprintf(buffer, "%02d/%02d/%02d", mDateTime.tm_mon+1, mDateTime.tm_mday, mDateTime.tm_year+1900);
        dt.append(buffer);
        break;
      case 'e':
        sprintf(buffer, "%d", mDateTime.tm_mon+1);
        if (mDateTime.tm_mon < 9) {
          dt.append(1, ' ');
        }
        dt.append(buffer);
        break;
      case 'F':
        sprintf(buffer, "%04d-%02d-%02d", mDateTime.tm_year+1900, mDateTime.tm_mon+1, mDateTime.tm_mday);
        dt.append(buffer);
        break;
      case 'H':
        sprintf(buffer, "%02d", mDateTime.tm_hour);
        dt.append(buffer);
        break;
      case 'I': {
          int h = mDateTime.tm_hour % 12;
          sprintf(buffer, "%02d", h == 0 ? 12 : h);
          dt.append(buffer);
        }
        break;
      case 'j':
        sprintf(buffer, "%03d", mDateTime.tm_yday+1);
        dt.append(buffer);
        break;
      case 'k':
        sprintf(buffer, "%d", mDateTime.tm_hour);
        if (mDateTime.tm_hour <= 9) {
          dt.append(1, ' ');
        }
        dt.append(buffer);
        break;
      case 'l': {
          int h = mDateTime.tm_hour % 12;
          sprintf(buffer, "%d", h);
          if (h <= 9) {
            dt.append(1, ' ');
          }
          dt.append(buffer);
        }
        break;
      case 'm':
        sprintf(buffer, "%02d", mDateTime.tm_mon+1);
        dt.append(buffer);
        break;
      case 'M':
        sprintf(buffer, "%02d", mDateTime.tm_min);
        dt.append(buffer);
        break;
      case 'n':
        dt.append(1, '\n');
        break;
      case 'p':
        if (mDateTime.tm_hour < 12) {
          dt.append("AM");
        } else {
          dt.append("PM");
        }
        break;
      case 'P':
        if (mDateTime.tm_hour < 12) {
          dt.append("am");
        } else {
          dt.append("pm");
        }
        break;
      case 'r': {
          int h = mDateTime.tm_hour % 12;
          sprintf(buffer, "%02d:%02d:%02d %s", h == 0 ? 12 : h,
                                               mDateTime.tm_min,
                                               mDateTime.tm_sec,
                                               (mDateTime.tm_hour < 12 ? "am" : "pm"));
          dt.append(buffer);
        }
        break;
      case 'R':
        sprintf(buffer, "%02d:%02d", mDateTime.tm_hour, mDateTime.tm_min);
        dt.append(buffer);
        break;
      case 's': {
          struct tm tmc = mDateTime;
          time_t t = mktime(&tmc);
          sprintf(buffer, "%lu", t);
          dt.append(buffer);
        }
        break;
      case 'S':
        sprintf(buffer, "%02d", mDateTime.tm_sec);
        dt.append(buffer);
        break;
      case 't':
        dt.append(1, '\t');
        break;
      case 'T':
        sprintf(buffer, "%02d:%02d:%02d", mDateTime.tm_hour, mDateTime.tm_min, mDateTime.tm_sec);
        dt.append(buffer);
        break;
      case 'u':
        sprintf(buffer, "%d", (mDateTime.tm_wday == 0 ? 7 : mDateTime.tm_wday));
        dt.append(buffer);
        break;
      case 'U':
        ::strftime(buffer, 256, "%U", &mDateTime);
        dt.append(buffer);
        break;
      case 'w':
        sprintf(buffer, "%d", mDateTime.tm_wday);
        dt.append(buffer);
        break;
      case 'W':
        ::strftime(buffer, 256, "%W", &mDateTime);
        dt.append(buffer);
        break;
      case 'x':
        ::strftime(buffer, 256, "%x", &mDateTime);
        dt.append(buffer);
        break;
      case 'X':
        ::strftime(buffer, 256, "%X", &mDateTime);
        dt.append(buffer);
        break;
      case 'y':
        sprintf(buffer, "%02d", (mDateTime.tm_year+1900) % 100);
        dt.append(buffer);
        break;
      case 'Y':
        sprintf(buffer, "%04d", mDateTime.tm_year+1900);
        dt.append(buffer);
        break;
      case 'z':
        ::strftime(buffer, 256, "%z", &mDateTime);
        dt.append(buffer);
        break;
      case 'Z':
        ::strftime(buffer, 256, "%Z", &mDateTime);
        dt.append(buffer);
        break;
      case '%':
        dt.append(1, '%');
        break;
      //case 'E':
      //case 'G':
      //case 'g':
      //case 'O':
      //case 'V':
      //case '+':
      default:
        // not supported, leave the %<char> as it is in the format string
        dt.append(1, fmt[p1-1]);
        dt.append(1, fmt[p1]);
        break;
      }
    } else {
      // we have a trailing % character
      dt.append(1, fmt[p1]);
      break;
    }
    p0 = p1+1;
    p1 = fmt.find('%', p0);
  }

  dt.append(fmt, p0, String::npos);
  return dt;
}

void Date::setYear(int year) {
  // if not a diff, need to add 1900 to tm_year
  set(get() + ((year - (mIsDiff ? 0 : 1900)) - mDateTime.tm_year) * gsSecsPerYear, mIsDiff);
}

void Date::setMonth(int m) {
  set(get() + (m - mDateTime.tm_mon) * gsSecsPerMonth, mIsDiff);
}

void Date::setDayOfWeek(int d) {
  set(get() + (d - mDateTime.tm_wday) * gsSecsPerDay, mIsDiff);
}

void Date::setDayOfMonth(int d) {
  set(get() + (d - mDateTime.tm_mday) * gsSecsPerDay, mIsDiff);
}

void Date::setDayOfYear(int d) {
  set(get() + (d - mDateTime.tm_yday) * gsSecsPerDay, mIsDiff);
}

void Date::setHour(int h) {
  set(get() + (h - mDateTime.tm_hour) * gsSecsPerHour, mIsDiff);
}

void Date::setMinute(int m) {
  set(get() + (m - mDateTime.tm_min) * gsSecsPerMinute, mIsDiff);
}

void Date::setSecond(int s) {
  set(get() + (s - mDateTime.tm_sec), mIsDiff);
}

Date& Date::operator=(const Date &rhs) {
  mDateTime = rhs.mDateTime;
  mIsDiff = rhs.mIsDiff;
  return *this;
}

Date& Date::operator+=(const Date &rhs) {
  if (mIsDiff) {
    if (rhs.isDiff()) {
      set(get() + rhs.get(), true);
    } else {
      set(rhs.get() + get(), false);
    }
  } else {
    if (rhs.isDiff()) {
      set(get() + rhs.get(), false);
    } else {
      // INVALID OPERATION
      throw std::runtime_error("Cannot add 2 absolute dates");
    }
  }
  return *this;
}

Date& Date::operator-=(const Date &rhs) {
  if (mIsDiff) {
    if (rhs.isDiff()) {
      set(get() - rhs.get(), true);
    } else {
      // INVALID OPERATION
      throw std::runtime_error("Cannot subtract an absolute date from a diff date");
    }
  } else {
    if (rhs.isDiff()) {
      set(get() - rhs.get(), false);
    } else {
      set(get() - rhs.get(), true);
    }
  }
  return *this;
}

bool Date::operator==(const Date &rhs) const {
  return (get() == rhs.get());
}

bool Date::operator<(const Date &rhs) const {
  return (get() < rhs.get());
}

}


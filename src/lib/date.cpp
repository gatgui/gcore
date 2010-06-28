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
#include <gcore/platform.h>

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

namespace gcore {

std::string GetDate() {
  char buffer[256];
  time_t curt = time(NULL);
  struct tm *t = 0;
  t = localtime(&curt);
  sprintf(buffer, "%04d/%02d/%02d %02d:%02d:%02d", 1900+t->tm_year, 1+t->tm_mon, t->tm_mday,
                                                   t->tm_hour, t->tm_min, t->tm_sec);
  return buffer;
}

// ---

Date::Date() {
  time_t curt = time(NULL);
  struct tm *t = localtime(&curt);
  mDateTime = *t;
  mIsDiff = false;
}

Date::Date(const Date &rhs)
  : mDateTime(rhs.mDateTime), mIsDiff(rhs.mIsDiff) {
}

Date::~Date() {
}

std::string Date::toString() const {
  
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

std::string Date::strftime(const std::string &fmt) const {
  char buffer[4096];
  if (::strftime(buffer, 4096, fmt.c_str(), &mDateTime) == 0) {
    // too long for buffer
    return "";
  }
  return buffer;
}

std::string Date::format(const std::string &fmt) const {
  
  if (mIsDiff) {
    return "";
  }
  
  char buffer[256];
  
  std::string dt;
  
  size_t p0 = 0;
  size_t p1 = fmt.find('%', p0);
  
  while (p1 != std::string::npos) {
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

  dt.append(fmt, p0, std::string::npos);
  return dt;
}

Date& Date::operator=(const Date &rhs) {
  mDateTime = rhs.mDateTime;
  mIsDiff = rhs.mIsDiff;
  return *this;
}

Date& Date::operator-=(const Date &rhs) {
  
  mDateTime.tm_year -= rhs.mDateTime.tm_year;
  mDateTime.tm_mon  -= rhs.mDateTime.tm_mon;
  mDateTime.tm_mday -= rhs.mDateTime.tm_mday;
  mDateTime.tm_hour -= rhs.mDateTime.tm_hour;
  mDateTime.tm_min  -= rhs.mDateTime.tm_min;
  mDateTime.tm_sec  -= rhs.mDateTime.tm_sec;
  mDateTime.tm_yday  = -1;
  mDateTime.tm_wday  = -1;
  
  std::cout << "=> dYear = " << mDateTime.tm_year << std::endl;
  std::cout << "=> dMonth = " << mDateTime.tm_mon << std::endl;
  std::cout << "=> dDay = " << mDateTime.tm_mday << std::endl;
  std::cout << "=> dHour = " << mDateTime.tm_hour << std::endl;
  std::cout << "=> dMin = " << mDateTime.tm_min << std::endl;
  std::cout << "=> dSec = " << mDateTime.tm_sec << std::endl;
  
  mIsDiff = true;
  
  return *this;
}

bool Date::operator==(const Date &rhs) const {
  return (year() == rhs.year() &&
          dayOfYear() == rhs.dayOfYear() &&
          hour() == rhs.hour() &&
          minute() == rhs.minute() &&
          second() == rhs.second());
}

bool Date::operator<(const Date &rhs) const {
  if (year() < rhs.year()) {
    return true;
  } else if (year() > rhs.year()) {
    return false;
  } else {
    if (dayOfYear() < rhs.dayOfYear()) {
      return true;
    } else if (dayOfYear() > rhs.dayOfYear()) {
      return false;
    } else {
      if (hour() < rhs.hour()) {
        return true;
      } else if (hour() > rhs.hour()) {
        return false;
      } else {
        if (minute() < rhs.minute()) {
          return true;
        } else if (minute() > rhs.minute()) {
          return false;
        } else {
          return (second() < rhs.second());
        }
      }
    }
  }
}

}


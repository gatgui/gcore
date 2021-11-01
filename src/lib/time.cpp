/*
MIT License

Copyright (c) 2016 Gaetan Guidet

This file is part of gcore.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <gcore/time.h>
#include <gcore/platform.h>
#include <cmath>
#ifdef __APPLE__
#  include <mach/clock.h>
#  include <mach/mach.h>
#endif

namespace gcore
{

#ifndef _WIN32
#ifdef __APPLE__

clock_serv_t gClockServ;

class ClockServiceInitializer
{
public:
   ClockServiceInitializer()
   {
      host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &gClockServ);
   }
   ~ClockServiceInitializer()
   {
      mach_port_deallocate(mach_task_self(), gClockServ);
   }
};

ClockServiceInitializer _initializeClockService;

static inline void clock_gettime(struct timespec *ts)
{
   mach_timespec_t mts;
   clock_get_time(gClockServ, &mts);
   ts->tv_sec = mts.tv_sec;
   ts->tv_nsec = mts.tv_nsec;
}

#else // __APPLE__

static inline void clock_gettime(struct timespec *ts)
{
   clock_gettime(CLOCK_PROCESS_CPUTIME_ID, ts);
}

#endif

#else // !_WIN32

static double gsFreq = 0.0;
static double gsInvFreq = 0.0;

class FrequencyInitializer
{
public:
   FrequencyInitializer()
   {
      LARGE_INTEGER freq;
      QueryPerformanceFrequency(&freq);
      if (freq.QuadPart != 0)
      {
         gsFreq = double(freq.QuadPart);
         gsInvFreq = 1.0 / gsFreq;
      }
   }
   ~FrequencyInitializer()
   {
   }
};

FrequencyInitializer _frequencyInitializer;

#endif

// ---

const char* TimeCounter::UnitsString(TimeCounter::Units units, bool compact)
{
   static const char* sStrs[] = {"nanosecond(s)", "microsecond(s)", "millisecond(s)", "second(s)", "minute(s)", "hour(s)"};
   static const char* sShortStrs[] = {"ns", "us", "ms", "s", "m", "h"};
   
   int idx = int(units);
   
   if (idx < 0 || idx > Hours)
   {
      return (compact ? "" : "(unknown units)");
   }
   else
   {
      return (compact ? sShortStrs[idx] : sStrs[idx]);
   }
}

double TimeCounter::ConvertUnits(double val, TimeCounter::Units srcUnits, TimeCounter::Units dstUnits)
{
   static const double sConvertUnits[][Hours+1] = {
      {            1.0,    1.0/1000.0, 1.0/1000000.0, 1.0/1000000000.0, 1.0/60000000000.0, 1.0/3600000000000.0},
      {         1000.0,           1.0,    1.0/1000.0,    1.0/1000000.0,    1.0/60000000.0,    1.0/3600000000.0},
      {      1000000.0,        1000.0,           1.0,       1.0/1000.0,       1.0/60000.0,       1.0/3600000.0},
      {   1000000000.0,     1000000.0,        1000.0,              1.0,          1.0/60.0,          1.0/3600.0},
      {  60000000000.0,    60000000.0,       60000.0,             60.0,               1.0,            1.0/60.0},
      {3600000000000.0,  3600000000.0,     3600000.0,           3600.0,              60.0,                 1.0}
   };
   
   int src = int(srcUnits);
   int dst = int(dstUnits);
   
   if (src < 0 || dst < 0 || src > Hours || dst > Hours)
   {
      return val;
   }
   
   double rv = val * sConvertUnits[int(srcUnits)][int(dstUnits)];
   
   return rv;
}

// ---

TimeCounter::TimeCounter(TimeCounter::Units units)
   : mUnits(units)
{
   if (mUnits < NanoSeconds || mUnits > Hours)
   {
      mUnits = Seconds;
   }
   restart();
}

TimeCounter::TimeCounter(double value, Units units)
   : mUnits(units)
{
   if (!setValue(value))
   {
      // Invalid value
      mUnits = Seconds;
#ifdef _WIN32
      mCounter.QuadPart = 0;
#else
      mCounter.tv_sec = 0;
      mCounter.tv_nsec = 0;
#endif
   }
}

TimeCounter::TimeCounter(const TimeCounter &rhs)
   : mUnits(rhs.mUnits)
   , mCounter(rhs.mCounter)
{
}

TimeCounter::~TimeCounter()
{
}

bool TimeCounter::setUnits(Units units)
{
   if (units == CurrentUnits)
   {
      return true;
   }
   else if (units < NanoSeconds || units > Hours)
   {
      return false;
   }
   mUnits = units;
   return true;
}

void TimeCounter::restart()
{
#ifdef _WIN32
   QueryPerformanceCounter(&mCounter);
#else
   clock_gettime(&mCounter);
#endif
}

bool TimeCounter::setValue(double value, TimeCounter::Units units)
{
   if (units == CurrentUnits)
   {
      units = mUnits;
   }
   if (units < NanoSeconds || units > Hours)
   {
      return false;
   }
#ifdef _WIN32
   double counter = ConvertUnits(value, units, Seconds);
   counter *= gsFreq;
   mCounter.QuadPart = (LONGLONG) floor(counter + 0.5);
#else
   double secs = ConvertUnits(value, units, Seconds);
   mCounter.tv_sec = (time_t) floor(secs); // do not round here, it may lead to negative nsecs
   double remain = value - ConvertUnits(double(mCounter.tv_sec), Seconds, units);
   double nsecs = ConvertUnits(remain, units, NanoSeconds);
   mCounter.tv_nsec = (long) floor(nsecs + 0.5);
#endif
   mUnits = units;
   return true;
}

double TimeCounter::value(TimeCounter::Units units) const
{
   Units u = (units == CurrentUnits ? mUnits : units);
#ifdef _WIN32
   return ConvertUnits(double(mCounter.QuadPart) * gsInvFreq, Seconds, u);
#else
   return (ConvertUnits(double(mCounter.tv_sec), Seconds, u) + ConvertUnits(double(mCounter.tv_nsec), NanoSeconds, u));
#endif
}

TimeCounter TimeCounter::elapsed() const
{
   TimeCounter e(mUnits);
   e -= *this;
   return e;
}

TimeCounter& TimeCounter::operator=(const TimeCounter &rhs)
{
   if (this != &rhs)
   {
      mUnits = rhs.mUnits;
      mCounter = rhs.mCounter;
   }
   return *this;
}

TimeCounter& TimeCounter::operator+=(const TimeCounter &rhs)
{
#ifdef _WIN32
   mCounter.QuadPart += rhs.mCounter.QuadPart;
#else
   mCounter.tv_sec += rhs.mCounter.tv_sec;
   mCounter.tv_nsec += rhs.mCounter.tv_nsec;
#endif
   return *this;
}

TimeCounter& TimeCounter::operator-=(const TimeCounter &rhs)
{
#ifdef _WIN32
   mCounter.QuadPart -= rhs.mCounter.QuadPart;
#else
   mCounter.tv_sec -= rhs.mCounter.tv_sec;
   mCounter.tv_nsec -= rhs.mCounter.tv_nsec;
#endif
   return *this;
}

bool TimeCounter::operator==(const TimeCounter &rhs) const
{
   if (this != &rhs)
   {
#ifdef _WIN32
      return (mCounter.QuadPart == rhs.mCounter.QuadPart);
#else
      return (mCounter.tv_sec == rhs.mCounter.tv_sec && mCounter.tv_nsec == rhs.mCounter.tv_nsec);
#endif
   }
   return true;
}

bool TimeCounter::operator!=(const TimeCounter &rhs) const
{
   return !operator==(rhs);
}

bool TimeCounter::operator<=(const TimeCounter &rhs) const
{
   if (this != &rhs)
   {
#ifdef _WIN32
      return (mCounter.QuadPart <= rhs.mCounter.QuadPart);
#else
      return (mCounter.tv_sec < rhs.mCounter.tv_sec ||
              (mCounter.tv_sec == rhs.mCounter.tv_sec && mCounter.tv_nsec <= rhs.mCounter.tv_nsec));
#endif
   }
   return true;
}

bool TimeCounter::operator>=(const TimeCounter &rhs) const
{
   if (this != &rhs)
   {
#ifdef _WIN32
      return (mCounter.QuadPart >= rhs.mCounter.QuadPart);
#else
      return (mCounter.tv_sec > rhs.mCounter.tv_sec ||
              (mCounter.tv_sec == rhs.mCounter.tv_sec && mCounter.tv_nsec >= rhs.mCounter.tv_nsec));
#endif
   }
   return true;
}

bool TimeCounter::operator<(const TimeCounter &rhs) const
{
   if (this != &rhs)
   {
#ifdef _WIN32
      return (mCounter.QuadPart < rhs.mCounter.QuadPart);
#else
      return (mCounter.tv_sec < rhs.mCounter.tv_sec ||
              (mCounter.tv_sec == rhs.mCounter.tv_sec && mCounter.tv_nsec < rhs.mCounter.tv_nsec));
#endif
   }
   return false;
}

bool TimeCounter::operator>(const TimeCounter &rhs) const
{
   if (this != &rhs)
   {
#ifdef _WIN32
      return (mCounter.QuadPart > rhs.mCounter.QuadPart);
#else
      return (mCounter.tv_sec > rhs.mCounter.tv_sec ||
              (mCounter.tv_sec == rhs.mCounter.tv_sec && mCounter.tv_nsec > rhs.mCounter.tv_nsec));
#endif
   }
   return false;
}

TimeCounter& TimeCounter::operator=(double rhs)
{
   setValue(rhs);
   return *this;
}

TimeCounter& TimeCounter::operator+=(double rhs)
{
   TimeCounter tc(rhs, mUnits);
   return operator+=(tc);
}

TimeCounter& TimeCounter::operator-=(double rhs)
{
   TimeCounter tc(rhs, mUnits);
   return operator-=(tc);
}

bool TimeCounter::operator==(double rhs) const
{
   return (fabs(value() - rhs) < 0.000001);
}

bool TimeCounter::operator!=(double rhs) const
{
   return !operator==(rhs);
}

bool TimeCounter::operator<=(double rhs) const
{
   return (value() <= rhs);
}

bool TimeCounter::operator>=(double rhs) const
{
   return (value() >= rhs);
}

bool TimeCounter::operator<(double rhs) const
{
   return (value() < rhs);
}

bool TimeCounter::operator>(double rhs) const
{
   return (value() > rhs);
}

}

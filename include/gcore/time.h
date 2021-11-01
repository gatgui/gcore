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

#ifndef __gcore_time_h__
#define __gcore_time_h__

#include <gcore/config.h>
#include <gcore/platform.h>

namespace gcore
{
   class GCORE_API TimeCounter
   {
   public:
      enum Units
      {
         CurrentUnits = -1,
         NanoSeconds,
         MicroSeconds,
         MilliSeconds,
         Seconds,
         Minutes,
         Hours
      };
      
      static const char* UnitsString(Units units, bool compact=false);
      static double ConvertUnits(double val, Units srcUnits, Units dstUnits);
   
   public:
      TimeCounter(Units units=Seconds);
      TimeCounter(double value, Units units=Seconds);
      TimeCounter(const TimeCounter &rhs);
      ~TimeCounter();
      
      void restart();
      
      bool setUnits(Units units);
      inline Units units() const { return mUnits; }
      
      bool setValue(double value, Units units=CurrentUnits);
      double value(Units units=CurrentUnits) const;
      inline double nanoseconds() const { return value(NanoSeconds); }
      inline double microseconds() const { return value(MicroSeconds); }
      inline double milliseconds() const { return value(MilliSeconds); }
      inline double seconds() const { return value(Seconds); }
      inline double minutes() const { return value(Minutes); }
      inline double hours() const { return value(Hours); }
      
      TimeCounter elapsed() const;
      
      TimeCounter& operator=(const TimeCounter &rhs);
      TimeCounter& operator+=(const TimeCounter &rhs);
      TimeCounter& operator-=(const TimeCounter &rhs);
      
      bool operator==(const TimeCounter &rhs) const;
      bool operator!=(const TimeCounter &rhs) const;
      bool operator<=(const TimeCounter &rhs) const;
      bool operator>=(const TimeCounter &rhs) const;
      bool operator<(const TimeCounter &rhs) const;
      bool operator>(const TimeCounter &rhs) const;
      
      // double variants use mUnits for rhs
      TimeCounter& operator=(double rhs);
      TimeCounter& operator+=(double rhs);
      TimeCounter& operator-=(double rhs);
      
      bool operator==(double rhs) const;
      bool operator!=(double rhs) const;
      bool operator<=(double rhs) const;
      bool operator>=(double rhs) const;
      bool operator<(double rhs) const;
      bool operator>(double rhs) const;
      
   private:
      Units mUnits;
#ifdef _WIN32
      LARGE_INTEGER mCounter;
#else
      struct timespec mCounter;
#endif
   };
}

inline std::ostream& operator<<(std::ostream &os, const gcore::TimeCounter &tc)
{
   os << tc.value() << gcore::TimeCounter::UnitsString(tc.units(), true);
   return os;
}

inline gcore::TimeCounter operator+(const gcore::TimeCounter &tc0, const gcore::TimeCounter &tc1)
{
   gcore::TimeCounter rv(tc0);
   rv += tc1;
   return rv;
}

inline gcore::TimeCounter operator-(const gcore::TimeCounter &tc0, const gcore::TimeCounter &tc1)
{
   gcore::TimeCounter rv(tc0);
   rv -= tc1;
   return rv;
}

#endif

/*

Copyright (C) 2016~  Gaetan Guidet

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
#ifndef __gcore_time_h__
#define __gcore_time_h__

#include <gcore/config.h>

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

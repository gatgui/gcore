/*

Copyright (C) 2009, 2010  Gaetan Guidet

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
#ifndef __gcore_perfLog_h__
#define __gcore_perfLog_h__

#include <gcore/config.h>
#include <gcore/platform.h>
#include <gcore/time.h>

namespace gcore
{
   class GCORE_API Log;
   
   class GCORE_API PerfLog
   {
   public:
      
      friend class ScopedPerfLog;
      
      enum ShowFlags
      {
         ShowTotalTime = 0x01,
         ShowFuncTime = 0x02,
         ShowAvgTotalTime = 0x04,
         ShowAvgFuncTime = 0x08,
         ShowNumCalls = 0x10,
         ShowDetailed = 0x20,
         ShowFlat = 0x40,
         ShowDefaults = ShowTotalTime,
         ShowAll = ShowTotalTime|ShowFuncTime|ShowAvgTotalTime|ShowAvgFuncTime|ShowNumCalls
      };
      
      enum SortCriteria
      {
         SortIdentifier = 0,
         SortTotalTime,
         SortFuncTime,
         SortAvgTotalTime,
         SortAvgFuncTime,
         SortNumCalls,
         SortReverse = 0x1000
      };
      
      enum Output
      {
         ConsoleOutput = 0,
         LogOutput
      };
      
   public:
      
      static PerfLog& SharedInstance();
      static void Begin(const std::string &id);
      static void End();
      static void Print(Output output=ConsoleOutput, int flags=ShowDefaults, int sortBy=SortFuncTime, TimeCounter::Units units=TimeCounter::CurrentUnits);
      static void Print(std::ostream &os, int flags=ShowDefaults, int sortBy=SortFuncTime, TimeCounter::Units units=TimeCounter::CurrentUnits);
      static void Print(Log &log, int flags=ShowDefaults, int sortBy=SortFuncTime, TimeCounter::Units units=TimeCounter::CurrentUnits);
      static void Clear();
      
   public:
      
      PerfLog(TimeCounter::Units units=TimeCounter::Seconds);
      PerfLog(const PerfLog &rhs);
      ~PerfLog();
      
      PerfLog& operator=(const PerfLog&);
      
      void begin(const std::string &id);
      void end();
      void print(Output output=ConsoleOutput, int flags=ShowDefaults, int sortBy=SortFuncTime, TimeCounter::Units units=TimeCounter::CurrentUnits);
      void print(std::ostream &os, int flags=ShowDefaults, int sortBy=SortFuncTime, TimeCounter::Units units=TimeCounter::CurrentUnits);
      void print(Log &log, int flags=ShowDefaults, int sortBy=SortFuncTime, TimeCounter::Units units=TimeCounter::CurrentUnits);
      void clear();
      bool empty() const;
      void merge(const PerfLog &rhs);
      
   public:
      
      class GCORE_API BaseEntry
      {
      public:
      
         double totalTime;
         double selfTime;
         size_t callCount;
      
         BaseEntry();
         BaseEntry(const BaseEntry &);
         BaseEntry& operator=(const BaseEntry &rhs);
         
         void merge(const BaseEntry &rhs);
      };
   
      class GCORE_API Entry : public BaseEntry
      {
      public:
      
         std::map<std::string, Entry> subs;
      
         Entry();
         Entry(const Entry &);
         Entry& operator=(const Entry &);
         
         void merge(const Entry &rhs);
      };
   
      class GCORE_API StackItem
      {
      public:
         Entry *entry;
         std::string id;
         int recursionCount;
         TimeCounter start;
         TimeCounter selfStart;
      
         StackItem();
         StackItem(const std::string &_id, Entry *_entry);
         StackItem(const StackItem &);
         StackItem& operator=(const StackItem &);
      
         void stopAll(double &total, double &self, TimeCounter::Units units);
         double stopSelf(TimeCounter::Units units);
         void startSelf();
         double duration(TimeCounter::Units units) const;
      
      private:
      
         double duration(const TimeCounter &from, TimeCounter::Units units) const;
      
         bool selfStopped;
      };
   
      typedef std::map<std::string, BaseEntry> BaseEntryMap;
      typedef std::map<std::string, Entry> EntryMap;
      
      
   private:
      
      const char* unitsString(TimeCounter::Units units) const;
      double convertUnits(double val, TimeCounter::Units srcUnits, TimeCounter::Units dstUnits) const;
      
      
   private:
      
      BaseEntryMap mEntries;
      EntryMap mRootEntries;
      std::deque<StackItem> mEntryStack;
      TimeCounter::Units mUnits;
   };

   class GCORE_API ScopedPerfLog
   {
   public:
   
      ScopedPerfLog(const std::string &msg);
      ScopedPerfLog(PerfLog &plog, const std::string &msg);
      ~ScopedPerfLog();
   
   private:
   
      PerfLog &mPerfLog;
   
      ScopedPerfLog();
      ScopedPerfLog(ScopedPerfLog&);
      ScopedPerfLog& operator=(const ScopedPerfLog&);
   };
}

#endif


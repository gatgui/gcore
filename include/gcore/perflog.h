/*
MIT License

Copyright (c) 2010 Gaetan Guidet

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
#ifndef __gcore_perfLog_h__
#define __gcore_perfLog_h__

#include <gcore/config.h>
#include <gcore/platform.h>
#include <gcore/time.h>
#include <gcore/string.h>

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
      
      static PerfLog& Get();
      static void Begin(const String &id);
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
      
      void begin(const String &id);
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
      
         std::map<String, Entry> subs;
      
         Entry();
         Entry(const Entry &);
         Entry& operator=(const Entry &);
         
         void merge(const Entry &rhs);
      };
   
      class GCORE_API StackItem
      {
      public:
         Entry *entry;
         String id;
         int recursionCount;
         TimeCounter start;
         TimeCounter selfStart;
      
         StackItem();
         StackItem(const String &_id, Entry *_entry);
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
   
      typedef std::map<String, BaseEntry> BaseEntryMap;
      typedef std::map<String, Entry> EntryMap;
      
      
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
   
      ScopedPerfLog(const String &msg);
      ScopedPerfLog(PerfLog &plog, const String &msg);
      ~ScopedPerfLog();
   
   private:
   
      PerfLog &mPerfLog;
   
      ScopedPerfLog();
      ScopedPerfLog(ScopedPerfLog&);
      ScopedPerfLog& operator=(const ScopedPerfLog&);
   };
}

#endif


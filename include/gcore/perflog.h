#ifndef __gcore_perfLog_h__
#define __gcore_perfLog_h__

#include <gcore/config.h>
#include <gcore/platform.h>

namespace gcore
{
   class GCORE_API Log;
   
   class GCORE_API PerfLog
   {
   public:
      
      friend class ScopedPerfLog;
      
      enum Units
      {
         CurrentUnits = -1,
         NanoSeconds,
         MilliSeconds,
         Seconds,
         Minutes,
         Hours
      };
      
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
      static void Print(Output output=ConsoleOutput, int flags=ShowDefaults, int sortBy=SortFuncTime, Units units=CurrentUnits);
      static void Print(std::ostream &os, int flags=ShowDefaults, int sortBy=SortFuncTime, Units units=CurrentUnits);
      static void Print(Log &log, int flags=ShowDefaults, int sortBy=SortFuncTime, Units units=CurrentUnits);
      static void Clear();
      
      static const char* UnitsString(Units units);
      static double ConvertUnits(double val, Units srcUnits, Units dstUnits);
      
      
   public:
      
      PerfLog(Units units=Seconds);
      PerfLog(const PerfLog &rhs);
      ~PerfLog();
      
      PerfLog& operator=(const PerfLog&);
      
      void begin(const std::string &id);
      void end();
      void print(Output output=ConsoleOutput, int flags=ShowDefaults, int sortBy=SortFuncTime, Units units=CurrentUnits);
      void print(std::ostream &os, int flags=ShowDefaults, int sortBy=SortFuncTime, Units units=CurrentUnits);
      void print(Log &log, int flags=ShowDefaults, int sortBy=SortFuncTime, Units units=CurrentUnits);
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
         #ifdef _WIN32
         typedef LARGE_INTEGER TimeCounter;
         #else
         typedef struct timespec TimeCounter;
         #endif
      
         Entry *entry;
         std::string id;
         int recursionCount;
         TimeCounter start;
         TimeCounter selfStart;
      
         StackItem();
         StackItem(const std::string &_id, Entry *_entry);
         StackItem(const StackItem &);
         StackItem& operator=(const StackItem &);
      
         void stopAll(double &total, double &self, Units units);
         double stopSelf(Units units);
         void startSelf();
         double duration(Units units) const;
      
      private:
      
         double duration(const TimeCounter &from, Units units) const;
      
         bool selfStopped;
      };
   
      typedef std::map<std::string, BaseEntry> BaseEntryMap;
      typedef std::map<std::string, Entry> EntryMap;
      
      
   private:
      
      const char* unitsString(Units units) const;
      double convertUnits(double val, Units srcUnits, Units dstUnits) const;
      
      
   private:
      
      BaseEntryMap mEntries;
      EntryMap mRootEntries;
      std::deque<StackItem> mEntryStack;
      Units mUnits;
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


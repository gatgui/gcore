#include <gcore/perflog.h>
#include <gcore/log.h>
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

#else

static inline void clock_gettime(struct timespec *ts)
{
   clock_gettime(CLOCK_PROCESS_CPUTIME_ID, ts);
}

#endif
#endif

PerfLog::StackItem::StackItem()
   : entry(0)
   , id("")
   , recursionCount(0)
   , selfStopped(false)
{
#ifdef _WIN32
   QueryPerformanceCounter(&start);
#else
   clock_gettime(&start);
#endif
   selfStart = start;
}

PerfLog::StackItem::StackItem(const std::string &_id, Entry *_entry)
   : entry(_entry)
   , id(_id)
   , recursionCount(0)
   , selfStopped(false)
{
#ifdef _WIN32
   QueryPerformanceCounter(&start);
#else
   clock_gettime(&start);
#endif
   selfStart = start;
}

PerfLog::StackItem::StackItem(const PerfLog::StackItem &rhs)
   : entry(rhs.entry)
   , id(rhs.id)
   , recursionCount(rhs.recursionCount)
   , start(rhs.start)
   , selfStart(rhs.selfStart)
   , selfStopped(rhs.selfStopped)
{
}

PerfLog::StackItem& PerfLog::StackItem::operator=(const PerfLog::StackItem &rhs)
{
   if (this != &rhs)
   {
      entry = rhs.entry;
      id = rhs.id;
      recursionCount = rhs.recursionCount;
      start = rhs.start;
      selfStart = rhs.selfStart;
      selfStopped = rhs.selfStopped;
   }
   return *this;
}

void PerfLog::StackItem::stopAll(double &total, double &self, PerfLog::Units units)
{
#ifdef _WIN32
   LARGE_INTEGER end, freq;
   
   QueryPerformanceFrequency(&freq);
   
   if (freq.QuadPart == 0)
   {
      total = 0;
      self = 0;
      return;
   }
   
   QueryPerformanceCounter(&end);
   
   total = double(end.QuadPart - start.QuadPart) / double(freq.QuadPart);
   total = PerfLog::ConvertUnits(total, Seconds, units);
   
   if (selfStopped)
   {
      self = 0;
   }
   else
   {
      self = double(end.QuadPart - selfStart.QuadPart) / double(freq.QuadPart);
      self = PerfLog::ConvertUnits(self, Seconds, units);
      selfStopped = true;
   }
#else
   struct timespec to;
   
   clock_gettime(&to);
   
   double dsec  = double(to.tv_sec - start.tv_sec);
   double dnsec = double(to.tv_nsec - start.tv_nsec);
   
   total = (PerfLog::ConvertUnits(dsec, Seconds, units) + PerfLog::ConvertUnits(dnsec, NanoSeconds, units));
   
   if (selfStopped)
   {
      self = 0;
   }
   else
   {
      dsec  = double(to.tv_sec - selfStart.tv_sec);
      dnsec = double(to.tv_nsec - selfStart.tv_nsec);
      self = (PerfLog::ConvertUnits(dsec, Seconds, units) + PerfLog::ConvertUnits(dnsec, NanoSeconds, units));
      selfStopped = true;
   }
#endif
}

double PerfLog::StackItem::stopSelf(PerfLog::Units units)
{
   if (!selfStopped)
   {
      selfStopped = true;
      return duration(selfStart, units);
   }
   else
   {
      return 0.0;
   }
}

void PerfLog::StackItem::startSelf()
{
   if (selfStopped)
   {
#ifdef _WIN32
      QueryPerformanceCounter(&selfStart);
#else
      clock_gettime(&selfStart);
#endif
      selfStopped = false;
   }
}

double PerfLog::StackItem::duration(const TimeCounter &from, PerfLog::Units units) const
{
#ifdef _WIN32
   LARGE_INTEGER to, freq;
   
   QueryPerformanceFrequency(&freq);
   
   if (freq.QuadPart == 0)
   {
      return 0.0;
   }
   
   QueryPerformanceCounter(&to);
   
   double dsec = double(to.QuadPart - from.QuadPart) / double(freq.QuadPart);
   
   return PerfLog::ConvertUnits(dsec, Seconds, units);
#else
   struct timespec to;
   
   clock_gettime(&to);
   
   double dsec  = double(to.tv_sec - from.tv_sec);
   double dnsec = double(to.tv_nsec - from.tv_nsec);
   
   return (PerfLog::ConvertUnits(dsec, Seconds, units) + PerfLog::ConvertUnits(dnsec, NanoSeconds, units));
#endif
}

double PerfLog::StackItem::duration(PerfLog::Units units) const
{
   return duration(start, units);
}

// ---

PerfLog::BaseEntry::BaseEntry()
   : totalTime(0)
   , selfTime(0)
   , callCount(0)
{
}

PerfLog::BaseEntry::BaseEntry(const PerfLog::BaseEntry &rhs)
   : totalTime(rhs.totalTime)
   , selfTime(rhs.selfTime)
   , callCount(rhs.callCount)
{
}

PerfLog::BaseEntry& PerfLog::BaseEntry::operator=(const PerfLog::BaseEntry &rhs)
{
   if (this != &rhs)
   {
      totalTime = rhs.totalTime;
      selfTime = rhs.selfTime;
      callCount = rhs.callCount;
   }
   return *this;
}

void PerfLog::BaseEntry::merge(const BaseEntry &rhs)
{
   totalTime += rhs.totalTime;
   selfTime += rhs.selfTime;
   callCount += rhs.callCount;
}

// ---

PerfLog::Entry::Entry()
   : BaseEntry()
{
}

PerfLog::Entry::Entry(const PerfLog::Entry &rhs)
   : BaseEntry(rhs)
   , subs(rhs.subs)
{
}

PerfLog::Entry& PerfLog::Entry::operator=(const PerfLog::Entry &rhs)
{
   BaseEntry::operator=(rhs);
   if (this != &rhs)
   {
      subs = rhs.subs;
   }
   return *this;
}

void PerfLog::Entry::merge(const Entry &rhs)
{
   BaseEntry::merge(rhs);
   
   std::map<std::string, Entry>::iterator it;
   std::map<std::string, Entry>::const_iterator rit;
   
   for (it = subs.begin(); it != subs.end(); ++it)
   {
      rit = rhs.subs.find(it->first);
      if (rit != rhs.subs.end())
      {
         it->second.merge(rit->second);
      }
   }
   
   for (rit = rhs.subs.begin(); rit != rhs.subs.end(); ++rit)
   {
      it = subs.find(rit->first);
      if (it == subs.end())
      {
         subs[rit->first] = rit->second;
      }
   }
}

// ---

const char* PerfLog::UnitsString(PerfLog::Units units)
{
   static const char* sStrs[] = {"nanosecond(s)", "millisecond(s)", "second(s)", "minute(s)", "hour(s)"};
   
   int idx = int(units);
   
   if (idx < 0 || idx > Hours)
   {
      return "(unknown units)";
   }
   else
   {
      return sStrs[idx];
   }
}

double PerfLog::ConvertUnits(double val, PerfLog::Units srcUnits, PerfLog::Units dstUnits)
{
   static const double sConvertUnits[][Hours+1] = {
      {            1.0, 1.0/1000000.0, 1.0/1000000000.0, 1.0/60000000000.0, 1.0/3600000000000.0},
      {      1000000.0,           1.0,       1.0/1000.0,       1.0/60000.0,       1.0/3600000.0},
      {   1000000000.0,        1000.0,              1.0,          1.0/60.0,          1.0/3600.0},
      {  60000000000.0,       60000.0,             60.0,               1.0,            1.0/60.0},
      {3600000000000.0,     3600000.0,           3600.0,              60.0,                 1.0}
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
   
PerfLog& PerfLog::SharedInstance()
{
   static PerfLog sShared;
   return sShared;
}

void PerfLog::Begin(const std::string &msg)
{
   SharedInstance().begin(msg);
}

void PerfLog::End()
{
   SharedInstance().end();
}

void PerfLog::Print(PerfLog::Output output, int flags, int sortBy, PerfLog::Units units)
{
   SharedInstance().print(output, flags, sortBy, units);
}

void PerfLog::Print(std::ostream &os, int flags, int sortBy, PerfLog::Units units)
{
   SharedInstance().print(os, flags, sortBy, units);
}

void PerfLog::Print(Log &log, int flags, int sortBy, PerfLog::Units units)
{
   SharedInstance().print(log, flags, sortBy, units);
}

void PerfLog::Clear()
{
   SharedInstance().clear();
}

// ---

PerfLog::PerfLog(PerfLog::Units units)
   : mUnits(units)
{
}

PerfLog::PerfLog(const PerfLog &rhs)
   : mEntries(rhs.mEntries)
   , mRootEntries(rhs.mRootEntries)
   , mEntryStack(rhs.mEntryStack)
   , mUnits(rhs.mUnits)
{
   if (mUnits == CurrentUnits)
   {
      mUnits = Seconds;
   }
}

PerfLog::~PerfLog()
{
}

PerfLog& PerfLog::operator=(const PerfLog &rhs)
{
   if (this != &rhs)
   {
      mEntries = rhs.mEntries;
      mRootEntries = rhs.mRootEntries;
      mEntryStack = rhs.mEntryStack;
      mUnits = rhs.mUnits;
   }
   return *this;
}

void PerfLog::clear()
{
   mEntries.clear();
   mRootEntries.clear();
   mEntryStack.clear();
}

void PerfLog::begin(const std::string &id)
{
   Entry *entry = 0;
   
   // update global entry
   BaseEntry &gentry = mEntries[id];
   gentry.callCount += 1;
   
   size_t n = mEntryStack.size();
   bool addItem = true;
   
   if (n == 0)
   {
      entry = &(mRootEntries[id]);
   }
   else
   {
      // there a previous log level, accumulate
      StackItem &item = mEntryStack.back();
      
      if (item.id == id)
      {
         // recursive call
         addItem = false;
         entry = item.entry;
         item.recursionCount += 1;
      }
      else
      {
         entry = &(item.entry->subs[id]);
         // Stop function timer temporarily
         double duration = item.stopSelf(mUnits);
         // update global entry function time
         mEntries[item.id].selfTime += duration;
         // update level function time
         item.entry->selfTime += duration;
         
      }
   }
   
   entry->callCount += 1;
   
   if (addItem)
   {
      mEntryStack.push_back(StackItem(id, entry));
   }
}

void PerfLog::end()
{
   size_t n = mEntryStack.size();
   
   if (n > 0)
   {
      StackItem &item = mEntryStack.back();
      
      BaseEntry &gentry = mEntries[item.id];
      Entry *entry = item.entry;
      
      if (--(item.recursionCount) < 0)
      {
         //double duration = item.duration(mUnits);
         //double sduration = item.stopSelf(mUnits);
         double duration, sduration;
         
         item.stopAll(duration, sduration, mUnits);
         
         // accumulate total time
         gentry.totalTime += duration;
         // accumulate current level total time
         entry->totalTime += duration;
         
         // accumulate self time
         gentry.selfTime += sduration;
         // accumulate current level self time
         entry->selfTime += sduration;
         
         if (n > 1)
         {
            // returning to previous level resets its selfStart time
            mEntryStack[n-2].startSelf();
         }
         
         mEntryStack.pop_back();
      }
      else
      {
         // keep timers going on until first recurence call
      }
   }
}

bool PerfLog::empty() const
{
   return (mEntries.size() == 0);
}

void PerfLog::merge(const PerfLog &rhs)
{
   BaseEntryMap::iterator eit;
   BaseEntryMap::const_iterator reit;
   
   for (eit=mEntries.begin(); eit!=mEntries.end(); ++eit)
   {
      reit = rhs.mEntries.find(eit->first);
      if (reit != rhs.mEntries.end())
      {
         eit->second.merge(reit->second);
      }
   }
   
   for (reit=rhs.mEntries.begin(); reit!=rhs.mEntries.end(); ++reit)
   {
      eit = mEntries.find(reit->first);
      if (eit == mEntries.end())
      {
         mEntries[reit->first] = reit->second;
      }
   }
   
   EntryMap::iterator rit;
   EntryMap::const_iterator rrit;
   
   for (rit=mRootEntries.begin(); rit!=mRootEntries.end(); ++rit)
   {
      rrit = rhs.mRootEntries.find(rit->first);
      if (rrit != rhs.mRootEntries.end())
      {
         rit->second.merge(rrit->second);
      }
   }
   
   for (rrit=rhs.mRootEntries.begin(); rrit!=rhs.mRootEntries.end(); ++rrit)
   {
      rit = mRootEntries.find(rrit->first);
      if (rit == mRootEntries.end())
      {
         mRootEntries[rrit->first] = rrit->second;
      }
   }
}

const char* PerfLog::unitsString(PerfLog::Units units) const
{
   if (units == CurrentUnits)
   {
      units = mUnits;
   }
   
   return UnitsString(units);
}

double PerfLog::convertUnits(double val, PerfLog::Units srcUnits, PerfLog::Units dstUnits) const
{
   if (srcUnits == CurrentUnits)
   {
      srcUnits = mUnits;
   }
   
   if (dstUnits == CurrentUnits)
   {
      dstUnits = mUnits;
   }
   
   return ConvertUnits(val, srcUnits, dstUnits);
}

void PerfLog::print(PerfLog::Output output, int flags, int sortBy, PerfLog::Units units)
{
   if (output == ConsoleOutput)
   {
      print(std::cout, flags, sortBy, units);
   }
   else
   {
      print(Log::Shared(), flags, sortBy, units);
   }
}

// ---

class Logger
{
public:
   
   typedef std::vector<std::string> Line;
   
   Logger(int showFlags, int sortFlags, PerfLog::Units srcUnits, PerfLog::Units dstUnits)
      : mNumFields(0)
      , mShowFlags(showFlags)
      , mSortFlags(sortFlags)
      , mSortReverse(false)
      , mSrcUnits(srcUnits)
      , mDstUnits(dstUnits)
   {
      mSortReverse = ((sortFlags & PerfLog::SortReverse) != 0);
      mSortFlags = mSortFlags & ~PerfLog::SortReverse;
      
      switch (mSortFlags)
      {
      case PerfLog::SortTotalTime:
         mShowFlags = mShowFlags | PerfLog::ShowTotalTime;
         break;
      case PerfLog::SortFuncTime:
         mShowFlags = mShowFlags | PerfLog::ShowFuncTime;
         break;
      case PerfLog::SortAvgTotalTime:
         mShowFlags = mShowFlags | PerfLog::ShowAvgTotalTime;
         break;
      case PerfLog::SortAvgFuncTime:
         mShowFlags = mShowFlags | PerfLog::ShowAvgFuncTime;
         break;
      case PerfLog::SortNumCalls:
         mShowFlags = mShowFlags | PerfLog::ShowNumCalls;
         break;
      default:
         break;
      }
      
      mHeader.push_back("Identifier");
      mFieldLengths.push_back(mHeader.back().length());
      
      if (mShowFlags & PerfLog::ShowTotalTime)
      {
         mHeader.push_back("Total");
         mFieldLengths.push_back(mHeader.back().length());
      }
      
      if (mShowFlags & PerfLog::ShowFuncTime)
      {
         mHeader.push_back("Function");
         mFieldLengths.push_back(mHeader.back().length());
      }
      
      if (mShowFlags & PerfLog::ShowNumCalls)
      {
         mHeader.push_back("Calls");
         mFieldLengths.push_back(mHeader.back().length());
      }
      
      if (mShowFlags & PerfLog::ShowAvgTotalTime)
      {
         mHeader.push_back("Total Avg.");
         mFieldLengths.push_back(mHeader.back().length());
      }
      
      if (mShowFlags & PerfLog::ShowAvgFuncTime)
      {
         mHeader.push_back("Function Avg.");
         mFieldLengths.push_back(mHeader.back().length());
      }
   }
   
   void appendLogLine(const std::string &id, const PerfLog::BaseEntry &entry)
   {
      mLines.push_back(Line());
      Line &lline = mLines.back();
      
      size_t field = 0;
      size_t len = 0;
      
      lline.resize(mFieldLengths.size());
      
      lline[field] = id;
      len = id.length();
      if (len > mFieldLengths[field])
      {
         mFieldLengths[field] = len;
      }
      ++field;
      
      if (mShowFlags & PerfLog::ShowTotalTime)
      {
         std::ostringstream oss;
         oss << PerfLog::ConvertUnits(entry.totalTime, mSrcUnits, mDstUnits);
         lline[field] = oss.str();
         len = lline[field].length();
         if (len > mFieldLengths[field])
         {
            mFieldLengths[field] = len;
         }
         ++field;
      }
      
      if (mShowFlags & PerfLog::ShowFuncTime)
      {
         std::ostringstream oss;
         oss << PerfLog::ConvertUnits(entry.selfTime, mSrcUnits, mDstUnits);
         lline[field] = oss.str();
         len = lline[field].length();
         if (len > mFieldLengths[field])
         {
            mFieldLengths[field] = len;
         }
         ++field;
      }
      
      if (mShowFlags & PerfLog::ShowNumCalls)
      {
         std::ostringstream oss;
         oss << entry.callCount;
         lline[field] = oss.str();
         len = lline[field].length();
         if (len > mFieldLengths[field])
         {
            mFieldLengths[field] = len;
         }
         ++field;
      }
      
      if (mShowFlags & PerfLog::ShowAvgTotalTime)
      {
         std::ostringstream oss;
         oss << PerfLog::ConvertUnits(entry.totalTime / entry.callCount, mSrcUnits, mDstUnits);
         lline[field] = oss.str();
         len = lline[field].length();
         if (len > mFieldLengths[field])
         {
            mFieldLengths[field] = len;
         }
         ++field;
      }

      if (mShowFlags & PerfLog::ShowAvgFuncTime)
      {
         std::ostringstream oss;
         oss << PerfLog::ConvertUnits(entry.selfTime / entry.callCount, mSrcUnits, mDstUnits);
         lline[field] = oss.str();
         len = lline[field].length();
         if (len > mFieldLengths[field])
         {
            mFieldLengths[field] = len;
         }
         ++field;
      }
   }
   
   void appendLogLines(const PerfLog::EntryMap &entries, const std::string &indent="")
   {
      if (entries.size() == 0)
      {
         return;
      }
      
      std::string sindent;
      std::vector<std::string> order;
      
      sortEntries(entries, order);
      
      if (indent.length() == 0)
      {
         sindent = "|- ";
      }
      else
      {
         sindent = "   " + indent;
      }
      
      PerfLog::EntryMap::const_iterator it;
      
      for (size_t i=0; i<order.size(); ++i)
      {
         it = entries.find(order[i]);
         
         if (it == entries.end())
         {
            continue;
         }
         
         const PerfLog::Entry &e = it->second;
         
         appendLogLine(indent+order[i], e);
         
         appendLogLines(e.subs, sindent);
      }
   }
   
   void appendLogLines(const PerfLog::BaseEntryMap &entries)
   {
      std::vector<std::string> order;
      PerfLog::BaseEntryMap::const_iterator it;
      
      sortEntries(entries, order);
      
      for (size_t i=0; i<order.size(); ++i)
      {
         it = entries.find(order[i]);
         
         if (it == entries.end())
         {
            continue;
         }
         
         const PerfLog::BaseEntry &e = it->second;
         
         appendLogLine(order[i], e);
      }
   }
   
   void appendSeparator()
   {
      mLines.push_back(Line());
      Line &l = mLines.back();
      l.resize(mFieldLengths.size());
      for (size_t i=0; i<l.size(); ++i)
      {
         l[i] = "";
      }
   }
   
   void print(std::ostream &os)
   {
      printLine(os, mHeader);
      printHorizontalSeparator(os);
      for (size_t i=0; i<mLines.size(); ++i)
      {
         printLine(os, mLines[i]);
      }
   }
   
   void print(Log &log)
   {
      printLine(log, mHeader);
      printHorizontalSeparator(log);
      for (size_t i=0; i<mLines.size(); ++i)
      {
         printLine(log, mLines[i]);
      }
   }
   
   size_t numLines() const
   {
      return mLines.size();
   }
   
   
private:
   
   void printWhitespace(std::ostream &os, size_t n)
   {
      for (size_t i=0; i<n; ++i) os << " ";
   }
   
   void printVerticalSeparator(std::ostream &os)
   {
      os << " | ";
   }
   
   void printHorizontalSeparator(std::ostream &os, bool noNewLine=false)
   {
      if (mFieldLengths.size() == 0)
      {
         return;
      }
      
      size_t len = 0;
      
      for (size_t i=0; i<mFieldLengths.size(); ++i)
      {
         len += mFieldLengths[i];
      }
      
      len += 3 * (mFieldLengths.size() - 1);
      
      for (size_t i=0; i<len; ++i)
      {
         os << "-";
      }
      
      if (!noNewLine)
      {
         os << std::endl;
      }
   }
   
   void printHorizontalSeparator(Log &log)
   {
      std::ostringstream oss;
      
      printHorizontalSeparator(oss, true);
      
      log.printInfo(oss.str().c_str());
   }
   
   void printLine(std::ostream &os, const Line &lline, bool noNewLine=false)
   {
      if (lline.size() != mFieldLengths.size())
      {
         return;
      }
      
      for (size_t i=0; i<lline.size(); ++i)
      {
         const std::string &field = lline[i];
         
         size_t pad = mFieldLengths[i] - field.length();
         
         os << field;
         
         printWhitespace(os, pad);
         
         if (i+1 < lline.size())
         {
            printVerticalSeparator(os);
         }
      }
      
      if (!noNewLine)
      {
         os << std::endl;
      }
   }
   
   void printLine(Log &log, const Line &lline)
   {
      std::ostringstream oss;
      
      printLine(oss, lline, true);
      
      log.printInfo(oss.str().c_str());
   }
   
   template <typename T>
   void sortById(const std::map<std::string, T> &entries, std::vector<std::string> &order)
   {
      order.reserve(entries.size());
      
      typename std::map<std::string, T>::const_iterator it = entries.begin();
      
      while (it != entries.end())
      {
         order.push_back(it->first);
         ++it;
      }
   }

   template <typename T>
   void sortByTotalTime(const std::map<std::string, T> &entries, std::vector<std::string> &order)
   {
      std::vector<double> tt;
      std::vector<double>::iterator lb;
      size_t idx;
      
      order.reserve(entries.size());
      tt.reserve(entries.size());
      
      typename std::map<std::string, T>::const_iterator it = entries.begin();
      
      while (it != entries.end())
      {
         lb = std::lower_bound(tt.begin(), tt.end(), it->second.totalTime);
         idx = lb - tt.begin();
         tt.insert(lb, it->second.totalTime);
         order.insert(order.begin() + idx, it->first);
         ++it;
      }
   }

   template <typename T>
   void sortByAvgTotalTime(const std::map<std::string, T> &entries, std::vector<std::string> &order)
   {
      std::vector<double> tt;
      std::vector<double>::iterator lb;
      double avg;
      size_t idx;
      
      order.reserve(entries.size());
      tt.reserve(entries.size());
      
      typename std::map<std::string, T>::const_iterator it = entries.begin();
      
      while (it != entries.end())
      {
         avg = it->second.totalTime / double(it->second.callCount);
         lb = std::lower_bound(tt.begin(), tt.end(), avg);
         idx = lb - tt.begin();
         tt.insert(lb, avg);
         order.insert(order.begin() + idx, it->first);
         ++it;
      }
   }

   template <typename T>
   void sortByFuncTime(const std::map<std::string, T> &entries, std::vector<std::string> &order)
   {
      std::vector<double> tt;
      std::vector<double>::iterator lb;
      size_t idx;
      
      order.reserve(entries.size());
      tt.reserve(entries.size());
      
      typename std::map<std::string, T>::const_iterator it = entries.begin();
      
      while (it != entries.end())
      {
         lb = std::lower_bound(tt.begin(), tt.end(), it->second.selfTime);
         idx = lb - tt.begin();
         tt.insert(lb, it->second.selfTime);
         order.insert(order.begin() + idx, it->first);
         ++it;
      }
   }

   template <typename T>
   void sortByAvgFuncTime(const std::map<std::string, T> &entries, std::vector<std::string> &order)
   {
      std::vector<double> tt;
      std::vector<double>::iterator lb;
      double avg;
      size_t idx;
      
      order.reserve(entries.size());
      tt.reserve(entries.size());
      
      typename std::map<std::string, T>::const_iterator it = entries.begin();
      
      while (it != entries.end())
      {
         avg = it->second.selfTime / double(it->second.callCount);
         lb = std::lower_bound(tt.begin(), tt.end(), avg);
         idx = lb - tt.begin();
         tt.insert(lb, avg);
         order.insert(order.begin() + idx, it->first);
         ++it;
      }
   }

   template <typename T>
   void sortByNumCalls(const std::map<std::string, T> &entries, std::vector<std::string> &order)
   {
      std::vector<size_t> ct;
      std::vector<size_t>::iterator lb;
      size_t idx;
      
      order.reserve(entries.size());
      ct.reserve(entries.size());
      
      typename std::map<std::string, T>::const_iterator it = entries.begin();
      
      while (it != entries.end())
      {
         lb = std::lower_bound(ct.begin(), ct.end(), it->second.callCount);
         idx = lb - ct.begin();
         ct.insert(lb, it->second.callCount);
         order.insert(order.begin() + idx, it->first);
         ++it;
      }
   }
   
   template <typename T>
   void sortEntries(const std::map<std::string, T> &entries, std::vector<std::string> &order)
   {
      if (mSortFlags == PerfLog::SortTotalTime)
      {
         sortByTotalTime(entries, order);
      }
      else if (mSortFlags == PerfLog::SortFuncTime)
      {
         sortByFuncTime(entries, order);
      }
      else if (mSortFlags == PerfLog::SortNumCalls)
      {
         sortByNumCalls(entries, order);
      }
      else if (mSortFlags == PerfLog::SortAvgTotalTime)
      {
         sortByAvgTotalTime(entries, order);
      }
      else if (mSortFlags == PerfLog::SortAvgFuncTime)
      {
         sortByAvgFuncTime(entries, order);
      }
      else
      {
         sortById(entries, order);
      }
      
      // normal order is opposite (we want by default greater first)
      
      if (!mSortReverse)
      {
         std::reverse(order.begin(), order.end());
      }
   }
   
   
private:
   
   size_t mNumFields;
   int mShowFlags;
   int mSortFlags;
   bool mSortReverse;
   PerfLog::Units mSrcUnits;
   PerfLog::Units mDstUnits;
   std::vector<size_t> mFieldLengths;
   Line mHeader;
   std::vector<Line> mLines;
};

// ---

void PerfLog::print(std::ostream &os, int flags, int sortBy, PerfLog::Units units)
{
   os << "Performances (in " << unitsString(units) << "):" << std::endl;
   
   Logger log(flags, sortBy, mUnits, (units == CurrentUnits ? mUnits : units));
   
   if (flags & ShowFlat)
   {
      // flag type display
      log.appendLogLines(mEntries);
   }
   
   if (flags & ShowDetailed)
   {
      if (log.numLines() > 0)
      {
         log.appendSeparator();
      }
      // graph type display
      log.appendLogLines(mRootEntries);
   }
   
   if (log.numLines() == 0)
   {
      // show flag view by default
      log.appendLogLines(mEntries);
   }
   
   log.print(os);
   
   if (mEntryStack.size() > 0)
   {
      os << "(Warning: Still have " << mEntryStack.size() << " entry(ies) on stack:" << std::endl;
      for (size_t i=0; i<mEntryStack.size(); ++i)
      {
        os << "   " << mEntryStack[mEntryStack.size() - 1 - i].id << std::endl;
      }
      os << ")" << std::endl;
   }
}

void PerfLog::print(Log &log, int flags, int sortBy, PerfLog::Units units)
{
   log.printInfo("Performances (in %s)", unitsString(units));
   log.indent();
   
   Logger logger(flags, sortBy, mUnits, (units == CurrentUnits ? mUnits : units));
   
   if (flags & ShowFlat)
   {
      // flag type display
      logger.appendLogLines(mEntries);
   }
   
   if (flags & ShowDetailed)
   {
      if (logger.numLines() > 0)
      {
         logger.appendSeparator();
      }
      // graph type display
      logger.appendLogLines(mRootEntries);
   }
   
   if (logger.numLines() == 0)
   {
      // show flag view by default
      logger.appendLogLines(mEntries);
   }
   
   logger.print(log);
   
   log.unIndent();
   
   if (mEntryStack.size() > 0)
   {
      log.printWarning("[PerfLog] Still have %lu entry(ies) on stack", mEntryStack.size());
      log.indent();
      for (size_t i=0; i<mEntryStack.size(); ++i)
      {
         log.printWarning(mEntryStack[mEntryStack.size() - 1 - i].id.c_str());
      }
      log.unIndent();
   }
}

// ----

ScopedPerfLog::ScopedPerfLog(const std::string &key)
   : mPerfLog(PerfLog::SharedInstance())
{
   mPerfLog.begin(key);
}

ScopedPerfLog::ScopedPerfLog(PerfLog &plog, const std::string &key)
   : mPerfLog(plog)
{
   mPerfLog.begin(key);
}

ScopedPerfLog::~ScopedPerfLog()
{
   mPerfLog.end();
}

}


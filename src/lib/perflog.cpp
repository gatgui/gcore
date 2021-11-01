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

#include <gcore/perflog.h>
#include <gcore/log.h>

namespace gcore
{

PerfLog::StackItem::StackItem()
   : entry(0)
   , id("")
   , recursionCount(0)
   , selfStopped(false)
{
   selfStart = start;
}

PerfLog::StackItem::StackItem(const String &_id, Entry *_entry)
   : entry(_entry)
   , id(_id)
   , recursionCount(0)
   , selfStopped(false)
{
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

void PerfLog::StackItem::stopAll(double &total, double &self, TimeCounter::Units units)
{
   TimeCounter end(units);
   
   total = (end - start).value();
   if (selfStopped)
   {
      self = 0;
   }
   else
   {
      self = (end - selfStart).value();
      selfStopped = true;
   }
}

double PerfLog::StackItem::stopSelf(TimeCounter::Units units)
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
      selfStart.restart();
      selfStopped = false;
   }
}

double PerfLog::StackItem::duration(const TimeCounter &from, TimeCounter::Units units) const
{
   return from.elapsed().value(units);
}

double PerfLog::StackItem::duration(TimeCounter::Units units) const
{
   return start.elapsed().value(units);
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
   
   std::map<String, Entry>::iterator it;
   std::map<String, Entry>::const_iterator rit;
   
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

PerfLog& PerfLog::Get()
{
   static PerfLog sShared;
   return sShared;
}

void PerfLog::Begin(const String &msg)
{
   Get().begin(msg);
}

void PerfLog::End()
{
   Get().end();
}

void PerfLog::Print(PerfLog::Output output, int flags, int sortBy, TimeCounter::Units units)
{
   Get().print(output, flags, sortBy, units);
}

void PerfLog::Print(std::ostream &os, int flags, int sortBy, TimeCounter::Units units)
{
   Get().print(os, flags, sortBy, units);
}

void PerfLog::Print(Log &log, int flags, int sortBy, TimeCounter::Units units)
{
   Get().print(log, flags, sortBy, units);
}

void PerfLog::Clear()
{
   Get().clear();
}

// ---

PerfLog::PerfLog(TimeCounter::Units units)
   : mUnits(units)
{
}

PerfLog::PerfLog(const PerfLog &rhs)
   : mEntries(rhs.mEntries)
   , mRootEntries(rhs.mRootEntries)
   , mEntryStack(rhs.mEntryStack)
   , mUnits(rhs.mUnits)
{
   if (mUnits == TimeCounter::CurrentUnits)
   {
      mUnits = TimeCounter::Seconds;
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

void PerfLog::begin(const String &id)
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

const char* PerfLog::unitsString(TimeCounter::Units units) const
{
   if (units == TimeCounter::CurrentUnits)
   {
      units = mUnits;
   }
   
   return TimeCounter::UnitsString(units);
}

double PerfLog::convertUnits(double val, TimeCounter::Units srcUnits, TimeCounter::Units dstUnits) const
{
   if (srcUnits == TimeCounter::CurrentUnits)
   {
      srcUnits = mUnits;
   }
   
   if (dstUnits == TimeCounter::CurrentUnits)
   {
      dstUnits = mUnits;
   }
   
   return TimeCounter::ConvertUnits(val, srcUnits, dstUnits);
}

void PerfLog::print(PerfLog::Output output, int flags, int sortBy, TimeCounter::Units units)
{
   if (output == ConsoleOutput)
   {
      print(std::cout, flags, sortBy, units);
   }
   else
   {
      print(Log::Get(), flags, sortBy, units);
   }
}

// ---

class Logger
{
public:
   
   typedef std::vector<String> Line;
   
   Logger(int showFlags, int sortFlags, TimeCounter::Units srcUnits, TimeCounter::Units dstUnits)
      : mShowFlags(showFlags)
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
   
   void appendLogLine(const String &id, const PerfLog::BaseEntry &entry)
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
         oss << TimeCounter::ConvertUnits(entry.totalTime, mSrcUnits, mDstUnits);
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
         oss << TimeCounter::ConvertUnits(entry.selfTime, mSrcUnits, mDstUnits);
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
         oss << TimeCounter::ConvertUnits(entry.totalTime / entry.callCount, mSrcUnits, mDstUnits);
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
         oss << TimeCounter::ConvertUnits(entry.selfTime / entry.callCount, mSrcUnits, mDstUnits);
         lline[field] = oss.str();
         len = lline[field].length();
         if (len > mFieldLengths[field])
         {
            mFieldLengths[field] = len;
         }
         ++field;
      }
   }
   
   void appendLogLines(const PerfLog::EntryMap &entries, const String &indent="")
   {
      if (entries.size() == 0)
      {
         return;
      }
      
      String sindent;
      std::vector<String> order;
      
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
      std::vector<String> order;
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
         const String &field = lline[i];
         
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
   void sortById(const std::map<String, T> &entries, std::vector<String> &order)
   {
      order.reserve(entries.size());
      
      typename std::map<String, T>::const_iterator it = entries.begin();
      
      while (it != entries.end())
      {
         order.push_back(it->first);
         ++it;
      }
   }

   template <typename T>
   void sortByTotalTime(const std::map<String, T> &entries, std::vector<String> &order)
   {
      std::vector<double> tt;
      std::vector<double>::iterator lb;
      size_t idx;
      
      order.reserve(entries.size());
      tt.reserve(entries.size());
      
      typename std::map<String, T>::const_iterator it = entries.begin();
      
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
   void sortByAvgTotalTime(const std::map<String, T> &entries, std::vector<String> &order)
   {
      std::vector<double> tt;
      std::vector<double>::iterator lb;
      double avg;
      size_t idx;
      
      order.reserve(entries.size());
      tt.reserve(entries.size());
      
      typename std::map<String, T>::const_iterator it = entries.begin();
      
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
   void sortByFuncTime(const std::map<String, T> &entries, std::vector<String> &order)
   {
      std::vector<double> tt;
      std::vector<double>::iterator lb;
      size_t idx;
      
      order.reserve(entries.size());
      tt.reserve(entries.size());
      
      typename std::map<String, T>::const_iterator it = entries.begin();
      
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
   void sortByAvgFuncTime(const std::map<String, T> &entries, std::vector<String> &order)
   {
      std::vector<double> tt;
      std::vector<double>::iterator lb;
      double avg;
      size_t idx;
      
      order.reserve(entries.size());
      tt.reserve(entries.size());
      
      typename std::map<String, T>::const_iterator it = entries.begin();
      
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
   void sortByNumCalls(const std::map<String, T> &entries, std::vector<String> &order)
   {
      std::vector<size_t> ct;
      std::vector<size_t>::iterator lb;
      size_t idx;
      
      order.reserve(entries.size());
      ct.reserve(entries.size());
      
      typename std::map<String, T>::const_iterator it = entries.begin();
      
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
   void sortEntries(const std::map<String, T> &entries, std::vector<String> &order)
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
   
   int mShowFlags;
   int mSortFlags;
   bool mSortReverse;
   TimeCounter::Units mSrcUnits;
   TimeCounter::Units mDstUnits;
   std::vector<size_t> mFieldLengths;
   Line mHeader;
   std::vector<Line> mLines;
};

// ---

void PerfLog::print(std::ostream &os, int flags, int sortBy, TimeCounter::Units units)
{
   os << "Performances (in " << unitsString(units) << "):" << std::endl;
   
   Logger log(flags, sortBy, mUnits, (units == TimeCounter::CurrentUnits ? mUnits : units));
   
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

void PerfLog::print(Log &log, int flags, int sortBy, TimeCounter::Units units)
{
   log.printInfo("Performances (in %s)", unitsString(units));
   log.indent();
   
   Logger logger(flags, sortBy, mUnits, (units == TimeCounter::CurrentUnits ? mUnits : units));
   
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

ScopedPerfLog::ScopedPerfLog(const String &key)
   : mPerfLog(PerfLog::Get())
{
   mPerfLog.begin(key);
}

ScopedPerfLog::ScopedPerfLog(PerfLog &plog, const String &key)
   : mPerfLog(plog)
{
   mPerfLog.begin(key);
}

ScopedPerfLog::~ScopedPerfLog()
{
   mPerfLog.end();
}

} // gcore


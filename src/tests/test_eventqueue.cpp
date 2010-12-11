#include <gcore/threads.h>
#include <gcore/threadpool.h>
#include <gcore/functor.h>
#include <deque>
#include <string>
#include <iostream>
#include <cstdarg>

gcore::Mutex gPrintMutex(true);
std::map<gcore::ThreadID, std::string> gThreadNames;

void safePrint(const char *fmt, ...)
{
  va_list args;
  
  gPrintMutex.lock();
  
  fprintf(stdout, "In thread %s - ", gThreadNames[gcore::Thread::CurrentID()].c_str());
  
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end(args);
  
  gPrintMutex.unlock();
}

void safeDebugPrint(const char *fmt, ...)
{
#ifdef _DEBUG
  va_list args;
  
  gPrintMutex.lock();
  
  fprintf(stdout, "In thread %s - ", gThreadNames[gcore::Thread::CurrentID()].c_str());
  
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end(args);
  
  gPrintMutex.unlock();
#endif
}

namespace gcore
{
  class EventQueue
  {
  public:
    
    struct Event
    {
      Functor0 func;
      bool sync;
      size_t id;
    };
    
    typedef std::deque<Event> EventList;
    
  public:
    
    EventQueue()
      : mOwner(Thread::CurrentID()), mAcceptEvents(true), mDoneID((size_t)-1), mCurID(0)
    {
      safePrint("Is event queue owner thread");
    }
    
    virtual ~EventQueue()
    {
      // will this be called by the right thread?
      acceptEvents(false);
      // shall we execute remaining events if we are in the right thread
      mEvents.clear();
    }
    
    bool push(Functor0 func, bool sync=false)
    {
      if (Thread::CurrentID() != mOwner)
      {
        mEventsMutex.lock();
        
        if (!mAcceptEvents)
        {
          mEventsMutex.unlock();
          return false;
        }
        
        size_t id = newID();
        
        Event evt = {func, sync, id};
        
        mEvents.push_back(evt);
        
        mEventsMutex.unlock();
        
        safeDebugPrint("Pushed event %d\n", id);
        
        if (sync)
        {
          mDoneMutex.lock();
          while (mDoneID != id)
          {
            safeDebugPrint("Not done yet\n");
            mDoneCond.wait(mDoneMutex);
            safeDebugPrint("Awoken, check if event %d is done\n", id);
          }
          safeDebugPrint("Event has been processed\n");
          mDoneID = (size_t)-1;
          mDoneMutex.unlock();
        }
        
        return true;
      }
      else
      {
        // execute straight away
        func();
        return true;
      }
    }
    
    size_t poll(size_t count)
    {
      if (Thread::CurrentID() != mOwner)
      {
        safeDebugPrint("Cannot poll\n");
        return 0;
      }
      
      mEventsMutex.lock();
      
      if (count > mEvents.size())
      {
        count = mEvents.size();
      }
      
      if (count == 0)
      {
        mEventsMutex.unlock();
        return 0;
      }
      
      EventList::iterator first = mEvents.begin();
      EventList::iterator last = first + count;
      
      EventList tmp(first, last);
      
      mEvents.erase(first, last);
      
      mEventsMutex.unlock();
      
      for (size_t i=0; i<tmp.size(); ++i)
      {
        Event &evt = tmp[i];
        
        safeDebugPrint("Execute event %d\n", evt.id);
        
        evt.func();
        
        // some events notification get skipped
        // we don't know when the thread waiting for this event
        // will be awoken, maybe another one will be awoken before
        // that will push a sync event again, and poll my even be called
        // before the waiting thread is awoken
        // -> the event is skipped and thread1 waits forever
        // -> we have to wait here that mDoneID return to -1
        
        if (evt.sync)
        {
          safeDebugPrint("Notify waiting threads event %d is done\n", evt.id);
          mDoneMutex.lock();
          mDoneID = evt.id;
          // Note: mDoneCond.notifyAll() won't do the job
          //       we have to be sure the waiting thread got the message
          while (mDoneID != (size_t)-1)
          {
            ScopeUnlock su(mDoneMutex);
            mDoneCond.notify();
          }
          mDoneMutex.unlock();
        }
      }
      
      return count;
    }
    
    void acceptEvents(bool v)
    {
      if (Thread::CurrentID() == mOwner)
      {
        mEventsMutex.lock();
        mAcceptEvents = v;
        mEventsMutex.unlock();
      }
    }
    
  private:
    
    EventQueue(const EventQueue &);
    EventQueue& operator=(const EventQueue &);
    
    size_t newID()
    {
      // this will eventually overflow, getting back to 0
      // by the time it overflow, hopefully the event with id 0
      // will have be processed
      // ideally we should not assign an id leaving in the queue
      
      /*
      size_t id = mCurID++;
      bool inqueue = true;
      while (inqueue)
      {
        size_t i = 0;
        for (i=0; i<mEvents.size(); ++i)
        {
          if (id == mEvents[i].id)
          {
            ++id;
            break;
          }
        }
        if (i == mEvents.size())
        {
          inqueue = false;
        }
      }
      return id;
      */
      
      return mCurID++;
    }
    
  protected:
    
    ThreadID mOwner;
    EventList mEvents; // protected by mEventsMutex
    bool mAcceptEvents; // also protected by mEventsMutex
    size_t mDoneID; // protected by mDoneMutex
    size_t mCurID;
    
    Mutex mEventsMutex;
    Mutex mDoneMutex;
    Condition mDoneCond;
  };
}

using namespace gcore;

EventQueue *gMainThreadQueue = 0;

int gThread1CurLoop = 0;
int gThread2CurLoop = 0;
int gThread3CurLoop = 0;
int gThread4CurLoop = 0;
bool gThread1Done = false;
bool gThread2Done = false;
bool gThread3Done = false;
bool gThread4Done = false;
const int gLoopCount = 10000;

void thread1SyncEvent()
{
  safePrint("(%d) Synchronized event for thread1\n", gThread1CurLoop);
}

void thread2SyncEvent()
{
  safePrint("(%d) Synchronized event for thread2\n", gThread2CurLoop);
}

void thread3SyncEvent()
{
  safePrint("(%d) Synchronized event for thread3\n", gThread3CurLoop);
}

void thread4SyncEvent()
{
  safePrint("(%d) Synchronized event for thread4\n", gThread4CurLoop);
}

void thread1ASyncEvent()
{
  safePrint("(%d) Asynchronized event for thread1\n", gThread1CurLoop);
}

void thread2ASyncEvent()
{
  safePrint("(%d) Asynchronized event for thread2\n", gThread2CurLoop);
}

void thread3ASyncEvent()
{
  safePrint("(%d) Asynchronized event for thread3\n", gThread3CurLoop);
}

void thread4ASyncEvent()
{
  safePrint("(%d) Asynchronized event for thread4\n", gThread4CurLoop);
}

void thread1Proc()
{
  gThreadNames[Thread::CurrentID()] = "thread1";
  
  Functor0 func;
  
  for (gThread1CurLoop=0; gThread1CurLoop<gLoopCount; ++gThread1CurLoop)
  {
    safeDebugPrint("Push synchronized event and wait for result...\n");
    
    Bind(thread1SyncEvent, func);
    gMainThreadQueue->push(func, true);
  }
  
  gThread1Done = true;
}

void thread2Proc()
{
  gThreadNames[Thread::CurrentID()] = "thread2";
  
  Functor0 func;
  
  for (gThread2CurLoop=0; gThread2CurLoop<gLoopCount; ++gThread2CurLoop)
  {
    safeDebugPrint("Push synchronized event and wait for result...\n");
    
    Bind(thread2SyncEvent, func);
    gMainThreadQueue->push(func, true);
    
    
    safeDebugPrint("Push asynchronized event\n");
    
    Bind(thread2ASyncEvent, func);
    gMainThreadQueue->push(func, false);
  }
  
  gThread2Done = true;
}

void thread3Proc()
{
  gThreadNames[Thread::CurrentID()] = "thread3";
  
  Functor0 func;
  
  for (gThread3CurLoop=0; gThread3CurLoop<gLoopCount; ++gThread3CurLoop)
  {
    safeDebugPrint("Push asynchronized event\n");
    
    Bind(thread3ASyncEvent, func);
    gMainThreadQueue->push(func, false);
  }
  
  gThread3Done = true;
}

void thread4Proc()
{
  gThreadNames[Thread::CurrentID()] = "thread4";
  
  Functor0 func;
  
  for (gThread4CurLoop=0; gThread4CurLoop<gLoopCount; ++gThread4CurLoop)
  {
    safeDebugPrint("Push asynchronized event\n");
    
    Bind(thread4ASyncEvent, func);
    gMainThreadQueue->push(func, false);
    
    safeDebugPrint("Push synchronized event and wait for result...\n");
    
    Bind(thread4SyncEvent, func);
    gMainThreadQueue->push(func, true);
  }
  
  gThread4Done = true;
}

// If two threads send synchronized event, it seems the first one to be awake works fine
// But the second one never awakes...

int main(int, char**)
{
  gThreadNames[Thread::CurrentID()] = "main";
  
  EventQueue mainThreadQueue;
  gMainThreadQueue = &mainThreadQueue;
  
  ThreadPool tpool;
  
#if 0
  
  tpool.start(2);
  
  Task thread1, thread2;
  
  Bind(thread1Proc, thread1);
  Bind(thread2Proc, thread2);
  
  tpool.runTask(thread1);
  tpool.runTask(thread2);
  
  safePrint("Enter event loop\n");
  
  while (!(gThread1Done && gThread2Done))
  {
    mainThreadQueue.poll(1);
  };
  
#else
  
  tpool.start(4);
  
  Task thread1, thread2, thread3, thread4;
  
  Bind(thread1Proc, thread1);
  Bind(thread2Proc, thread2);
  Bind(thread3Proc, thread3);
  Bind(thread4Proc, thread4);
  
  tpool.runTask(thread1);
  tpool.runTask(thread2);
  tpool.runTask(thread3);
  tpool.runTask(thread4);
  
  safePrint("Enter event loop\n");
  
  while (!(gThread1Done && gThread2Done && gThread3Done && gThread4Done))
  {
    mainThreadQueue.poll(1);
  };

#endif

  safePrint("Finished\n");
  
  return 0;
}


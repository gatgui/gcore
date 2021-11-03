/*
MIT License

Copyright (c) 2009 Gaetan Guidet

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

#ifndef __gcore_threads_h_
#define __gcore_threads_h_

#include <gcore/functor.h>

namespace gcore {

  class GCORE_API Mutex {
    public:
      
      friend class Condition;
      
      Mutex(bool recursive=false);
      virtual ~Mutex();
      
      void lock();
      bool tryLock();
      void unlock();
      bool isLocked();
      
    private:
      
      Mutex(const Mutex &){}
      Mutex& operator=(const Mutex &){return *this;}
      
    private:
      
      unsigned char mData[64];

      //sizeof(CRITICAL_SECTION) == 24
      //sizeof(pthread_mutex_t)  == 44 (OSX)
      //round up to closest power of 2
  };
  
  // another kind of lock in pthread: rwlock !
  class GCORE_API RWLock {
    public:
      
      RWLock();
      virtual ~RWLock();
      
      void readLock();
      bool tryReadLock();
      void readUnlock();
      
      void writeLock();
      bool tryWriteLock();
      void writeUnlock();
      
    private:
    
      RWLock(const RWLock&){}
      RWLock& operator=(const RWLock&){return *this;}
      
    private:
      
      unsigned char mData[64];
  };
  
  class GCORE_API Condition {
    public:
      
      Condition();
      virtual ~Condition();
      
      // mtx must be locked before call to wait or tryWait

      void wait(Mutex &mtx);
      bool timedWait(Mutex &mtx, unsigned long msec);
      void notify();
      void notifyAll();
      
    private:
      
      Condition(const Condition &);
      Condition& operator=(const Condition &){return *this;}
      
    private:
      
      unsigned char  mData[64];

      //sizeof(pthread_cond_t) == 28 (OSX)
      //2*sizeof(HANDLE) + sizeof(CRITICAL_SECTION) + sizeof(ulong) == 48
      //    [with 64 bits HANDLE]
      //round up to closest power of 2
  };
  
  // a semaphore is in some way a mutex that you can be locked several times
  // [dec will block when its count reach 0, until it get greater than 0]
  class GCORE_API Semaphore {
    public:
      
      Semaphore(long init, long maxval=0x7FFFFFF);
      virtual ~Semaphore();
      
      void increment(long count=1); //release
      void decrement();             //lock (blocking)
      bool tryDecrement();          //try lock
      bool timedDecrement(unsigned long ms);
      
    private:
      
      Semaphore(const Semaphore &){}
      Semaphore& operator=(const Semaphore &){return *this;}
      
      //long mMax;
      unsigned char mData[128];
  };


  class GCORE_API ScopeLock {
    public:
      inline ScopeLock(Mutex &mtx)
        : mMutex(mtx) {
        mMutex.lock();
      }
      inline ~ScopeLock() {
        mMutex.unlock();
      }
    private:
      ScopeLock();
      ScopeLock(const ScopeLock&);
      ScopeLock& operator=(const ScopeLock&); 
    private:
      Mutex &mMutex;
  };


  class GCORE_API ScopeUnlock {
    public:
      inline ScopeUnlock(Mutex &mtx)
        : mMutex(mtx) {
        mMutex.unlock();
      }
      inline ~ScopeUnlock() {
        mMutex.lock();
      }
    private:
      ScopeUnlock();
      ScopeUnlock(const ScopeUnlock&);
      ScopeUnlock& operator=(const ScopeUnlock&); 
    private:
      Mutex &mMutex;
  };

  typedef void* ThreadID;

  class GCORE_API Thread {
    
    public:
    
      enum Priority {
        PRI_UNKNOWN = -1,
        PRI_DEFAULT,
        PRI_VERY_LOW,
        PRI_LOW,
        PRI_NORMAL,
        PRI_HIGH,
        PRI_VERY_HIGH
      };

      enum Scheduling {
        SCH_UNKNOWN = -1,
        SCH_DEFAULT,
        SCH_FIFO,
        SCH_RR
      };
      
      typedef Functor0wR<int> Procedure;
      typedef Functor1<int> EndCallback;
      
      Thread();
      
      Thread(Procedure proc, EndCallback ended=EndCallback(), bool waitStart=false);
      
      template <typename R>
      Thread(R (*run)(), void (*done)(int)=0, bool waitStart=false)
        : mRunning(false), mStarted(false) {
        Bind(run, mProc);
        if (done) {
          Bind(done, mEndCB);
        }
        restart(waitStart);
      }
      
      template <typename R, typename T>
      Thread(R (*run)(), T *obj, void (T::*done)(int), bool waitStart=false)
        : mRunning(false), mStarted(false) {
        Bind(run, mProc);
        Bind(obj, done, mEndCB);  
        restart(waitStart);
      }
      
      template <typename T, typename R>
      Thread(T *obj, R (T::*run)(), bool waitStart=false)
        : mRunning(false), mStarted(false) {
        Bind(obj, run, mProc);
        restart(waitStart);  
      }
      
      template <typename T, typename R>
      Thread(T *obj, R (T::*run)(), void (T::*done)(int), bool waitStart=false)
        : mRunning(false), mStarted(false) {
        Bind(obj, run, mProc);
        Bind(obj, done, mEndCB);
        restart(waitStart);  
      }
      
      template <typename T, typename R, typename U>
      Thread(T *obj0, R (T::*run)(), U *obj1, void (U::*done)(int), bool waitStart=false)
        : mRunning(false), mStarted(false) {
        Bind(obj0, run, mProc);
        Bind(obj1, done, mEndCB);  
        restart(waitStart);
      }
      
      template <typename T, typename R, typename U>
      Thread(T *obj0, R (T::*run)(), void (*done)(int), bool waitStart=false)
        : mRunning(false), mStarted(false) {
        Bind(obj0, run, mProc);
        Bind(done, mEndCB);  
        restart(waitStart);
      }
      
      virtual ~Thread();
      
      // run the same thread again
      bool restart(bool waitStart=false);
      // is this thread running
      bool running() const;
      // detach this thread [no join needed to free thread ressources, no effect on windows]
      bool detach();
      // wait for thread to be done and detach
      bool join(int *retval=0);
      // try cancel this thread [need cancel points in thread to work out]
      bool cancel();
      // access this thread priority
      Priority priority() const;
      bool priority(Priority prio);
      // scheduling
      Scheduling scheduling() const;
      bool scheduling(Scheduling s);
      // this thread id
      ThreadID id() const;
      // suspend curreny thread
      bool suspend();
      // resume suspended thread
      bool resume();

      // sleep in current trhead
      static void SleepCurrent(unsigned long msec);
      // yield current thread time quantum
      static void YieldCurrent();
      // get current thread id
      static ThreadID CurrentID();
      // get number of processor on machine
      static int GetProcessorCount();
      
    private:
      
      bool detachable() const;
      bool suspendable() const;
      bool cancelable() const;
      bool joinable() const;
      void notifyStarted();
      void waitStarted();
      
#ifdef _WIN32
      static unsigned int __stdcall _ThreadEntryFunc(void *data);
      volatile ThreadID mSelfId;
#else
      static void* _ThreadEntryFunc(void *data);
#endif
      
      Thread(const Thread &);
      Thread& operator=(const Thread &);
      
      volatile ThreadID mSelf;
      volatile bool mRunning;
      Procedure mProc;
      EndCallback mEndCB;
      Condition mStartedCond;
      Mutex mMutex;
      bool mStarted;
  };
}


#endif




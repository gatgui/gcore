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

#include <gcore/threads.h>
#include <gcore/platform.h>


#ifdef _WIN32

#ifdef WIN32_REAL_MUTEX

#define MUTEX *((HANDLE*)&(mData[0]))

gcore::Mutex::Mutex(bool) {
  MUTEX = CreateMutex(NULL, FALSE, NULL);
}

gcore::Mutex::~Mutex() {
  CloseHandle(MUTEX);
}

void gcore::Mutex::lock() {
  WaitForSingleObject(MUTEX, INFINITE);
}

bool gcore::Mutex::tryLock() {
  return (WaitForSingleObject(MUTEX, 0) == WAIT_OBJECT_0);
}

void gcore::Mutex::unlock() {
  ReleaseMutex(MUTEX);
}

#else

gcore::Mutex::Mutex(bool) {
  InitializeCriticalSection((CRITICAL_SECTION*)&mData);
}

gcore::Mutex::~Mutex() {
  DeleteCriticalSection((CRITICAL_SECTION*)&mData);
}

void gcore::Mutex::lock() {
  EnterCriticalSection((CRITICAL_SECTION*)&mData);
}

bool gcore::Mutex::tryLock() {
#if (_WIN32_WINNT >= 0x0400) //need to be define manually
  return (TryEnterCriticalSection((CRITICAL_SECTION*)&mData) != 0);
#else
  return false;
#endif
}

void gcore::Mutex::unlock() {
  LeaveCriticalSection((CRITICAL_SECTION*)&mData);
}

#endif

// ---

namespace details {
  struct RWL_WIN32 {
    CRITICAL_SECTION mutex;
    CRITICAL_SECTION access;
    unsigned long numReaders;
  };
}

gcore::RWLock::RWLock() {
  assert(sizeof(mData)>=sizeof(details::RWL_WIN32));
  details::RWL_WIN32 *rwl = (details::RWL_WIN32*)mData;
  rwl->numReaders = 0;
  InitializeCriticalSection(&(rwl->mutex));
  InitializeCriticalSection(&(rwl->access));
}

gcore::RWLock::~RWLock() {
  details::RWL_WIN32 *rwl = (details::RWL_WIN32*)mData;
  DeleteCriticalSection(&(rwl->mutex));
  DeleteCriticalSection(&(rwl->access));
}

void gcore::RWLock::readLock() {
  details::RWL_WIN32 *rwl = (details::RWL_WIN32*)mData;
  EnterCriticalSection(&(rwl->mutex));
  if (++rwl->numReaders == 1) {
    // only first reader acquire the lock
    EnterCriticalSection(&(rwl->access));
  }
  LeaveCriticalSection(&(rwl->mutex));
}

bool gcore::RWLock::tryReadLock() {
#if (_WIN32_WINNT >= 0x0400) //need to be define manually
  details::RWL_WIN32 *rwl = (details::RWL_WIN32*)mData;
  if (TryEnterCriticalSection(&(rwl->mutex))) {
    if (++rwl->numReaders==1 && !TryEnterCriticalSection(&(rwl->access))) {
      --rwl->numReaders;
      LeaveCriticalSection(&(rwl->mutex));
      return false;
    }
    LeaveCriticalSection(&(rwl->mutex));
    return true;
  }
#endif
  return false;
}

void gcore::RWLock::readUnlock() {
  details::RWL_WIN32 *rwl = (details::RWL_WIN32*)mData;
  EnterCriticalSection(&(rwl->mutex));
  if (--rwl->numReaders==0) {
    // only last reader release the lock
    LeaveCriticalSection(&(rwl->access));
  }
  LeaveCriticalSection(&(rwl->mutex));
}

void gcore::RWLock::writeLock() {
  details::RWL_WIN32 *rwl = (details::RWL_WIN32*)mData;
  EnterCriticalSection(&(rwl->access));
}

bool gcore::RWLock::tryWriteLock() {
  details::RWL_WIN32 *rwl = (details::RWL_WIN32*)mData;
#if (_WIN32_WINNT >= 0x0400)
  return (TryEnterCriticalSection(&(rwl->access)) != 0);
#else
  return false;
#endif
}

void gcore::RWLock::writeUnlock() {
  details::RWL_WIN32 *rwl = (details::RWL_WIN32*)mData;
  LeaveCriticalSection(&(rwl->access));
}

// ---

#ifdef WIN32_REAL_MUTEX

namespace gcore {
  namespace details {
    struct ConditionData {
      HANDLE sema;
      HANDLE waiterDone;
      unsigned long waiterCount;
      CRITICAL_SECTION waiterCountLock;
      bool wasBroadcast;  
    }; 
  }
}

gcore::Condition::Condition() {
  details::ConditionData *cond = (details::ConditionData*)&(mData[0]);
  
  InitializeCriticalSection(&cond->waiterCountLock);
  cond->sema = CreateSemaphore(NULL, 0, 0x7FFFFFF, NULL);
  cond->waiterDone = CreateEvent(NULL, FALSE, FALSE, NULL);
  cond->waiterCount = 0;
  cond->wasBroadcast = false;
}

gcore::Condition::~Condition() {
  details::ConditionData *cond = (details::ConditionData*)&(mData[0]);
  
  CloseHandle(cond->waiterDone);
  CloseHandle(cond->sema);
  DeleteCriticalSection(&cond->waiterCountLock);  
}

void gcore::Condition::wait(gcore::Mutex &mtx) {
  details::ConditionData *cond = (details::ConditionData*)&(mData[0]);
  
  EnterCriticalSection(&cond->waiterCountLock);
  cond->waiterCount++;
  LeaveCriticalSection(&cond->waiterCountLock);

  SignalObjectAndWait(*((HANDLE*)&(mtx.mData[0])), cond->sema, INFINITE, FALSE)
  
  EnterCriticalSection(&cond->waiterCountLock);
  cond->waiterCount--;
  bool lastWaiter = (cond->wasBroadcast && cond->waiterCount==0);
  LeaveCriticalSection(&cond->waiterCountLock);
  
  if (lastWaiter) {
    SignalObjectAndWait(cond->waiterDone, *((HANDLE*)&(mtx.mData[0])), INFINITE, FALSE);
  } else {
    WaitForSingleObject(*((HANDLE*)&(mtx.mData[0])), INFINITE);
  }
}

bool gcore::Condition::timedWait(gcore::Mutex &mtx, unsigned long ms) {
  details::ConditionData *cond = (details::ConditionData*)&(mData[0]);
  
  EnterCriticalSection(&cond->waiterCountLock);
  cond->waiterCount++;
  LeaveCriticalSection(&cond->waiterCountLock);

  bool timedout =
    SignalObjectAndWait(*((HANDLE*)&(mtx.mData[0])), cond->sema, ms, FALSE) == WAIT_OBJET_0;
  
  EnterCriticalSection(&cond->waiterCountLock);
  cond->waiterCount--;
  bool lastWaiter = (!timedout && cond->wasBroadcast && cond->waiterCount==0);
  LeaveCriticalSection(&cond->waiterCountLock);
  
  if (lastWaiter) {
    SignalObjectAndWait(cond->waiterDone, *((HANDLE*)&(mtx.mData[0])), INFINITE, FALSE);
  } else {
    WaitForSingleObject(*((HANDLE*)&(mtx.mData[0])), INFINITE);
  }
  
  return !timedout;
}

void gcore::Condition::notify() {
  details::ConditionData *cond = (details::ConditionData*)&(mData[0]);
  
  EnterCriticalSection(&cond->waiterCountLock);
  bool haveWaiter = (cond->waiterCount > 0);
  LeaveCriticalSection(&cond->waiterCountLock);
  
  if (haveWaiter) {
    ReleaseSemaphore(cond->sema, 1, 0);
  }
}

void gcore::Condition::notifyAll() {
  details::ConditionData *cond = (details::ConditionData*)&(mData[0]);
  
  EnterCriticalSection(&cond->waiterCountLock);
  bool haveWaiter = false;
  if (cond->waiterCount > 0) {
    cond->wasBroadcast = true;
    haveWaiter = true;
  }
  if (haveWaiter) {
    ReleaseSemaphore(cond->sema, cond->waiterCount, 0);
    LeaveCriticalSection(&cond->waiterCountLock);
    WaitForSingleObject(cond->waiterDone, INFINITE);
    cond->wasBroadcast = false;
  } else {
    LeaveCriticalSection(&cond->waiterCountLock);
  }
}

#else

namespace gcore {
  namespace details {
    struct COND_WIN32 {
      HANDLE notifyOne;
      HANDLE notifyAll;
      unsigned long blocked;
      CRITICAL_SECTION blockedLock;
    };
  }
}

gcore::Condition::Condition() {
  details::COND_WIN32 *cond = (details::COND_WIN32*)(&mData[0]);
  cond->notifyOne = CreateEvent(NULL,FALSE,FALSE,NULL); //auto reset
  cond->notifyAll = CreateEvent(NULL,TRUE,FALSE,NULL); //manual reset
  cond->blocked = 0;
  InitializeCriticalSection(&(cond->blockedLock));
}

gcore::Condition::~Condition() {
  details::COND_WIN32 *cond = (details::COND_WIN32*)(&mData[0]);
  CloseHandle(cond->notifyOne);
  CloseHandle(cond->notifyAll);
  DeleteCriticalSection(&(cond->blockedLock));
}

void gcore::Condition::wait(gcore::Mutex &mtx) {
  details::COND_WIN32 *cond = (details::COND_WIN32*)(&mData[0]);
  
  EnterCriticalSection(&(cond->blockedLock));
  cond->blocked += 1;
  LeaveCriticalSection(&(cond->blockedLock)); 

  mtx.unlock();

  DWORD ret = WaitForMultipleObjects(2, &(cond->notifyOne), FALSE, INFINITE);
  
  EnterCriticalSection(&(cond->blockedLock));
  cond->blocked -= 1;
  if ((ret == WAIT_OBJECT_0+1) && (cond->blocked == 0)) {
    ResetEvent(cond->notifyAll);
  }
  LeaveCriticalSection(&(cond->blockedLock));
  
  mtx.lock();
}

bool gcore::Condition::timedWait(gcore::Mutex &mtx, unsigned long msec) {
  details::COND_WIN32 *cond = (details::COND_WIN32*)(&mData[0]);
  
  EnterCriticalSection(&(cond->blockedLock));
  cond->blocked += 1;
  LeaveCriticalSection(&(cond->blockedLock));

  mtx.unlock();
    
  DWORD ret = WaitForMultipleObjects(2, &(cond->notifyOne), FALSE, msec);
  
  EnterCriticalSection(&(cond->blockedLock));
  cond->blocked -= 1;
  if ((ret == WAIT_OBJECT_0+1) && (cond->blocked == 0)) {
    ResetEvent(cond->notifyAll);
  }
  LeaveCriticalSection(&(cond->blockedLock));
  
  mtx.lock();
  
  return (ret != WAIT_TIMEOUT);
}

void gcore::Condition::notify() {
  details::COND_WIN32 *cond = (details::COND_WIN32*)(&mData[0]);
  
  EnterCriticalSection(&(cond->blockedLock));
  bool blocked = (cond->blocked > 0);
  LeaveCriticalSection(&(cond->blockedLock));
  
  if (blocked) {
    SetEvent(cond->notifyOne);
  }
}

void gcore::Condition::notifyAll() {
  details::COND_WIN32 *cond = (details::COND_WIN32*)(&mData[0]);
  
  EnterCriticalSection(&(cond->blockedLock));
  bool blocked = (cond->blocked > 0);
  LeaveCriticalSection(&(cond->blockedLock));
  
  if (blocked) {
    SetEvent(cond->notifyAll);
  }
}

#endif

// ---

gcore::Semaphore::Semaphore(long init, long maxval) {
  *((HANDLE*)mData) = CreateSemaphore(NULL, init, maxval, NULL);
}

gcore::Semaphore::~Semaphore() {
  CloseHandle((HANDLE)mData);
}

void gcore::Semaphore::increment(long count) {
  ReleaseSemaphore((HANDLE)mData, count, NULL);
}

void gcore::Semaphore::decrement() {
  WaitForSingleObject((HANDLE)mData,INFINITE);
}

bool gcore::Semaphore::timedDecrement(unsigned long ms) {
  return (WaitForSingleObject((HANDLE)mData, ms) == WAIT_OBJECT_0);
}

bool gcore::Semaphore::tryDecrement() {
  return (WaitForSingleObject((HANDLE)mData, 0) == WAIT_OBJECT_0);
}

#else // _WIN32

gcore::Mutex::Mutex(bool recursive) {
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, (recursive?PTHREAD_MUTEX_RECURSIVE:PTHREAD_MUTEX_DEFAULT));
  pthread_mutex_init((pthread_mutex_t*)mData, &attr);
  pthread_mutexattr_destroy(&attr);
}

gcore::Mutex::~Mutex() {
  pthread_mutex_destroy((pthread_mutex_t*)mData);
}

void gcore::Mutex::lock() {
  pthread_mutex_lock((pthread_mutex_t*)mData);
}

bool gcore::Mutex::tryLock() {
  return (pthread_mutex_trylock((pthread_mutex_t*)mData) == 0);
}

void gcore::Mutex::unlock() {
  pthread_mutex_unlock((pthread_mutex_t*)mData);
}

bool gcore::Mutex::isLocked() {
  if (tryLock()) {
    unlock();
    return false;
  }
  return true;
}

// ---

gcore::RWLock::RWLock() {
  pthread_rwlock_init((pthread_rwlock_t*)mData, NULL);
}

gcore::RWLock::~RWLock() {
  pthread_rwlock_destroy((pthread_rwlock_t*)mData);
}

void gcore::RWLock::readLock() {
  pthread_rwlock_rdlock((pthread_rwlock_t*)mData);
}

bool gcore::RWLock::tryReadLock() {
  return (pthread_rwlock_tryrdlock((pthread_rwlock_t*)mData) == 0);
}

void gcore::RWLock::readUnlock() {
  pthread_rwlock_unlock((pthread_rwlock_t*)mData);
}

void gcore::RWLock::writeLock() {
  pthread_rwlock_wrlock((pthread_rwlock_t*)mData);
}

bool gcore::RWLock::tryWriteLock() {
  return (pthread_rwlock_trywrlock((pthread_rwlock_t*)mData) == 0);
}

void gcore::RWLock::writeUnlock() {
  pthread_rwlock_unlock((pthread_rwlock_t*)mData);
}

// ---

gcore::Condition::Condition() {
  pthread_cond_init((pthread_cond_t*)mData, NULL);
}

gcore::Condition::~Condition() {
  pthread_cond_destroy((pthread_cond_t*)mData);
}

void gcore::Condition::wait(gcore::Mutex &mtx) {
  pthread_cond_wait((pthread_cond_t*)mData, (pthread_mutex_t*)mtx.mData);
}

bool gcore::Condition::timedWait(gcore::Mutex &mtx, unsigned long msec) {
  register int rval;
  
  struct timespec ts;
  struct timeval tv;
  
  gettimeofday(&tv,0);
  
  ts.tv_nsec  = (tv.tv_usec * 1000) + (msec * 1000000);
  ts.tv_sec   = ts.tv_nsec / 1000000000;
  ts.tv_nsec -= ts.tv_sec * 1000000000;
  ts.tv_sec  += tv.tv_sec;
  
  rval = EINTR;
  while (rval == EINTR) {
    rval = pthread_cond_timedwait((pthread_cond_t*)mData, (pthread_mutex_t*)mtx.mData, &ts);
  }
  
  return (rval != ETIMEDOUT);
}

void gcore::Condition::notify() {
  pthread_cond_signal((pthread_cond_t*)mData);
}

void gcore::Condition::notifyAll() {
  pthread_cond_broadcast((pthread_cond_t*)mData);
}

// ---

namespace details {
  struct sema_t {
    long count;
    long waiterCount;
    long maxCount;
    pthread_mutex_t lock;
    pthread_cond_t nonZero;
  };
}

gcore::Semaphore::Semaphore(long init, long maxval) {
  details::sema_t *s = (details::sema_t*)(&mData[0]);
  pthread_mutex_init(&s->lock, NULL);
  pthread_cond_init(&s->nonZero, NULL);
  s->count = init;
  s->maxCount = maxval;
}

gcore::Semaphore::~Semaphore() {
  details::sema_t *s = (details::sema_t*)(&mData[0]);
  pthread_mutex_destroy(&s->lock);
  pthread_cond_destroy(&s->nonZero);
}

void gcore::Semaphore::increment(long count) {
  register long i, j;
  
  details::sema_t *s = (details::sema_t*)(&mData[0]);
  
  if (count <= 0) {
    return;
  }
  
  pthread_mutex_lock(&s->lock);
  
  // limit count so we don't go over maxCount
  if (s->count + count > s->maxCount) {
    count = s->maxCount - s->count;
  }
  
  // notify waiter if any. should not notify more waiters than count
  j = (s->waiterCount < count ? s->waiterCount : count);
  for (i=0; i<j; ++i) {
    pthread_cond_signal(&s->nonZero);
  }
  
  // increment semaphore
  s->count += count;
  pthread_mutex_unlock(&s->lock);
}

void gcore::Semaphore::decrement() {
  details::sema_t *s = (details::sema_t*)(&mData[0]);
  
  pthread_mutex_lock(&s->lock);
  s->waiterCount += 1;
  while (s->count == 0) {
    pthread_cond_wait(&s->nonZero, &s->lock);
  }
  s->waiterCount -= 1;
  s->count -= 1;
  pthread_mutex_unlock(&s->lock);
}

bool gcore::Semaphore::tryDecrement() {
  details::sema_t *s = (details::sema_t*)(&mData[0]);
  
  pthread_mutex_lock(&s->lock);
  
  s->waiterCount += 1;
  
  bool success = (s->count > 0);
  
  s->waiterCount -= 1;
  
  if (success) {
    s->count -= 1;
  }
  
  pthread_mutex_unlock(&s->lock);
  
  return success;
}

bool gcore::Semaphore::timedDecrement(unsigned long ms) {
  details::sema_t *s = (details::sema_t*)(&mData[0]);
  
  struct timespec ts;
  struct timeval tv;
  bool timedout = false;
  
  pthread_mutex_lock(&s->lock);
  
  s->waiterCount += 1;
  
  gettimeofday(&tv, 0);
  ts.tv_nsec  = ms * 1000000 + tv.tv_usec * 1000;
  ts.tv_sec   = ts.tv_nsec / 1000000000;
  ts.tv_nsec -= ts.tv_sec * 1000000000;
  ts.tv_sec  += ts.tv_sec;
  
  if (s->count == 0) {
    timedout = (pthread_cond_timedwait(&s->nonZero, &s->lock, &ts) != 0);
  }
  
  s->waiterCount -= 1;
  
  if (!timedout) {
    s->count -= 1;
  }
  
  pthread_mutex_unlock(&s->lock);
  
  return !timedout;
}

/*
gcore::Semaphore::Semaphore(long init, long maxval)
  : mMax(maxval) {
  sem_init((sem_t*)mData, 0, (unsigned int)init);
}

gcore::Semaphore::~Semaphore() {
  sem_destroy((sem_t*)mData);
}

void gcore::Semaphore::increment(long count) {
  int cval = 0;
  if (sem_getvalue((sem_t*)mData, &cval) == 0) {
    while (count > 0 && cval < mMax) {
      sem_post((sem_t*)mData);
      --count;
      ++cval;
    }
  }
  //sem_post((sem_t*)mData);
}

void gcore::Semaphore::decrement() {
  sem_wait((sem_t*)mData);
}

bool gcore::Semaphore::timedDecrement(unsigned long ms) {
  struct timespec ts;
  struct timeval tv;
  
  gettimeofday(&tv, 0);
  
  ts.tv_nsec  = (tv.tv_usec * 1000) + (ms * 1000000);
  ts.tv_sec   = ts.tv_nsec / 1000000000;
  ts.tv_nsec -= ts.tv_sec * 1000000000;
  ts.tv_sec  += tv.tv_sec;
  
#ifndef __APPLE__
  while (1) {
    if (sem_timedwait((sem_t*)mData, &ts) != 0) {
      if (errno != EINTR) {
        return false;
      }
    } else {
      return true;
    }
  }
#else
  // Not supported on OSX
#endif
  
  return false;
}

bool gcore::Semaphore::tryDecrement() {
  return (sem_trywait((sem_t*)mData) == 0);
}
*/

#endif



gcore::Thread::Thread(gcore::Thread::Procedure proc,
                     gcore::Thread::EndCallback end,
                     bool waitStart)
  : mSelf(0), mRunning(false), mProc(proc), mEndCB(end),
    mStarted(false) {
  restart(waitStart);
}

gcore::Thread::~Thread() {
  if (detachable()) {
    detach();
  }
}

void gcore::Thread::notifyStarted() {
  mMutex.lock();
  mStarted = true;
  mStartedCond.notify();
  mMutex.unlock();
}

void gcore::Thread::waitStarted() {
  mMutex.lock();
  while (mStarted == false) {
    mStartedCond.wait(mMutex);
  }
  mMutex.unlock();
}

bool gcore::Thread::detachable() const {
  return (mRunning == true && mProc != 0);
}

bool gcore::Thread::suspendable() const {
  return (mRunning == true && mProc != 0);
}

bool gcore::Thread::cancelable() const {
  return (mRunning == true && mProc != 0);
}

bool gcore::Thread::joinable() const {
  return (mRunning == true && mProc != 0);
}

bool gcore::Thread::running() const {
  return mRunning;
}

#ifdef _WIN32

gcore::ThreadID gcore::Thread::id() const {
  return mSelfId;
}

unsigned int __stdcall gcore::Thread::_ThreadEntryFunc(void *data) {
  register int ret = -1;
  
  gcore::Thread *thr = (gcore::Thread*)data;
  
  if (thr) {
    
    thr->notifyStarted();
    
    try {
      
      ret = thr->mProc();
      
      thr->mRunning = false;
      
      if (thr->mEndCB) {
        thr->mEndCB(ret);
      }
      
    } catch (...) {
      fprintf(stderr, "Error running thread: %p\n", thr->id());
    }
  }
  return ret;
}

gcore::Thread::Thread()
  : mSelf(0), mRunning(false), mStarted(false) {
  DWORD tid = GetCurrentThreadId();
  mSelf = (void*) OpenThread(THREAD_ALL_ACCESS, true, tid);
  mSelfId = (void*) tid;
  mRunning = (mSelf != 0);
}

bool gcore::Thread::restart(bool waitStart) {
  
  if (mRunning == true) {
    fprintf(stderr, "Thread is already running...\n");
    return false;
  }
  
  if (mProc == 0) {
    fprintf(stderr, "No thread procedure...\n");
    return false;
  }
  
  DWORD dw;
  
  mStarted = false;
  mRunning = true;
  
  mSelf = (ThreadID)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&_ThreadEntryFunc, this, 0, &dw);
  
  if (mSelf != 0) {
    mSelfId = (void*) GetThreadId((HANDLE)mSelf);
    //mRunning = true;
    if (waitStart) {
      waitStarted();
    }
    return true;
  } else {
    return false;
  }
  
}

bool gcore::Thread::detach() {
  if (detachable()) {
    CloseHandle((HANDLE)mSelf);
    mRunning = false;
    mSelf = 0;
    mSelfId = 0;
    return true;
  }
  return false;
}

bool gcore::Thread::join(int *retval) {
  if (joinable()) {
    if (WaitForSingleObject((HANDLE)mSelf,INFINITE) == WAIT_OBJECT_0) {
      if (retval != 0) {
        DWORD dw;
        GetExitCodeThread((HANDLE)mSelf, &dw);
        *retval = dw;
      }
      CloseHandle((HANDLE)mSelf);
      mSelf = 0;
      mSelfId = 0;
      return true;
    }
  }
  return false;
}

bool gcore::Thread::cancel() {
  if (cancelable()) {
    if (TerminateThread((HANDLE)mSelf, 0)) {
      CloseHandle((HANDLE)mSelf);
      mSelf = 0;
      mSelfId = 0;
      mRunning = false;
      return true;
    }
  }
  return false;
}

bool gcore::Thread::suspend() {
  if (suspendable()) {
    return (SuspendThread((HANDLE)mSelf) != -1);
  }
  return false;
}

bool gcore::Thread::resume() {
  if (suspendable()) {
    return (ResumeThread((HANDLE)mSelf) != -1);
  }
  return false;
}

gcore::Thread::Priority gcore::Thread::priority() const {
  Priority result = PRI_UNKNOWN;
  if (mRunning) {
    int pri = GetThreadPriority((HANDLE)mSelf);
    if (pri != THREAD_PRIORITY_ERROR_RETURN) {
      switch(pri) {
      case THREAD_PRIORITY_IDLE:
        result = PRI_VERY_LOW;
        break;
      case THREAD_PRIORITY_BELOW_NORMAL:
        result = PRI_LOW;
        break;
      case THREAD_PRIORITY_NORMAL:
        result = PRI_NORMAL;
        break;
      case THREAD_PRIORITY_ABOVE_NORMAL:
        result = PRI_HIGH;
        break;
      case THREAD_PRIORITY_HIGHEST:
        result = PRI_VERY_HIGH;
        break;
      default:
        result = PRI_DEFAULT;
        break;
      }
    }
  }
  return result;
}

bool gcore::Thread::priority(gcore::Thread::Priority prio) {
  if (mRunning) {
    int pri;
    switch (prio) {
    case PRI_VERY_LOW:
      pri = THREAD_PRIORITY_IDLE;
      break;
    case PRI_LOW:
      pri = THREAD_PRIORITY_BELOW_NORMAL;
      break;
    case PRI_NORMAL:
      pri = THREAD_PRIORITY_NORMAL;
      break;
    case PRI_HIGH:
      pri = THREAD_PRIORITY_ABOVE_NORMAL;
      break;
    case PRI_VERY_HIGH:
      pri = THREAD_PRIORITY_HIGHEST;
      break;
    default:
      pri = THREAD_PRIORITY_NORMAL;
      break;
    }
    return (SetThreadPriority((HANDLE)mSelf, pri) == TRUE);
  }
  return false;
}

gcore::Thread::Scheduling gcore::Thread::scheduling() const {
  return SCH_DEFAULT;
}

bool gcore::Thread::scheduling(gcore::Thread::Scheduling) {
  return true;
}

void gcore::Thread::SleepCurrent(unsigned long msec) {
  Sleep(msec);
}

void gcore::Thread::YieldCurrent() {
  Sleep(0);
}

gcore::ThreadID gcore::Thread::CurrentID() {
  return (void*) GetCurrentThreadId();
}

int gcore::Thread::GetProcessorCount() {
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return int(info.dwNumberOfProcessors);
}

#else

#ifdef __APPLE__
# ifdef Status
#   undef Status
# endif
# include <CoreServices/CoreServices.h>
#endif

gcore::ThreadID gcore::Thread::id() const {
  return mSelf;
}

void* gcore::Thread::_ThreadEntryFunc(void *data) {
  register int ret = 0;
  
  Thread *thr = (Thread*)data;
  
  //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  
  if (thr) {
    
    thr->notifyStarted();
  
    try {
      
      ret = thr->mProc();
    
      thr->mRunning = false;

      if (thr->mEndCB != 0) {
        thr->mEndCB(ret);
      }
    
    } catch (...) {
      int tid = (int)(size_t) thr->id();
      fprintf(stderr, "\nError running thread: %d\n", tid);
    }  
  }
  
  return reinterpret_cast<void*>(ret);
}

gcore::Thread::Thread()
  : mSelf(0), mRunning(false), mStarted(false) {
  mSelf = (ThreadID)pthread_self();
  mRunning = (mSelf != 0);
}

bool gcore::Thread::restart(bool waitStart) {
  
  if (mRunning == true) {
    fprintf(stderr, "Thread already running...\n");
    return false;
  }
  
  if (mProc == 0) {
    fprintf(stderr, "No procedure for thread...\n");
    return false;
  }

  pthread_t thr;
  pthread_attr_t attr;
  
  pthread_attr_init(&attr);
  
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
  
  mStarted = false;
  mRunning = true; // before create thread
  
  bool created = (pthread_create(
    &thr, &attr, &_ThreadEntryFunc, (void*)this) == 0);
  
  pthread_attr_destroy(&attr);
  
  if (created) {
    mSelf = (ThreadID)thr;
    //mRunning = true;
    if (waitStart) {
      waitStarted();
    }
    return true;
  } else {
    return false;
  }
}

bool gcore::Thread::detach() {
  if (detachable()) {
    bool detached = (pthread_detach((pthread_t)mSelf) == 0);
    if (detached) {
      mRunning = false;
      mSelf = 0;
      return true;
    }
  }
  return false;
}

bool gcore::Thread::join(int *retval) {
  if (joinable()) {
    void *rval = NULL;
    if (pthread_join((pthread_t)mSelf, &rval) == 0) {
      if (retval) {
        *retval = (int)(size_t)rval;
      }
      mSelf = 0;
      return true;
    }
  }
  return false;
}

bool gcore::Thread::cancel() {
  if (cancelable()) {
    if (pthread_cancel((pthread_t)mSelf) == 0) {
      pthread_join((pthread_t)mSelf, NULL);
      mSelf = 0;
      mRunning = false;
      return true;
    }
  }
  return false;
}

bool gcore::Thread::suspend() {
  if (suspendable()) {
    return (pthread_kill((pthread_t)mSelf, SIGSTOP) == 0);
  }
  return false;
}

bool gcore::Thread::resume() {
  if (suspendable()) {
    return (pthread_kill((pthread_t)mSelf, SIGCONT) == 0);
  }
  return false;
}

gcore::Thread::Priority gcore::Thread::priority() const {
  Priority result = PRI_UNKNOWN;
  if (mRunning) {
    sched_param sched; //{0, 0};
    int plcy = 0;
    if (pthread_getschedparam((pthread_t)mSelf, &plcy, &sched) == 0) {
#if defined(_POSIX_PRIORITY_SCHEDULING)
      int priomax = sched_get_priority_max(plcy);
      int priomin = sched_get_priority_min(plcy);
#elif defined(PTHREAD_MINPRIORITY) && defined(PTHREAD_MAX_PRIORITY)
      int priomin = PTHREAD_MIN_PRIORITY;
      int priomax = PTHREAD_MAX_PRIORITY;
#else
      int priomin = 0;
      int priomax = 32;
#endif
      int priomed = (priomax + priomin) / 2;
      if (sched.sched_priority < priomed) {
        if (sched.sched_priority <= priomin) {
          result = PRI_VERY_LOW;
        } else {
          result = PRI_LOW;
        }
      } else if (sched.sched_priority < priomed) {
        if (sched.sched_priority >= priomax) {
          result = PRI_VERY_HIGH;
        } else {
          result = PRI_HIGH;
        }
      } else {
        result = PRI_NORMAL;
      }
      return result;
    }
  }
  return result;
}

bool gcore::Thread::priority(gcore::Thread::Priority prio) {
  if (mRunning) {
    sched_param sched; // = {0, 0};
    int plcy = 0;
    if (pthread_getschedparam((pthread_t)mSelf, &plcy, &sched) == 0) {
#if defined(_POSIX_PRIORITY_SCHEDULING)
      int priomax = sched_get_priority_max(plcy);
      int priomin = sched_get_priority_min(plcy);
#elif defined(PTHREAD_MINPRIORITY) && defined(PTHREAD_MAX_PRIORITY)
      int priomin = PTHREAD_MIN_PRIORITY;
      int priomax = PTHREAD_MAX_PRIORITY;
#else
      int priomin = 0;
      int priomax = 32;
#endif
      int priomed = (priomax + priomin) / 2;
      switch (prio) {
      case PRI_VERY_LOW:
        sched.sched_priority = priomin;
        break;
      case PRI_LOW:
        sched.sched_priority = (priomin + priomed) / 2;
        break;
      case PRI_NORMAL:
        sched.sched_priority = priomed;
        break;
      case PRI_HIGH:
        sched.sched_priority = (priomax + priomed) / 2;
        break;
      case PRI_VERY_HIGH:
        sched.sched_priority = priomax;
        break;
      default:
        sched.sched_priority = priomed;
        break;
      }
      return (pthread_setschedparam((pthread_t)mSelf, plcy, &sched) == 0);
    }
  }
  return false;
}

gcore::Thread::Scheduling gcore::Thread::scheduling() const {
  Scheduling result = SCH_UNKNOWN;
  if (mRunning) {
    sched_param sched; // = {0, 0};
    int plcy = 0;
    if (pthread_getschedparam((pthread_t)mSelf, &plcy, &sched) == 0) {
      switch (plcy) {
      case SCHED_FIFO:
        result = SCH_FIFO;
        break;
      case SCHED_RR:
        result = SCH_RR;
        break;
      default:
        result = SCH_DEFAULT;
        break;
      }
    }
  }
  return result;
}

bool gcore::Thread::scheduling(gcore::Thread::Scheduling sced) {
  if (mRunning) {
    sched_param sched; // = {0, 0};
    int oldplcy = 0;
    int newplcy = 0;
    if (pthread_getschedparam((pthread_t)mSelf, &oldplcy, &sched) == 0) {
      switch (sced) {
      case SCH_FIFO:
        newplcy = SCHED_FIFO;
        break;
      case SCH_RR:
        newplcy = SCHED_RR;
        break;
      default:
        newplcy = SCHED_OTHER;
        break;
      }
      return (pthread_setschedparam((pthread_t)mSelf, newplcy, &sched) == 0);
    }
  }
  return false;
}

void gcore::Thread::SleepCurrent(unsigned long msec) {
  struct timespec ts;
  unsigned long nsec = msec * 1000000;
  ts.tv_sec = nsec / 1000000000;
  ts.tv_nsec = nsec - (ts.tv_sec * 1000000000);
  nanosleep(&ts,NULL);
}

void gcore::Thread::YieldCurrent() {
  sched_yield();
}

gcore::ThreadID gcore::Thread::CurrentID() {
  return (ThreadID)pthread_self();
}

int gcore::Thread::GetProcessorCount() {
//#if defined(__APPLE__)
//  return MPProcessors();
//#else
  int result = sysconf(_SC_NPROCESSORS_ONLN);
  if (result < 0) {
    result = 1;
  }
  return result;
//#endif
}

#endif


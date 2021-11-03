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

#include <gcore/threadpool.h>

namespace gcore {

ThreadPool::Worker::Worker(ThreadPool *pool)
  : mThr(0), mPool(pool), mProcessing(false) {
  mThr = new Thread(this, &Worker::run, &Worker::done);
}

ThreadPool::Worker::~Worker() {
  delete mThr;
}

int ThreadPool::Worker::run() {
  Task task;
  while ((task = mPool->getTask(this)) != NullTask) {
    task();
    mPool->notifyTaskDone(this);
  }
  return 0;
}

void ThreadPool::Worker::done(int) {
  // notify pool to remove this worker
  mPool->notifyDone(this);
}

// ---

ThreadPool::ThreadPool()
  : mState(TPS_STOPPED), mDriverThread(Thread::CurrentID()), mRestartWorkersCount(0) {
}

ThreadPool::~ThreadPool() {
  stop();
}

size_t ThreadPool::_numIdleWorkers() {
  size_t n = 0;
  for (size_t i=0; i<mWorkers.size(); ++i) {
    n += (mWorkers[i]->processing() ? 0 : 1);
  }
  return n;
}

size_t ThreadPool::numIdleWorkers() {
  ScopeLock lock(mWorkersAccess);
  return _numIdleWorkers();
}

size_t ThreadPool::numWorkers() {
  ScopeLock lock(mWorkersAccess);
  return mWorkers.size();
}

bool ThreadPool::restart() {
  if (Thread::CurrentID() == mDriverThread) {
    start(mRestartWorkersCount);
    return true;
  }
  return false;
}

bool ThreadPool::start(size_t numThreads) {
  
  if (Thread::CurrentID() != mDriverThread) {
    return false;
  }
  
  mTasksAccess.lock();
  
  if (mState != TPS_STOPPED) {
    // pool is already running
    mTasksAccess.unlock();
    return true;
  }
  
  // adjust threads count
  
  if (numThreads == 0) {
    numThreads = Thread::GetProcessorCount();
  }
  
  // add workers

  mWorkersAccess.lock();
  assert(mWorkers.size() == 0);
  mWorkers.resize(numThreads);
  for (size_t i=0; i<mWorkers.size(); ++i) {
    // threads will start straight away and be lock in getTask()
    mWorkers[i] = new Worker(this);
  }
  mWorkersAccess.unlock();
  
  mState = TPS_RUNNING;
  mTasksChanged.notifyAll();
  mTasksAccess.unlock();
  
  return true;
}

bool ThreadPool::wait() {
  
  if (Thread::CurrentID() != mDriverThread) {
    return false;
  }
  
  mTasksAccess.lock();
  
  if (mState != TPS_RUNNING) {
    // pool is not running
    mTasksAccess.unlock();
    return true;
  }

  mState = TPS_WAITING;
  
  // wait until we have no more tasks in queue
  while (mTasks.size() > 0) {
    mTasksChanged.wait(mTasksAccess);
  }
  
  mTasksChanged.notifyAll();
  mTasksAccess.unlock();
  
  // wait for all workers to run idle
  mWorkersAccess.lock();
  while (_numIdleWorkers() != mWorkers.size()) {
    mWorkersChanged.wait(mWorkersAccess);
  }
  mWorkersAccess.unlock();
  
  // switch back status to "running"
  mTasksAccess.lock();
  mState = TPS_RUNNING;
  mTasksChanged.notifyAll();
  mTasksAccess.unlock();
  
  return true;
}

bool ThreadPool::stop() {
  
  if (Thread::CurrentID() != mDriverThread) {
    return false;
  }
  
  mTasksAccess.lock();
  
  if (mState != TPS_RUNNING) {
    // pool is not running
    mTasksAccess.unlock();
    return true;
  }
  
  mState = TPS_STOPPED;
  mTasksChanged.notifyAll();
  mTasksAccess.unlock();

  mRestartWorkersCount = numWorkers();
  
  // wait all workers to be done

  mWorkersAccess.lock();
  while (mWorkers.size() > 0) {
    mWorkersChanged.wait(mWorkersAccess);
  }
  mWorkersAccess.unlock();
  
  return true;
}

bool ThreadPool::runTask(Task task, bool /*wait*/) {
  
  if (Thread::CurrentID() != mDriverThread) {
    return false;
  }
  
  mTasksAccess.lock();
  
  if (mState != TPS_RUNNING) {
    mTasksAccess.unlock();
    return false;
  }
  
  mTasks.push_back(task);
  mTasksChanged.notifyAll();
  mTasksAccess.unlock();

  return true;
}

void ThreadPool::notifyTaskDone(Worker *wt) {
  mWorkersAccess.lock();
  wt->processing(false);
  mWorkersChanged.notifyAll();
  mWorkersAccess.unlock();
}

void ThreadPool::notifyDone(Worker *wt) {
  
  mWorkersAccess.lock();
  
  List<Worker*>::iterator it = std::find(mWorkers.begin(), mWorkers.end(), wt);
  
  if (it != mWorkers.end()) {
    delete *it;
    mWorkers.erase(it);
    mWorkersChanged.notify();
    mWorkersAccess.unlock();
    
  } else {
    mWorkersAccess.unlock();
  }
}

Task ThreadPool::getTask(Worker *wt) {
  
  Task t = NullTask;

  mTasksAccess.lock();
  
  if (mState == TPS_STOPPED) {
    // pool is not running return NullTask
    mTasksAccess.unlock();

  } else {
    
    // wait until we have an available task or pool is stopped
    while (mTasks.size() == 0 && mState != TPS_STOPPED) {
      mTasksChanged.wait(mTasksAccess);
    }
    
    if (mState == TPS_STOPPED) {
      mTasksAccess.unlock();
      
    } else {
      assert(mTasks.size() > 0);
      // we have pending task(s)
      t = mTasks.front();
      wt->processing(true);
      mTasks.pop_front();
      mTasksChanged.notifyAll();
      mTasksAccess.unlock();
      
    }
  }
  
  return t;
}

bool ThreadPool::addWorkers(size_t n) {
  
  mTasksAccess.lock();
  
  if (mState == TPS_WAITING) {
    // pool is waiting for tasks to complete, cannot modify workers
    mTasksAccess.unlock();
    return false;
  
  } else if (mState == TPS_RUNNING) {
    // pool is running, stop it once
    mTasksAccess.unlock();
    stop();

  } else {
    mTasksAccess.unlock();
  }
  
  // mRestartWorkersCount is set in ThreadPool::stop
  start(n + mRestartWorkersCount);
  
  return true;
}

bool ThreadPool::removeWorkers(size_t n) {
  
  mTasksAccess.lock();
  
  if (mState == TPS_WAITING) {
    // pool is waiting for tasks to complete, cannot modify workers
    mTasksAccess.unlock();
    return false;

  } else if (mState == TPS_RUNNING) {
    // pool is running, stop it once
    mTasksAccess.unlock();
    stop();
  
  } else {
    mTasksAccess.unlock();
  }
  
  if (n < mRestartWorkersCount) {
    // restart pool if we have at least one worker left
    start(mRestartWorkersCount - n);
  }
  
  return true;
}
  
}


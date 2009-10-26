/*

Copyright (C) 2009  Gaetan Guidet

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
  mPool->notifyDone(this);
}

// ---

ThreadPool::ThreadPool()
  : mState(TPS_STOPPED), mRestartWorkersCount(0) {
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

void ThreadPool::restart() {
  start(mRestartWorkersCount);
}

void ThreadPool::start(size_t numThreads) {
  
  if (numThreads == 0) {
    numThreads = Thread::GetProcessorCount();
  }

  ScopeLock lock(mStateTaskAccess);

  if (mState != TPS_STOPPED) {
    return;
  }
  
  // add workers
  mWorkersAccess.lock();
  mWorkers.resize(numThreads);
  for (size_t i=0; i<mWorkers.size(); ++i) {
    mWorkers[i] = new Worker(this);
  }
  mWorkersAccess.unlock();
  
  // change state and notify
  mState = TPS_RUNNING;
  mStateTaskChanged.notifyAll();
}

void ThreadPool::wait() {
  ScopeLock lock(mStateTaskAccess);  

  if (mState != TPS_RUNNING) {
    return;
  }

  mState = TPS_WAITING;
  mStateTaskChanged.notifyAll();

  while (mTasks.size() > 0) {
    mStateTaskChanged.wait(mStateTaskAccess);
  }

  // wait all done
  mWorkersAccess.lock();
  while (_numIdleWorkers() != mWorkers.size()) {
    mWorkersChanged.wait(mWorkersAccess);
  }
  mWorkersAccess.unlock();

  mState = TPS_RUNNING;
  mStateTaskChanged.notifyAll();
}

void ThreadPool::stop() {
  // beware here do not use scope lock
  // WIN32 do not like recursive lock/unlock
  mStateTaskAccess.lock();
  
  if (mState != TPS_RUNNING) {
    mStateTaskAccess.unlock();
    return;
  }
  
  mRestartWorkersCount = numWorkers();
  mState = TPS_STOPPED;
  mStateTaskChanged.notifyAll();
  
  mStateTaskAccess.unlock();
  
  mWorkersAccess.lock();
  while (mWorkers.size() > 0) {
    mWorkersChanged.wait(mWorkersAccess);
  }
  mWorkersAccess.unlock();
}

bool ThreadPool::runTask(Task task, bool /*wait*/) {
  ScopeLock lock(mStateTaskAccess);

  if (mState != TPS_RUNNING) {
    return false;
  }

  mTasks.push_back(task);

  mStateTaskChanged.notifyAll();
  
  return true;
}

void ThreadPool::notifyTaskDone(Worker *wt) {
  ScopeLock lock(mWorkersAccess);
  
  std::vector<Worker*>::iterator it =
    std::find(mWorkers.begin(), mWorkers.end(), wt);
  
  if (it != mWorkers.end()) {
    (*it)->mProcessing = false;
  }

  mWorkersChanged.notify();
  //mWorkersChanged.notifyAll();
}

void ThreadPool::notifyDone(Worker *wt) {
  ScopeLock lock(mWorkersAccess);

  std::vector<Worker*>::iterator it =
    std::find(mWorkers.begin(), mWorkers.end(), wt);

  if (it != mWorkers.end()) {
    delete *it;
    mWorkers.erase(it);
    mWorkersChanged.notify();
    //mWorkersChanged.notifyAll();
  }
}

Task ThreadPool::getTask(Worker *wt) {
  // return a Null Task if no more task availbale and pool stopped
  Task t = NullTask;

  ScopeLock lock(mStateTaskAccess);

  if (mState == TPS_STOPPED) {
    return t;

  } else {
    
    while (mTasks.size() == 0 && mState != TPS_STOPPED) {
      mStateTaskChanged.wait(mStateTaskAccess);
    }

    if (mTasks.size() > 0) {
      t = mTasks.front();
      mTasks.pop_front();
      mStateTaskChanged.notifyAll();

      ScopeLock wlock(mWorkersAccess);
      
      std::vector<Worker*>::iterator it =
        std::find(mWorkers.begin(), mWorkers.end(), wt);
    
      if (it != mWorkers.end()) {
        // worker found, take the task
        (*it)->mProcessing = true;
        mWorkersChanged.notify();
        //mWorkersChanged.notifyAll();
      
      } else {
        // put task pack in queue
        mTasks.push_front(t);
        mStateTaskChanged.notifyAll();
        t = NullTask;
        
      } 
    }
  }
  return t;
}

void ThreadPool::addWorkers(size_t n) {
  ScopeLock lock(mStateTaskAccess);

  n += mWorkers.size();
  
  if (mState == TPS_WAITING) {
    std::cout << "*** Should never get here" << std::endl;
    return;
  
  } else if (mState == TPS_RUNNING) {
    ScopeUnlock unlock(mStateTaskAccess);
    stop();
  }
  
  if (mState == TPS_STOPPED) {
    ScopeUnlock unlock(mStateTaskAccess);
    start(n);
  }
}

void ThreadPool::removeWorkers(size_t n) {
  ScopeLock lock(mStateTaskAccess);
  
  if (n >= mWorkers.size()) {
    n = 0;
  } else {
    n = mWorkers.size() - n;
  }
  
  if (mState == TPS_WAITING) {
    std::cout << "*** Should never get here" << std::endl;
    return;
  } else if (mState == TPS_RUNNING) {
    ScopeUnlock unlock(mStateTaskAccess);
    stop();
  }
  
  if (mState == TPS_STOPPED && n > 0) {
    ScopeUnlock unlock(mStateTaskAccess);
    start(n);
  }
}
  
}


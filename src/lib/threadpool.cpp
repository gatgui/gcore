/*

Copyright (C) 2009~  Gaetan Guidet

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

namespace gcore
{

ThreadPool::Worker::Worker(ThreadPool *pool)
   : mThr(0)
   , mPool(pool)
   , mProcessing(false)
{
   mThr = new Thread(this, &Worker::run, &Worker::done);
}

ThreadPool::Worker::~Worker()
{
   delete mThr;
}

int ThreadPool::Worker::run()
{
   Task task;
   while ((task = mPool->getTask(this)) != NullTask)
   {
      task();
      mPool->notifyTaskDone(this);
   }
   return 0;
}

void ThreadPool::Worker::done(int)
{
   // notify pool to remove this worker
   mPool->notifyDone(this);
}

// ---

ThreadPool::ThreadPool()
   : mState(TPS_STOPPED)
   , mDriverThread(Thread::CurrentID())
   , mRestartWorkersCount(0)
{
}

ThreadPool::~ThreadPool()
{
   stop();
}

size_t ThreadPool::_numIdleWorkers()
{
   size_t n = 0;
   for (size_t i=0; i<mWorkers.size(); ++i)
   {
      n += (mWorkers[i]->processing() ? 0 : 1);
   }
   return n;
}

size_t ThreadPool::numIdleWorkers()
{
   ScopeLock lock(mWorkersAccess);
   return _numIdleWorkers();
}

size_t ThreadPool::numWorkers()
{
   ScopeLock lock(mWorkersAccess);
   return mWorkers.size();
}

bool ThreadPool::restart()
{
   if (Thread::CurrentID() == mDriverThread)
   {
      start(mRestartWorkersCount);
      return true;
   }
   return false;
}

bool ThreadPool::start(size_t numThreads)
{
   if (Thread::CurrentID() != mDriverThread)
   {
      return false;
   }
   
   mTasksAccess.lock();
   
   if (mState != TPS_STOPPED)
   {
      // pool is already running
      mTasksAccess.unlock();
      return true;
   }
   
   // adjust threads count
   
   if (numThreads == 0)
   {
      numThreads = Thread::ProcessorCount();
   }
   
   // add workers

   mWorkersAccess.lock();
   assert(mWorkers.size() == 0);
   mWorkers.resize(numThreads);
   for (size_t i=0; i<mWorkers.size(); ++i)
   {
      // threads will start straight away and be lock in getTask()
      mWorkers[i] = new Worker(this);
   }
   mWorkersAccess.unlock();
   
   mState = TPS_RUNNING;
   mTasksChanged.notifyAll();
   mTasksAccess.unlock();
   
   return true;
}

bool ThreadPool::wait()
{
   if (Thread::CurrentID() != mDriverThread)
   {
      return false;
   }
   
   mTasksAccess.lock();
   
   if (mState != TPS_RUNNING)
   {
      // pool is not running
      mTasksAccess.unlock();
      return true;
   }

   mState = TPS_WAITING;
   
   // wait until we have no more tasks in queue
   while (mTasks.size() > 0)
   {
      mTasksChanged.wait(mTasksAccess);
   }
   
   mTasksChanged.notifyAll();
   mTasksAccess.unlock();
   
   // wait for all workers to run idle
   mWorkersAccess.lock();
   while (_numIdleWorkers() != mWorkers.size())
   {
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

bool ThreadPool::stop()
{
   if (Thread::CurrentID() != mDriverThread)
   {
      return false;
   }
   
   mTasksAccess.lock();
   
   if (mState != TPS_RUNNING)
   {
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
   while (mWorkers.size() > 0)
   {
      mWorkersChanged.wait(mWorkersAccess);
   }
   mWorkersAccess.unlock();
   
   return true;
}

bool ThreadPool::runTask(Task task, bool /*wait*/)
{
   if (Thread::CurrentID() != mDriverThread)
   {
      return false;
   }
   
   mTasksAccess.lock();
   
   if (mState != TPS_RUNNING)
   {
      mTasksAccess.unlock();
      return false;
   }
   
   mTasks.push_back(task);
   mTasksChanged.notifyAll();
   mTasksAccess.unlock();

   return true;
}

void ThreadPool::notifyTaskDone(Worker *wt)
{
   mWorkersAccess.lock();
   wt->processing(false);
   mWorkersChanged.notifyAll();
   mWorkersAccess.unlock();
}

void ThreadPool::notifyDone(Worker *wt)
{
   mWorkersAccess.lock();
   
   List<Worker*>::iterator it = std::find(mWorkers.begin(), mWorkers.end(), wt);
   
   if (it != mWorkers.end())
   {
      delete *it;
      mWorkers.erase(it);
      mWorkersChanged.notify();
      mWorkersAccess.unlock();
   }
   else
   {
      mWorkersAccess.unlock();
   }
}

Task ThreadPool::getTask(Worker *wt)
{
   Task t = NullTask;

   mTasksAccess.lock();
   
   if (mState == TPS_STOPPED)
   {
      // pool is not running return NullTask
      mTasksAccess.unlock();
   }
   else
   {
      // wait until we have an available task or pool is stopped
      while (mTasks.size() == 0 && mState != TPS_STOPPED)
      {
         mTasksChanged.wait(mTasksAccess);
      }
      
      if (mState == TPS_STOPPED)
      {
         mTasksAccess.unlock();
      }
      else
      {
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

bool ThreadPool::addWorkers(size_t n)
{
   mTasksAccess.lock();
   
   if (mState == TPS_WAITING)
   {
      // pool is waiting for tasks to complete, cannot modify workers
      mTasksAccess.unlock();
      return false;
   }
   else if (mState == TPS_RUNNING)
   {
      // pool is running, stop it once
      mTasksAccess.unlock();
      stop();
   }
   else
   {
      mTasksAccess.unlock();
   }
   
   // mRestartWorkersCount is set in ThreadPool::stop
   start(n + mRestartWorkersCount);
   
   return true;
}

bool ThreadPool::removeWorkers(size_t n)
{
   mTasksAccess.lock();
   
   if (mState == TPS_WAITING)
   {
      // pool is waiting for tasks to complete, cannot modify workers
      mTasksAccess.unlock();
      return false;
   }
   else if (mState == TPS_RUNNING)
   {
      // pool is running, stop it once
      mTasksAccess.unlock();
      stop();
   }
   else
   {
      mTasksAccess.unlock();
   }
   
   if (n < mRestartWorkersCount)
   {
      // restart pool if we have at least one worker left
      start(mRestartWorkersCount - n);
   }
   
   return true;
}
   
}


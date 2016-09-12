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

#ifndef __gcore_threadpool_h_
#define __gcore_threadpool_h_

#include <gcore/threads.h>
#include <gcore/list.h>

namespace gcore
{
   typedef Functor0 Task;
   
   const Functor0 NullTask;
   
   class GCORE_API ThreadPool
   {
   public:
   
      ThreadPool();
      
      ~ThreadPool();

      bool start(size_t numThreads=0);
      
      bool restart();

      bool wait();

      bool stop();
      
      bool runTask(Task task, bool wait=true);
      
      bool addWorkers(size_t n);
      
      bool removeWorkers(size_t n);
      
      size_t numIdleWorkers();

      size_t numWorkers();

   protected:

      class Worker
      {
      private:
      
         Thread *mThr;
         ThreadPool *mPool;
         bool mProcessing;
      
      public:
         
         friend class ThreadPool;
      
         Worker(ThreadPool *p);
         ~Worker();
         
         inline bool processing() const
         {
            return mProcessing;
         }
         
         int run();
         void done(int);

      protected:
      
         inline void processing(bool p)
         {
            mProcessing = p;
         }
      };
      
      friend class Worker;
      
   protected:
      
      Task getTask(Worker *wt);

      void notifyTaskDone(Worker *wt);

      void notifyDone(Worker *wt);
      
      size_t _numIdleWorkers();

   protected:
      
      enum State
      {
         TPS_STOPPED,
         TPS_RUNNING,
         TPS_WAITING
      };
      
      State mState;
      ThreadID mDriverThread;
      List<Worker*> mWorkers;
      std::deque<Task> mTasks;
      size_t mRestartWorkersCount;
      
      Mutex mWorkersAccess;
      Condition mWorkersChanged;
      
      Mutex mTasksAccess;
      Condition mTasksChanged;
   };
}

#endif

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

#ifndef __gcore_threadpool_h_
#define __gcore_threadpool_h_

#include <gcore/threads.h>
#include <gcore/list.h>

namespace gcore {
  
  typedef Functor0 Task;
  
  const Functor0 NullTask;
  
  class GCORE_API ThreadPool {
    
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

      class Worker {
        private:
        
          Thread *mThr;
          ThreadPool *mPool;
          bool mProcessing;
        
        public:
          
          friend class ThreadPool;
        
          Worker(ThreadPool *p);
          ~Worker();
          
          inline bool processing() const {
            return mProcessing;
          }
          
          int run();
          void done(int);

        protected:
        
          inline void processing(bool p) {
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
      
      enum State {
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

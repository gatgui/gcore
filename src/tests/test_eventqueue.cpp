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

#include <gcore/threads.h>
#include <gcore/functor.h>
#include <gcore/threadpool.h>
#include <gcore/eventqueue.h>
#include <deque>
#include <string>
#include <iostream>
#include <cstdarg>

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
Mutex gPrintMutex(true);
std::map<ThreadID, std::string> gThreadNames;

void safePrint(const char *fmt, ...)
{
  va_list args;
  
  gPrintMutex.lock();
  
  fprintf(stdout, "In thread %s - ", gThreadNames[Thread::CurrentID()].c_str());
  
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end(args);
  
  gPrintMutex.unlock();
}

#ifdef _DEBUG
void safeDebugPrint(const char *fmt, ...)
{
  va_list args;
  
  gPrintMutex.lock();
  
  fprintf(stdout, "In thread %s - ", gThreadNames[Thread::CurrentID()].c_str());
  
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end(args);
  
  gPrintMutex.unlock();
}
#else
void safeDebugPrint(const char *, ...)
{
}
#endif

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
  std::cout << "Thread 1: 0x" << std::hex << Thread::CurrentID() << std::dec << std::endl;
  
  Event func;
  
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
  std::cout << "Thread 2: 0x" << std::hex << Thread::CurrentID() << std::dec << std::endl;
  
  Event func;
  
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
  std::cout << "Thread 3: 0x" << std::hex << Thread::CurrentID() << std::dec << std::endl;
  
  Event func;
  
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
  std::cout << "Thread 4: 0x" << std::hex << Thread::CurrentID() << std::dec << std::endl;
  
  Event func;
  
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
  std::cout << "Main thread: 0x" << std::hex << Thread::CurrentID() << std::dec << std::endl;
  
  EventQueue mainThreadQueue;
  gMainThreadQueue = &mainThreadQueue;
  
  ThreadPool tpool;
  
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
  
  safePrint("Finished\n");
  
  return 0;
}


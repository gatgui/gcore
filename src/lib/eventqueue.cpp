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

#include <gcore/eventqueue.h>

namespace gcore
{

EventQueue::EventQueue()
  : mOwner(Thread::CurrentID()), mAcceptEvents(true), mDoneID((size_t)-1), mCurID(0)
{
}

EventQueue::~EventQueue()
{
  // will this be called by the right thread?
  acceptEvents(false);
  // shall we execute remaining events if we are in the right thread
  mEvents.clear();
}

bool EventQueue::push(Event evt, bool sync)
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
    
    EventInfo ei = {evt, sync, id};
    
    mQueueIDs.insert(id);
    mEvents.push_back(ei);
    
    mEventsMutex.unlock();
    
    if (sync)
    {
      mDoneMutex.lock();
      while (mDoneID != id)
      {
        mDoneCond.wait(mDoneMutex);
      }
      mDoneID = (size_t)-1;
      mDoneMutex.unlock();
    }
    
    return true;
  }
  else
  {
    evt();
    return true;
  }
}

size_t EventQueue::poll(size_t count)
{
  if (Thread::CurrentID() != mOwner)
  {
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
  
  EventInfoList::iterator first = mEvents.begin();
  EventInfoList::iterator last = first + count;
  
  EventInfoList tmp(first, last);
  
  mEvents.erase(first, last);
  
  mEventsMutex.unlock();
  
  for (size_t i=0; i<tmp.size(); ++i)
  {
    EventInfo &ei = tmp[i];
    
    ei.func();
    
    if (ei.sync)
    {
      mDoneMutex.lock();
      mDoneID = ei.id;
      while (mDoneID != (size_t)-1)
      {
        ScopeUnlock su(mDoneMutex);
        mDoneCond.notify();
      }
      mDoneMutex.unlock();
    }
    
    ScopeLock sl(mEventsMutex);
    mQueueIDs.erase(mQueueIDs.find(ei.id));
  }
  
  return count;
}

void EventQueue::acceptEvents(bool v)
{
  if (Thread::CurrentID() == mOwner)
  {
    mEventsMutex.lock();
    mAcceptEvents = v;
    mEventsMutex.unlock();
  }
}

size_t EventQueue::newID()
{
  size_t id = mCurID++;
  
  while (mQueueIDs.find(id) != mQueueIDs.end())
  {
    id = mCurID++;
  }
  return id;
  
  //return mCurID++;
}

}


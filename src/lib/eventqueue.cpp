/*

Copyright (C) 2010~  Gaetan Guidet

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

#include <gcore/eventqueue.h>

namespace gcore
{

EventQueue::EventQueue()
   : mOwner(Thread::CurrentID())
   , mAcceptEvents(true)
   , mDoneID((size_t)-1)
   , mCurID(0)
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
}

} // gcore


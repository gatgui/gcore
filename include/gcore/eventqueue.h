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

#ifndef __gcore_eventqueue_h_
#define __gcore_eventqueue_h_

#include <gcore/threads.h>

namespace gcore
{
   typedef Functor0 Event;
   
   class GCORE_API EventQueue
   {
   public:
      
      struct EventInfo
      {
         Event func;
         bool sync;
         size_t id;
      };
      
      typedef std::deque<EventInfo> EventInfoList;
      
   public:
      
      EventQueue();
      virtual ~EventQueue();
      
      bool push(Event evt, bool sync=false);
      size_t poll(size_t count);
      void acceptEvents(bool v);
      
   private:
      
      EventQueue(const EventQueue &);
      EventQueue& operator=(const EventQueue &);
      
      size_t newID();
      
   protected:
      
      ThreadID mOwner;
      EventInfoList mEvents;
      bool mAcceptEvents;
      size_t mDoneID;
      std::set<size_t> mQueueIDs;
      size_t mCurID;
      
      Mutex mEventsMutex;
      Mutex mDoneMutex;
      Condition mDoneCond;
   };
}

#endif

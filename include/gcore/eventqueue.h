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

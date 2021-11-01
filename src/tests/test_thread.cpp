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

#include <gcore/threads.h>

class State
{
public:
   
   State()
      :m_done1(false), m_done2(false)
   {
   }
   
   bool isT1Done() const
   {
      return m_done1;
   }

   void waitT1Done() const
   {
      m_mtx.lock();
      while (m_done1 == false)
      {
         m_t1done.wait(m_mtx);
      }
      m_mtx.unlock();
   }

   bool isT2Done() const
   {
      return m_done2;
   }

   void waitT2Done() const
   {
      m_mtx.lock();
      while (m_done2 == false)
      {
         m_t2done.wait(m_mtx);
      }
      m_mtx.unlock();
   }

   void setT1Done(bool b, bool notify=false)
   {
      m_done1 = b;
      if (notify)
      {
         m_t1done.notifyAll();
      }
   }
   void setT2Done(bool b, bool notify=false)
   {
      m_done2 = b;
      if (notify)
      {
         m_t2done.notifyAll();
      }
   }

   void print(FILE *file, const char *format, ...)
   {
      m_pmtx.lock();
      va_list al;
      va_start(al, format);
      vfprintf(file, format, al);
      va_end(al);
      m_pmtx.unlock();
   }

   int run1()
   {
      print(stderr, "\nThread1: I have to prepare things before Thread 2 comes\n");

      for (int i=0; i<1000; ++i)
      {
         print(stderr, "*");
      }
      
      print(stderr, "\nThread1: Hum... let's wait for Thread2 to start\n");

      waitT2Done();
      
      print(stderr, "\nThread1: Yeah i can do my stuffs !!!\n");

      return 0;
   }
   void done1(int)
   {
      print(stderr, "\nThread1: I'm done !!\n");
   }
   
   int run2()
   {
      print(stderr, "\nThread2: I Have to hurry, Thread1 might be waiting for me\n");

      for (int i=0; i<10000; ++i)
      {
         print(stderr, ".");
      }

      print(stderr, "\n");

      setT2Done(true, true);

      return 0;
   }
   void done2(int)
   {
      print(stderr, "\nThread2: I'm done !!\n");
   }

protected:

   bool  m_done1;
   bool  m_done2;
   mutable gcore::Mutex m_mtx;
   mutable gcore::Mutex m_pmtx;
   mutable gcore::Condition m_t1done;
   mutable gcore::Condition m_t2done;
};

int main(int, char **)
{
   State s;
   
   gcore::Thread thr1(&s, &State::run1, &State::done1);
   gcore::Thread thr2(&s, &State::run2, &State::done2);
   
   thr1.join();
   thr1.join();
   
   s.print(stderr, "\nThat's all folks !!\n");

   return 0;
}




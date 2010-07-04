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

#include <gcore/gcore>
#include <cmath>
#include <cstdarg>


// this make a 2 param method/function call into a no param function/method call
// PApp: Partial Application
template <typename A, typename B>
class PApp2 {
  
  public:

    template <typename C, typename D>
    PApp2(void (*Func)(C, D), A a, B b)
      : mA(a), mB(b) {
      gcore::Bind(Func, mProc);
    }
    
    template <class Callee, class Calltype, typename C, typename D>
    PApp2(Callee *c, void (Calltype::*Method)(C, D), A a, B b)
      : mA(a), mB(b) {
      gcore::Bind(c, Method, mProc);
    }

    ~PApp2() {
    }

    void apply() {
      mProc(mA, mB);
    }

  protected:
  
    gcore::Functor2<A, B> mProc;
    A mA;
    B mB;
};

gcore::Mutex IOMutex;

void safe_print(const char *fmt, ...) {
  IOMutex.lock();
  va_list args;
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end(args);
  IOMutex.unlock();
}

void ComputeFunc(char c, int i) {
  float sum = 0.0f;
  for (int j=0; j<i; ++j) {
    sum += float(pow(sin(j * 3.5687 / 180.0), 2.0));
  }
  safe_print("%c = %f\n", c, sum);
}


int main(int, char**) {
  
  int i;
  size_t n;

  gcore::Task task[4];
  
  PApp2<char, int> tmp0(ComputeFunc, '0', 10000);
  PApp2<char, int> tmp1(ComputeFunc, '1', 10000);
  PApp2<char, int> tmp2(ComputeFunc, '2', 10000);
  PApp2<char, int> tmp3(ComputeFunc, '3', 10000);
  
  gcore::Bind(&tmp0, &PApp2<char, int>::apply, task[0]);
  gcore::Bind(&tmp1, &PApp2<char, int>::apply, task[1]);
  gcore::Bind(&tmp2, &PApp2<char, int>::apply, task[2]);
  gcore::Bind(&tmp3, &PApp2<char, int>::apply, task[3]);

  gcore::ThreadPool pool;

  safe_print("Start thread pool\n");
  pool.start();
  
  n = pool.numWorkers();

  safe_print("Add 10 tasks\n");
  for (i=0; i<10; ++i) {
    pool.runTask(task[i%n]);
  }
  
  //gcore::Thread::YieldCurrent();

  safe_print("Add 2 workers...\n");
  pool.addWorkers(2);
  safe_print("  ...added\n");
  
  n += 2;
  
  safe_print("Add 10 tasks\n");
  for (i=0; i<10; ++i) {
    pool.runTask(task[i%n]);
  }
  
  //gcore::Thread::YieldCurrent();
  
  safe_print("Remove 3 workers...\n");
  pool.removeWorkers(3);
  safe_print("  ...removed\n");
  
  n -= 3;
  
  safe_print("Add 10 tasks\n");
  for (i=0; i<10; ++i) {
    pool.runTask(task[i%n]);
  }

  safe_print("Wait workers...\n");
  pool.wait();
  pool.stop();
  
  safe_print("Done!\n");

  return 0;
}

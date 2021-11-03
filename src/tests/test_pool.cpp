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

#include <gcore/all.h>
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
int gExecutedCount = 0;

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
  gExecutedCount++;
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
  pool.start(8);
  
  n = pool.numWorkers();
  safe_print("%d workers\n", n);

  safe_print("Add 10 tasks\n");
  for (i=0; i<10; ++i) {
    pool.runTask(task[i%4]);
  }
  
  safe_print("Wait workers...\n");
  pool.wait();
  
  n = pool.numWorkers();
  safe_print("%d workers waiting for tasks\n", n);
  
  safe_print("Add 300 tasks\n");
  for (i=0; i<300; ++i) {
    pool.runTask(task[i%4]);
  }

  safe_print("Add 2 workers...\n");
  pool.addWorkers(2);
  safe_print("  ...added\n");
  
  safe_print("Add 100 tasks\n");
  for (i=0; i<100; ++i) {
    pool.runTask(task[i%4]);
  }

  safe_print("Remove 6 workers...\n");
  pool.removeWorkers(6);
  safe_print("  ...removed\n");

  safe_print("Add 100 tasks\n");
  for (i=0; i<100; ++i) {
    pool.runTask(task[i%4]);
  }

  // stopping pool without waiting will result in un-executed tasks
  safe_print("Wait workers...\n");
  pool.wait();
  
  safe_print("Stop pool...\n");
  pool.stop();
  
  safe_print("Done, executed %d tasks\n", gExecutedCount);

  return 0;
}

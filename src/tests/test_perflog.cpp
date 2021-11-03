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

#include <gcore/perflog.h>
#include <gcore/log.h>
#include <cstdio>
#include <cstring>

using namespace gcore;

#define LOG_PERF(title) ScopedPerfLog logPerf(title)

double SpendTime(unsigned long n)
{
   double rv = 0.0;
   for (unsigned int i=0; i<n; ++i)
   {
      rv += double(i);
   }
   return rv;
}

void FuncE(unsigned long n)
{
   LOG_PERF("FuncE");
   SpendTime(n);
}

void FuncD(unsigned long n)
{
   LOG_PERF("FuncD");
   SpendTime(n);
}

void FuncC(unsigned long n)
{
   LOG_PERF("FuncC");
   FuncD(n << 1);
   SpendTime(n);
}

void FuncB(unsigned long n)
{
   LOG_PERF("FuncB");
   SpendTime(n);
   FuncD(n >> 1);
   FuncE(n << 1);
}

void FuncA(unsigned long n)
{
   LOG_PERF("FuncA");
   SpendTime(n);
   FuncB(n >> 1);
   FuncC(n >> 2);
}

int main(int argc, char **argv)
{
   unsigned long loop = 10;
   unsigned long n = 1000;
  
   if (argc >= 2)
   {
      unsigned long val;
      
      if (sscanf(argv[1], "%lu", &val) == 1)
      {
         loop = val;
      }
      
      if (argc >= 3)
      {
         if (sscanf(argv[2], "%lu", &val) == 1)
         {
            n = val;
         }
      }
   }
   
   LOG_PERF("Top");
   
   for (unsigned long i=0; i<loop; ++i)
   {
      FuncA(n);
   }
   
   // default print out
   PerfLog::Print(PerfLog::LogOutput, PerfLog::ShowAll|PerfLog::ShowDetailed|PerfLog::ShowFlat, PerfLog::SortTotalTime);
   
   return 0;
}


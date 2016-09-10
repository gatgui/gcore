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


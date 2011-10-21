#include <gcore/perflog.h>
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
   bool flat = false;
  
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
      
      if (argc >= 4)
      {
         flat = !strcmp(argv[3], "flat");
      }
   }
   
   for (unsigned long i=0; i<loop; ++i)
   {
      FuncA(n);
   }
   
   // default print out
   PerfLog::Print(std::cout, PerfLog::ShowAll|PerfLog::ShowDetailed|PerfLog::ShowFlat, PerfLog::SortTotalTime);
   
   return 0;
}


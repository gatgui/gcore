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

#include <gcore/net.h>
#include <gcore/threads.h>

int ReadStdin()
{
   bool end = false;
   char rdbuf[512];
   while (!end)
   {
      if (fgets(rdbuf, 512, stdin))
      {
         if (!strncmp(rdbuf, "QUIT", 4))
         {
            end = true;
         }
      }
   }
   return 0;
}

int main(int, char**)
{
   gcore::NetScopeInit gni;

   if (gni)
   {
      gcore::Status stat;
      gcore::TCPSocket socket(4001, &stat);
      
      if (!stat)
      {
         std::cerr << stat << std::endl;
         return 1;
      }
      
      stat = socket.bindAndListen(5);
      if (!stat)
      {
         std::cerr << stat << std::endl;
         return 1;
      }
      
      gcore::Thread thr(&ReadStdin, NULL, true);
      
      std::cout << "Type 'QUIT' to exit." << std::endl;
         
      while (thr.isRunning())
      {
         if (socket.selectReadable(0, &stat))
         {
            gcore::TCPConnection *conn;
            gcore::String data;
            
            while ((conn = socket.nextReadable()) != NULL)
            {
               if (conn->readUntil(" ", data, -1, &stat))
               {
                  std::cout << "\"" << data << "\"" << std::endl;
               }
               else if (!stat)
               {
                  std::cerr << stat << std::endl;
                  // should I wait here?
                  socket.close(conn);
               }
               else
               {
                  std::cout << "[partial] \"" << data << "\"" << std::endl;
               }
            }
         }
         else
         {
            if (!stat)
            {
               std::cerr << stat << std::endl;
               break;
            }
            else
            {
               // Passively wait 50ms
               // CPU consumption wise, tt is seems better to do select return immediately and follow with a sleep
               // than to do a timed out select
               gcore::Thread::SleepCurrent(50);
            }
         }
      }
      
      thr.join();
   }
   
   return 0;
}

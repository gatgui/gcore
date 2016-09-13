/*

Copyright (C) 2009~  Gaetan Guidet

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

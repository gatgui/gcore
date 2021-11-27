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

class ReadStdin
{
public:
   ReadStdin(gcore::TCPSocket *socket)
      : mSocket(socket)
   {
   }
   ~ReadStdin()
   {
   }
   int run()
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
      if (mSocket)
      {
         // mSocket->disconnect();
         mSocket->close();
      }
      return 0;
   }
private:
   gcore::TCPSocket *mSocket;
};

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
      
      ReadStdin reader(&socket);
      gcore::Thread thr(&reader, METHOD(ReadStdin, run), true);
      
      std::cout << "Type 'QUIT' to exit." << std::endl;
      
      while (thr.isRunning())
      {
         void *buffer = 0;
         size_t len = 0;
         
         gcore::TCPConnection *conn = socket.accept(&stat);
         if (!conn)
         {
            std::cerr << "Server error: " << stat << std::endl;
            continue;
         }
         
         //if (conn->read(buffer, len, -1, &stat)) {
         if (conn->readUntil(" ", 1, buffer, len, -1, &stat))
         {
            if (buffer)
            {
               std::cout << "\"" << (const char*)buffer << "\"" << std::endl;
            }
            else
            {
               std::cout << "<null>" << std::endl;
            }
         }
         else if (!stat)
         {
            std::cout << "Client error: " << stat << std::endl;
         }
         else
         {
            if (buffer)
            {
               std::cout << "[partial] \"" << (const char*)buffer << "\"" << std::endl;
            }
         }
         
         conn->free(buffer);
         
         socket.close(conn);
      }
      
      thr.join();
   }
   
   return 0;
}

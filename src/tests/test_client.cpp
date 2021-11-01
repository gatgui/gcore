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

int main(int argc, char **argv)
{
   gcore::NetScopeInit gni;
   
   if (gni)
   {
      if (argc > 3)
      {
         std::cout << "test_client [server [port]]" << std::endl;
         return 1;
      }
   
      gcore::String server = "localhost";
      unsigned short port = 4001;

      if (argc >= 2)
      {
         server = argv[1];
      }

      if (argc >= 3)
      {
         sscanf(argv[2], "%hu", &port);
      }
      
      gcore::Status stat;

      std::cout << "Get Host...";
      gcore::Host host(server, port);
      std::cout << "DONE: " << host.address() << ":" << host.port() << std::endl;
      
      gcore::TCPSocket socket(host, &stat);
      if (!stat)
      {
         std::cerr << stat << std::endl;
         return 1;
      }
      
      std::cout << "Connect to server..." << std::endl;
      gcore::TCPConnection *conn = socket.connect(&stat);
      if (!conn)
      {
         std::cerr << stat << std::endl;
         return 1;
      }

      std::cout << "Type 'QUIT' to exit." << std::endl;
         
      bool end = false;
      
      while (!end)
      {
         char buffer[512];
         
         std::cout << "Input text: ";
         if (!fgets(buffer, 512, stdin))
         {
            end = true;
            continue;
         }
         
         size_t len = strlen(buffer);
         if (len > 0 && buffer[len-1] == '\n')
         {
            buffer[len-1] = '\0';
         }
         
         if (!strcmp(buffer, "QUIT"))
         {
            conn->shutdown();
            end = true;
         }
         else
         {
            std::cout << "Send data: \"" << buffer << "\"..." << std::endl;
            if (conn->write(buffer, len-1, -1, &stat) == 0 && !stat)
            {
               // nothing written and stat set to failed
               std::cerr << stat << std::endl;
               end = true;
            }
         }
      }
      
      char buffer[8];
      std::cout << "Press any Key to Terminate" << std::endl;
      fgets(buffer, 8, stdin);
   }
   
   return 0;
}

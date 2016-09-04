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

#include <gcore/all.h>

int main(int argc, char **argv) {
  
  gcore::NetScopeInit gni;
  
  if (gni) {
    
    if (argc > 3) {
      std::cout << "test_client [server [port]]" << std::endl;
      return 1;
    }
  
    std::string server = "localhost";
    unsigned short port = 4001;

    if (argc >= 2) {
      server = argv[1];
    }

    if (argc >= 3) {
      sscanf(argv[2], "%hu", &port);
    }
    
    gcore::Status stat;

    std::cout << "Get Host...";
    gcore::Host host(server, port);
    std::cout << "DONE: " << host.address() << ":" << host.port() << std::endl;
    
    gcore::TCPSocket socket(host, &stat);
    if (!stat) {
      std::cerr << stat << std::endl;
      return 1;
    }
    
    std::cout << "Connect to server..." << std::endl;
    gcore::TCPConnection *conn = socket.connect(&stat);
    if (!conn) {
      std::cerr << stat << std::endl;
      return 1;
    }

    std::cout << "Type 'QUIT' to exit." << std::endl;
      
    bool end = false;
    
    while (!end) {
      
      char buffer[512];
      
      std::cout << "Input text: ";
      if (!fgets(buffer, 512, stdin)) {
        end = true;
        continue;
      }
      
      size_t len = strlen(buffer);
      if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
      }
      
      if (!strcmp(buffer, "QUIT")) {
        conn->shutdown();
        end = true;
      
      } else {
        std::cout << "Send data: \"" << buffer << "\"..." << std::endl;
        if (conn->write(buffer, len-1, -1, &stat) == 0 && !stat) {
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

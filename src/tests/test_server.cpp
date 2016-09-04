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

class ReadStdin {
public:
  ReadStdin(gcore::TCPSocket *socket)
    : mSocket(socket) {
  }
  ~ReadStdin() {
  }
  int run() {
    bool end = false;
    char rdbuf[512];
    while (!end) {
      if (fgets(rdbuf, 512, stdin)) {
        if (!strncmp(rdbuf, "QUIT", 4)) {
          end = true;
        }
      }
    }
    if (mSocket) {
      // mSocket->disconnect();
      mSocket->close();
    }
    return 0;
  }
private:
  gcore::TCPSocket *mSocket;
};

int main(int, char**) {
  
  gcore::NetScopeInit gni;
  
  if (gni)
  {
    gcore::Status stat;
    gcore::TCPSocket socket(4001, &stat);
    
    if (!stat) {
      std::cerr << stat << std::endl;
      return 1;
    }
    
    stat = socket.bindAndListen(5);
    if (!stat) {
      std::cerr << stat << std::endl;
      return 1;
    }
    
    ReadStdin reader(&socket);
    gcore::Thread thr(&reader, METHOD(ReadStdin, run), true);
    
    std::cout << "Type 'QUIT' to exit." << std::endl;
    
    while (thr.running()) {
      char *buffer = 0;
      size_t len = 0;
      
      gcore::TCPConnection *conn = socket.accept(&stat);
      if (!conn) {
        std::cerr << "Server error: " << stat << std::endl;
        continue;
      }
      
      //if (conn->read(buffer, len, -1, &stat)) {
      if (conn->readUntil(" ", buffer, len, -1, &stat)) {
        if (buffer) {
          std::cout << "\"" << buffer << "\"" << std::endl;
        } else {
          std::cout << "<null>" << std::endl;
        }
      } else if (!stat) {
        std::cout << "Client error: " << stat << std::endl;
      } else {
        if (buffer) {
          std::cout << "[partial] \"" << buffer << "\"" << std::endl;
        }
      }
      
      conn->free(buffer);
      
      socket.close(conn);
    }
    
    thr.join();
  }
  
  return 0;
}

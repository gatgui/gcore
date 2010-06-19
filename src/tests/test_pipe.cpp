/*

Copyright (C) 2009  Gaetan Guidet

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

#include <gcore/process.h>
#include <gcore/threads.h>
#include <string>
#include <iostream>
using namespace std;
using namespace gcore;

class PipeReader {
  public:
    
    PipeReader(Process *p)
      : m_p(p), m_prompt("A stupid prompt> ") {
    }
    
    ~PipeReader() {
    }
    
    int read() {
      string rbuf = "";
      while (m_p->read(rbuf) >= 0) {
        cout << endl << "### Read from pipe START" << endl;
        cout << rbuf;
        cout << endl << "### END";
      }
      return 0;
    }
    
    void done(int ecode) {
      cout << endl << "### Read pipe thread exited with code(" << ecode << ")" << endl;
    }

  protected:
    
    Process *m_p;
    string m_prompt;
};

void procOutput(const char *s) {
  cout << "Process Info: " << s;
}

int main(int argc, char **argv) {
  // get program as command iline
  bool in = false;
  bool out = true;
  
  if (argc < 2 || argc > 3) {
    cout << "Usage: test_pipe <program> <redirect_in=0|1>?" << endl;
    return -1;
  }
  if (argc == 3) {
    in = !strcmp(argv[2], "1");
  }
  
  char inb[1024];
  
  Process p;
  PipeReader r(&p);
  
  Thread *thr = 0;
  
  p.setOutputFunc(procOutput);
  p.verbose(true);
  p.setEnv("CUSTOM_ENV", "poo");
  p.captureOut(out);
  p.captureErr(out, false);
  p.redirectIn(in);
  p.run(argv[1], 0);
  
  if (out) {
    //t.start();
    thr = new Thread(&r, &PipeReader::read, &PipeReader::done);
  }
  
  if (in) {
    while (p.running()) {
      fgets(inb, 1024, stdin);
      p.write(inb);
    }
  }
  
  p.wait(true);

  if (thr) {
    thr->join();
    delete thr;
  }
  
  return 0;
}



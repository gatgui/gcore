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

#include <gcore/process.h>
#include <gcore/threads.h>
#include <gcore/argparser.h>
#include <string>
#include <iostream>

class PipeReader {
  public:
    
    PipeReader(gcore::Process *p)
      : m_p(p), m_prompt("A stupid prompt> ") {
    }
    
    ~PipeReader() {
    }
    
    int read() {
      gcore::String rbuf = "";
      //while (m_p->read(rbuf) >= 0) {
      int rv = m_p->read(rbuf);
      while (rv != 0) {
        if (rv > 0) {
          // could be an error
          std::cout << std::endl << "### Read from pipe START" << std::endl;
          std::cout << rbuf;
          std::cout << std::endl << "### END";
        } else {
          // some error
          std::cout << std::endl << "### Read from pipe ERROR" << std::endl;
        }
        rv = m_p->read(rbuf);
      }
      return 0;
    }
    
    void done(int ecode) {
      std::cout << std::endl << "### Read pipe thread exited with code(" << ecode << ")" << std::endl;
      m_p->wait(true);
    }

  protected:
    
    gcore::Process *m_p;
    gcore::String m_prompt;
};

void procOutput(const char *s) {
  std::cout << "Process Info: " << s;
}

gcore::FlagDesc args_desc[] = 
{
  { gcore::FlagDesc::FT_OPTIONAL, "captureOut", "co", 0 },
  { gcore::FlagDesc::FT_OPTIONAL, "captureErr", "ce", 0 },
  { gcore::FlagDesc::FT_OPTIONAL, "errToOut",   "eo", 0 },
  { gcore::FlagDesc::FT_OPTIONAL, "redirectIn", "ri", 0 },
  { gcore::FlagDesc::FT_OPTIONAL, "verbose",    "v",  0 },
  ACCEPTS_NOFLAG_ARGUMENTS(1)
};

int main(int argc, char **argv) {
  // get program as command iline
  bool in = false;
  bool out = false;
  bool err = false;
  bool err2out = false;
  bool verbose = false;
  
  gcore::ArgParser args(args_desc, sizeof(args_desc)/sizeof(gcore::FlagDesc));
  
  try {
    args.parse(argc-1, argv+1);
    
  } catch (gcore::ArgParserError &e) {
    std::cerr << "Error while parsing arguments: " << e.what() << std::endl; 
    std::cout << "Usage: test_pipe (--captureOut/-co)? (--captureErr/-ce)? (--errToOut/-eo)? (--redirectIn/-ri)? (--verbose/-v)? <program>" << std::endl;
    return -1;
  }
  
  gcore::String prog;
  args.getArgument(0, prog);
  in = args.isFlagSet("redirectIn");
  out = args.isFlagSet("captureOut");
  err = args.isFlagSet("captureErr");
  err2out = args.isFlagSet("errToOut");
  verbose = args.isFlagSet("verbose");
  
  char inb[1024];
  
  gcore::Process p;
  PipeReader r(&p);
  
  gcore::Thread *thr = 0;
  
  p.setOutputFunc(procOutput);
  p.verbose(verbose);
  p.setEnv("CUSTOM_ENV", "poo");
  p.captureOut(out);
  p.captureErr(err, err2out);
  p.redirectIn(in);
  p.run(prog, 0);
  
  if (out) {
    thr = new gcore::Thread(&r, &PipeReader::read, &PipeReader::done);
  }
  
  if (in) {
    while (p.running()) {
      fgets(inb, 1024, stdin);
      p.write(inb);
    }
  }
  
  //p.wait(true);

  if (thr) {
    thr->join();
    delete thr;
  }
  
  return 0;
}



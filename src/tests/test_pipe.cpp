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

class PipeReader
{
public:
   
   PipeReader(gcore::Process *p)
      : m_p(p)
      , m_prompt("A stupid prompt> ")
   {
   }
   
   ~PipeReader()
   {
   }
   
   int read()
   {
      gcore::Status stat;
      char rbuf[256];
      int rv = m_p->readOut(rbuf, 256, &stat);
      while (stat.succeeded() && rv > 0)
      {
         std::cout << std::endl << "### Read from pipe START" << std::endl;
         std::cout << rbuf;
         std::cout << std::endl << "### END";
         rv = m_p->readOut(rbuf, 256, &stat);
      }
      if (!stat)
      {
         std::cerr << stat.message() << std::endl;
      }
      return 0;
   }
   
   void done(int ecode)
   {
      std::cout << std::endl << "### Read pipe thread exited with code(" << ecode << ")" << std::endl;
      m_p->wait(true);
   }

protected:
   
   gcore::Process *m_p;
   gcore::String m_prompt;
};

gcore::FlagDesc args_desc[] = 
{
   { gcore::FlagDesc::FT_OPTIONAL, "redirectOut", "ro", 0 },
   { gcore::FlagDesc::FT_OPTIONAL, "redirectErr", "re", 0 },
   { gcore::FlagDesc::FT_OPTIONAL, "errToOut",   "eo", 0 },
   { gcore::FlagDesc::FT_OPTIONAL, "redirectIn", "ri", 0 },
   ACCEPTS_NOFLAG_ARGUMENTS(1)
};

int main(int argc, char **argv)
{
   // get program as command iline
   bool in = false;
   bool out = false;
   bool err = false;
   bool err2out = false;
   
   gcore::ArgParser args(args_desc, sizeof(args_desc)/sizeof(gcore::FlagDesc));
   
   gcore::Status stat = args.parse(argc-1, argv+1);
   if (!stat)
   {
      std::cerr << "Error while parsing arguments: " << stat << std::endl; 
      std::cout << "Usage: test_pipe (--redirectOut/-ro)? (--redirectErr/-re)? (--errToOut/-eo)? (--redirectIn/-ri)? <program>" << std::endl;
      return -1;
   }
   
   gcore::String prog;
   args.getArgument(0, prog);
   in = args.isFlagSet("redirectIn");
   out = args.isFlagSet("redirectOut");
   err = args.isFlagSet("redirectErr");
   err2out = args.isFlagSet("errToOut");
   
   char inb[1024];
   
   gcore::Process::Options opts;
   
   gcore::Process::SetDefaultOptions(opts);
   opts.redirectOut = out;
   opts.redirectErr = err;
   opts.redirectErrToOut = err2out;
   opts.redirectIn = in;
   opts.env["CUSTOM_ENV"] = "poo";
   
   /*
   gcore::Process p;
   p.setEnv("CUSTOM_ENV", "poo");
   p.setRedirectOut(out);
   p.setRedirectErr(err);
   p.setRedirectErrToOut(err2out);
   p.setRedirectIn(in);
   stat = p.run(prog);
   */
   gcore::Process p(prog, &opts, &stat);
   
   if (!stat)
   {
      std::cerr << stat << std::endl;
      exit(1);
   }
   
   PipeReader r(&p);
   gcore::Thread *thr = 0;
   
   if (out)
   {
      thr = new gcore::Thread(&r, &PipeReader::read, &PipeReader::done);
   }
   
   if (in)
   {
      while (p.isRunning())
      {
         fgets(inb, 1024, stdin);
         p.write(inb);
      }
   }
   
   p.wait(true);
   
   std::cout << "Process exit(" << p.returnCode() << ")" << std::endl;

   if (thr)
   {
      thr->join();
      delete thr;
   }
   
   return 0;
}



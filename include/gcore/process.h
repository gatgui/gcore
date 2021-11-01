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

#ifndef __gcore_process_h_
#define __gcore_process_h_

#include <gcore/pipe.h>
#include <gcore/env.h>

namespace gcore
{

#ifdef _WIN32
   
   typedef unsigned long ProcessID;
   const ProcessID INVALID_PID = 0;
   
   inline bool IsValidProcessID(ProcessID pid)
   {
      return (pid != 0);
   }
   
#else
   
   typedef int ProcessID;
   const ProcessID INVALID_PID = -1;
   
   inline bool IsValidProcessID(ProcessID pid)
   {
      return (pid > 0);
   }
   
#endif

   class GCORE_API Process
   {
   public:
         
      struct GCORE_API Options
      {
         bool redirectOut;
         bool redirectErr;
         bool redirectErrToOut;
         bool redirectIn;
         bool showConsole;
         bool keepAlive;
         Env::Dict env;
      };
      
      static void SetDefaultOptions(Options &opts);
      static bool EnquoteString(const String &in, String &out);
      
   public:
   
      Process();
      Process(const char *cmdline, Options *options=0, Status *status=0);
      Process(int argc, const char **argv, Options *options=0, Status *status=0);
      Process(const String &cmdline, Options *options=0, Status *status=0);
      Process(const StringList &args, Options *options=0, Status *status=0);
      ~Process();

      // Process setup
      
      void setEnv(const String &key, const String &value);
      
      const Options& options() const;
      void setOptions(const Options &options);
      
      void setRedirectOut(bool ro);
      bool redirectOut() const;

      void setRedirectErr(bool re);
      bool redirectErr() const;
      
      void setRedirectErrToOut(bool e2o);
      bool redirectErrToOut() const;

      void setRedirectIn(bool ri);
      bool redirectIn() const;

      // windows only
      void setShowConsole(bool sc);
      bool showConsole() const;

      // don't kill process in Process object destructor
      void setKeepAlive(bool ka);
      bool keepAlive() const;

      // Running process 
      
      Status run(const char *cmdline);
      Status run(int argc, const char **args);
      Status run(int argc, ...);
      Status run(const String &cmdline);
      Status run(const StringList &args);

      ProcessID id() const;

      bool isRunning();
      // Returns -1 on failure, 0 if process is still running (non-blocking mode) or 1 if process completed
      // In blocking mode, will wait for process completion
      int wait(bool blocking, Status *status=0);
      // Returns -1 on failure
      Status kill();

      inline const String& cmdLine() const { return mCmdLine; }
      // return code is initialized to -1
      inline int returnCode() const { return mReturnCode; }

      // Interacting with running process

      // Returns -1 on error, read/written bytes otherwise
      bool canReadOut() const;
      int readOut(char *buffer, int size, Status *status=0) const;
      bool canReadErr() const;
      int readErr(char *buffer, int size, Status *status=0) const;
      bool canWriteIn() const;
      int write(const char *buffer, int size, Status *status=0) const;
      int write(const String &str, Status *status=0) const;

      PipeID readOutID() const;
      PipeID readErrID() const;
      PipeID writeID() const;

   private:

      int waitNoClose(bool blocking, Status *status=0);
      Status run(int argc, va_list va);
      Status run();
      void closePipes();

   private:

      StringList  mArgs;
      ProcessID   mPID;
      Options     mOpts;
      Pipe        mReadOutPipe;
      Pipe        mReadErrPipe;
      Pipe        mWritePipe;
      char**      mStdArgs;
      String      mCmdLine;
      int         mReturnCode;
   };
}

#endif


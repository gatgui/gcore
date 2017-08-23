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

#ifndef __gcore_process_h_
#define __gcore_process_h_

#include <gcore/pipe.h>

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
         StringDict env;
      };
      
      static void SetDefaultOptions(Options &opts);
      
   public:
   
      Process();
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
      int readOut(void *buffer, int size, Status *status=0) const;
      bool canReadErr() const;
      int readErr(void *buffer, int size, Status *status=0) const;
      bool canWriteIn() const;
      int write(const void *buffer, int size, Status *status=0) const;
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


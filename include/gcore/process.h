/*

Copyright (C) 2009, 2010  Gaetan Guidet

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
  
  inline bool IsValidProcessID(ProcessID pid) {
    return (pid != 0);
  }
  
#else
  
  typedef int ProcessID;
  const ProcessID INVALID_PID = -1;
  
  inline bool IsValidProcessID(ProcessID pid) {
    return (pid > 0);
  }
  
#endif

  class GCORE_API Process {
    
    public:
    
      typedef void (*OutputFunc)(const char*);

      Process();
      ~Process();

      // Process setup
      
      void setOutputFunc(OutputFunc of);
      void setEnv(const String &key, const String &value);
      
      void captureOut(bool co);
      bool captureOut() const;

      void captureErr(bool enable, bool errToOut=false);
      bool captureErr() const;
      bool redirectErrToOut() const;

      void redirectIn(bool ri);
      bool redirectIn() const;

      // for windows
      void showConsole(bool sc);
      bool showConsole() const;

      void keepAlive(bool ka);
      bool keepAlive() const;

      void verbose(bool v);
      bool verbose() const;

      // Running process 
      
      // Returns INVALID_PID on failure
      ProcessID run(const String &cmdline);
      ProcessID run(const String &progPath, char **argv);
      ProcessID run(const String &progPath, int argc, ...);

      ProcessID getId() const;

      bool running();
      // Returns -1 on failure, 0 if process is still running (non-blocking mode) or 1 if process completed
      // In blocking mode, will wait for process completion
      int wait(bool blocking);
      // Returns -1 on failure
      int kill();

      inline const String& getCmdLine() const { return mCmdLine; }
      // return code is initialized to -1
      inline int returnCode() const { return mReturnCode; }

      // Interacting with running process

      // Returns -1 on error, read/written bytes otherwise
      int read(char *buffer, int size) const;
      int read(String &str) const;
      int write(const char *buffer, int size) const;
      int write(const String &str) const;
      int readErr(String &str) const;
      int writeErr(const String &str) const;

      PipeID readID() const;
      PipeID writeID() const;


    private:

      static void std_output(const char*);

      int _wait(bool blocking);
      ProcessID run();
      void closePipes();

    private:

      StringList  mArgs;
      ProcessID   mPID;
      OutputFunc  mOutFunc;
      Pipe        mReadPipe;
      Pipe        mWritePipe;
      Pipe        mErrorPipe;
      bool        mCapture;
      bool        mRedirect;
      bool        mVerbose;
      bool        mShowConsole;
      char**      mStdArgs; // used on nix
      String      mCmdLine;
      bool        mCaptureErr;
      bool        mErrToOut;
      bool        mKeepAlive;
      StringDict  mEnv;
      int         mReturnCode;
  };
}

#endif


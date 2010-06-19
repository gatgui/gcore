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

      void setOutputFunc(OutputFunc of);
      void setEnv(const std::string &key, const std::string &value);

      ProcessID run(const std::string &progPath, char **argv);
      ProcessID run(const std::string &progPath, int argc, ...);
      
      ProcessID getId() const;

      int read(std::string &str) const;
      int write(const std::string &str) const;
      //int readErr(std::string &str) const;

      inline const std::string& getCmdLine() const {
        return mCmdLine;
      } 

      //what about err?
      void captureOut(bool co);
      bool captureOut() const;

      //void captureErr(bool yesno, bool toout=false);
      //bool captureErr() const;

      void redirectIn(bool ri);
      bool redirectIn() const;

      void verbose(bool v);
      bool verbose() const;

      bool running();
      int wait(bool blocking);
      int kill();

      // for windows
      void showConsole(bool sc);
      bool showConsole() const;

    private:

      static void std_output(const char*);

      ProcessID run();
      void closePipes();

    private:

      std::vector<std::string> mArgs;
      ProcessID                mPID;
      OutputFunc               mOutFunc;
      Pipe                     mReadPipe;
      Pipe                     mWritePipe;
      bool                     mCapture;
      bool                     mRedirect;
      bool                     mVerbose;
      bool                     mShowConsole;
      char**                   mStdArgs; // used on nix
      std::string              mCmdLine;

      std::map<std::string, std::string> mEnv;
  };
}

#endif


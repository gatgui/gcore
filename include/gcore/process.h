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
      void setEnv(const String &key, const String &value);

      ProcessID run(const String &cmdline);
      ProcessID run(const String &progPath, char **argv);
      ProcessID run(const String &progPath, int argc, ...);
      
      ProcessID getId() const;

      int read(char *buffer, int size) const;
      int read(String &str) const;
      int write(const char *buffer, int size) const;
      int write(const String &str) const;
      int readErr(String &str) const;
      int writeErr(const String &str) const;
      
      PipeID readID() const;
      PipeID writeID() const;
      
      inline const String& getCmdLine() const {
        return mCmdLine;
      } 

      //what about err?
      void captureOut(bool co);
      bool captureOut() const;

      void captureErr(bool enable, bool errToOut=false);
      bool captureErr() const;
      bool redirectErrToOut() const;

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

      void keepAlive(bool ka);
      bool keepAlive() const;

    private:

      static void std_output(const char*);

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
  };
}

#endif


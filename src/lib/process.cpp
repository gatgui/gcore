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
#include <gcore/platform.h>
#include <gcore/env.h>

//------------------------------------------------------------------------------

#ifdef _WIN32

static HANDLE _process_getHandle(gcore::ProcessID pid) {
  return OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
}

static void _process_closeHandle(HANDLE phdl) {
  if (phdl) {
    CloseHandle(phdl);
  }
}

struct SearchBatOrCom {
  gcore::String name;
  gcore::String outname;
  bool isbat;

  bool check(const gcore::Path &dir) {
    gcore::Path path(dir);

    path.push(name + ".bat");
    if (path.isFile()) {
      isbat = true;
      outname = path.pop();
      return false;
    }

    path.pop();
    path.push(name + ".com");
    if (path.isFile()) {
      isbat = false;
      outname = path.pop();
      return false;
    }

    path.pop();
    path.push(name + ".exe");
    if (path.isFile()) {
      isbat = false;
      // keep name as it is, but stop iterating
      outname = name;
      return false;
    }

    return true;
  }
};

// returns true when 's' has changed
static bool _addBatOrCom(gcore::String &s, bool &isBat) {
  gcore::Path path(s);

  gcore::String ext = path.getExtension();

  if (ext == "") {
    gcore::String dir = path.dirname('/');
    gcore::String name = path.basename();

    // check for a .bat or .com file first (takes into account current working directory)
    path.pop();
    path.push(name + ".bat");
    if (path.isFile()) {
      s += ".bat";
      isBat = true;
      return true;
    }

    path.pop();
    path.push(name + ".com");
    if (path.isFile()) {
      s += ".com";
      isBat = false;
      return true;
    }

    if (dir == "") {
      // look in PATH
      gcore::Env::EachInPathFunc func;
      SearchBatOrCom search;

      search.name = name;
      search.outname = "";
      search.isbat = false;

      gcore::Bind(&search, METHOD(SearchBatOrCom, check), func);

      gcore::Env::EachInPath("PATH", func);

      if (search.outname.length() > 0 && search.outname != search.name) {
        isBat = search.isbat;
        s += (isBat ? ".bat" : ".com");
        return true;
      }
    }
  } else {
    isBat = (ext.casecompare("bat") == 0);
  }

  return false;
}

#endif

//------------------------------------------------------------------------------

void gcore::Process::std_output(const char *str) {
  fprintf(stdout,"%s",str);
}

gcore::Process::Process()
  : mPID(INVALID_PID), mCapture(false), mRedirect(false),
    mVerbose(false), mShowConsole(true), mStdArgs(0), mCmdLine(""),
    mCaptureErr(false), mErrToOut(false), mKeepAlive(false) {
  mOutFunc = &std_output;
}

gcore::Process::~Process() {
  closePipes();
  if (!mKeepAlive && running()) {
    kill();
  }
  mArgs.clear();
  if (mStdArgs) {
    delete[] mStdArgs;
  }
}

void gcore::Process::closePipes() {
  mReadPipe.close();
  mWritePipe.close();
  mErrorPipe.close();
}

int gcore::Process::read(char *buffer, int size) const {
  if (mReadPipe.canRead()) {
    return mReadPipe.read(buffer, size);
  }
  return -1;
}

int gcore::Process::read(String &str) const {
  if (mReadPipe.canRead()) {
    return mReadPipe.read(str);
  }
  return -1;
}

int gcore::Process::write(const char *buffer, int size) const {
  if (mWritePipe.canWrite()) {
    return mWritePipe.write(buffer, size);
  }
  return -1;
}

int gcore::Process::write(const String &str) const {
  if (mWritePipe.canWrite()) {
    return mWritePipe.write(str);
  }
  return -1;
}

int gcore::Process::readErr(String &str) const {
  if (mErrorPipe.canRead()) {
    return mErrorPipe.read(str);
  }
  return -1;
}

int gcore::Process::writeErr(const String &str) const {
  if (mErrorPipe.canWrite()) {
    return mErrorPipe.write(str);
  }
  return -1;
}

gcore::PipeID gcore::Process::readID() const {
  if (mReadPipe.canRead()) {
    return mReadPipe.readID();
  } else {
    return gcore::INVALID_PIPE;
  }
}

gcore::PipeID gcore::Process::writeID() const {
  if (mWritePipe.canWrite()) {
    return mWritePipe.writeID();
  } else {
    return gcore::INVALID_PIPE;
  }
}

gcore::ProcessID gcore::Process::getId() const {
  return mPID;
}

void gcore::Process::setOutputFunc(gcore::Process::OutputFunc of) {
  mOutFunc = of;
  if (! mOutFunc) {
    mVerbose = false;
  }
}

void gcore::Process::setEnv(const String &key, const String &value) {
  mEnv[key] = value;
}

bool gcore::Process::running() {
  if (IsValidProcessID(mPID)) {
    return (wait(false) == 0);
  } else {
    return false;
  }
}

void gcore::Process::keepAlive(bool ka) {
  mKeepAlive = ka;
}

bool gcore::Process::keepAlive() const {
  return mKeepAlive;
}

void gcore::Process::captureOut(bool co) {
  mCapture = co;
}

bool gcore::Process::captureOut() const {
  return mCapture;
}

void gcore::Process::captureErr(bool ce, bool e2o) {
  mCaptureErr = ce;
  mErrToOut = e2o;
}

bool gcore::Process::captureErr() const {
  return mCaptureErr;
}

bool gcore::Process::redirectErrToOut() const {
  return mErrToOut;
}

void gcore::Process::redirectIn(bool ri) {
  mRedirect = ri;
}

bool gcore::Process::redirectIn() const {
  return mRedirect;
}

void gcore::Process::verbose(bool v) {
  mVerbose = v;
}

bool gcore::Process::verbose() const {
  return mVerbose;
}

void gcore::Process::showConsole(bool sc) {
  mShowConsole = sc;
}

bool gcore::Process::showConsole() const {
  return mShowConsole;
}

gcore::ProcessID gcore::Process::run() {
  
  if (running() || mArgs.size() < 1) {
    return INVALID_PID;
  }

  // m_args[0] must contain program path

  // Build command line
  size_t i = 0;
  
#ifdef _WIN32
  // on windows, we need to fully specify extension for non .exe files
  bool isBat = false;
  _addBatOrCom(mArgs[0], isBat);
#endif

  mCmdLine = mArgs[i++];

  while (i < mArgs.size()) {
    mCmdLine += " " + mArgs[i++];
  }
  
#ifndef _WIN32

  // Build arguments list
  if (mStdArgs) {
    delete[] mStdArgs;
  }

  mStdArgs = new char*[mArgs.size()+1];

  for (i=0; i<mArgs.size(); ++i) {
    mStdArgs[i] = (char*)(mArgs[i].c_str());
  }
  
  mStdArgs[mArgs.size()] = 0;

  // Spawn new process
  Pipe iPipe;
  Pipe oPipe;
  Pipe ePipe;
  
  iPipe.create();
  oPipe.create();
  ePipe.create();
  
  mPID = fork();
  
  if (mPID == 0) {
    // Child process
    
    if (mCapture || (mCaptureErr && mErrToOut)) {
      oPipe.closeRead();
      mWritePipe = oPipe;
      if (mCapture) {
        dup2(mWritePipe.writeID(), 1);
        // writing to stdout will write to mWritePipe
      }
      
    } else {
      oPipe.close();
    }
    
    if (mCaptureErr) {
      if (mErrToOut) {
        ePipe.close();
        dup2(mWritePipe.writeID(), 2);
        // writing to stderr will write to mWritePipe
      
      } else {
        ePipe.closeRead();
        mErrorPipe = ePipe;
        dup2(mErrorPipe.writeID(), 2);
        // writing to stderr will write to mErrorPipe
      }
    } else {
      ePipe.close();
    }
    
    if (mRedirect) {
      iPipe.closeWrite();
      mReadPipe = iPipe;
      dup2(mReadPipe.readID(), 0);
      // reading from stdin will read from this pipe
    } else {
      iPipe.close();
    }
    
    Env::SetAll(mEnv, true);
    
    int failed = execvp(mArgs[0].c_str(), mStdArgs);
    
    // execution done... callback with error code ?
    //
    // if (mCallback) {
    //   mCallback(failed);
    // }
    
    if (failed == -1) {
      exit(-1);
    }
  
  } else {
    // Parent process
    
    if (mCapture || (mCaptureErr && mErrToOut)) {
      oPipe.closeWrite();
      mReadPipe = oPipe;
      // reading from this pipe is reading from child stdout
    } else {
      oPipe.close();
    }

    if (mCaptureErr && !mErrToOut) {
      ePipe.closeWrite();
      mErrorPipe = ePipe;
      // reading from this pipe is reading from child stderr
    } else {
      ePipe.close();
    }

    if (mRedirect) {
      iPipe.closeRead();
      mWritePipe = iPipe;
      // riting to this pipe is writing to child stdin
    } else {
      iPipe.close();
    }
  }

  return mPID;

#else

  // Spawn new process
  mPID = 0;
  
  PROCESS_INFORMATION pinfo;
  STARTUPINFO sinfo;
  
  ZeroMemory(&sinfo, sizeof(sinfo));
  
  Pipe inPipe;
  Pipe outPipe;
  Pipe errPipe;
  
  sinfo.cb = sizeof(STARTUPINFO);
  sinfo.dwFlags = STARTF_USESHOWWINDOW;
  
  // Child STDIN pipe [if redirect only] --> parent write to this pipe
  if (mRedirect) {
    inPipe.create();
    SetHandleInformation(inPipe.writeID(), HANDLE_FLAG_INHERIT, 0);
    sinfo.hStdInput = inPipe.readID();
    mWritePipe = inPipe;
    sinfo.dwFlags = sinfo.dwFlags | STARTF_USESTDHANDLES;
  } else {
    sinfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  }
  
  // Child STDOUT pipe [if capture only] --> parent read on this pipe
  //if (mCapture) {
  if (mCapture || (mCaptureErr && mErrToOut)) {
    outPipe.create();
    SetHandleInformation(outPipe.readID(), HANDLE_FLAG_INHERIT, 0);
    if (mCapture) {
      sinfo.hStdOutput = outPipe.writeID();
    }
    //sinfo.hStdError = outPipe.writeID();
    //sinfo.hStdOutput = outPipe.writeID();
    mReadPipe = outPipe;
    sinfo.dwFlags = sinfo.dwFlags | STARTF_USESTDHANDLES;
  } else {
    //sinfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    sinfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  }
  
  if (mCaptureErr) {
    if (mErrToOut) {
      sinfo.hStdError = outPipe.writeID();
    } else {
      errPipe.create();
      SetHandleInformation(errPipe.readID(), HANDLE_FLAG_INHERIT, 0);
      sinfo.hStdError = errPipe.writeID();
      mErrorPipe = errPipe;
    }
    sinfo.dwFlags = sinfo.dwFlags | STARTF_USESTDHANDLES;
  } else {
    sinfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  }
  
  // this is to hide console if requested
  sinfo.wShowWindow = (mShowConsole ? SW_SHOW : SW_HIDE);
  
  // on windows, CreateProcess will do the fork internally
  // need to setup the env before
  // OR find the equivalent of fork/exec 
  Env env;
  
  // backup current environment
  env.push();
  
  // modify environment
  env.setAll(mEnv, true);

  if (isBat) {
    // for .bat files, we need to prepend cmd.exe /c
    mCmdLine = "cmd.exe /c " + mCmdLine;
  }
  
  // Don't try to inheritHandles when using .bat files
  if (CreateProcess(NULL, (char*)mCmdLine.c_str(), NULL, NULL,
                    (isBat ? FALSE : TRUE), 0, 0, NULL, &sinfo, &pinfo)) {
    // In parent only
    
    mPID = pinfo.dwProcessId;
    
    if (mCapture || (mCaptureErr && mErrToOut)) {
      outPipe.closeWrite();
    }
    
    if (mCaptureErr && !mErrToOut) {
      errPipe.closeWrite();
    }
    
    if (mRedirect) {
      inPipe.closeRead();
    }
    
    // restore environment
    env.pop();
    
  } else {
    mPID = INVALID_PID;
    closePipes();
  }
  return mPID;

#endif

}

gcore::ProcessID gcore::Process::run(const gcore::String &cmdline) {
  gcore::String tmp = cmdline;
  bool inSingleQuote = false;
  bool inDoubleQuote = false;
  for (size_t i=0; i<tmp.length(); ++i)
  {
    if (tmp[i] == '\"')
    {
      if (i == 0 || tmp[i-1] != '\\')
      {
        inDoubleQuote = !inDoubleQuote;
      }
    }
    else if (tmp[i] == '\'')
    {
      if (i == 0 || tmp[i-1] != '\\')
      {
        inSingleQuote = !inSingleQuote;
      }
    }
    else
    {
      if (tmp[i] == ' ' && !inSingleQuote && !inDoubleQuote)
      {
        tmp[i] = '\n';
      }
    }
  }
  
  mArgs.clear();
  tmp.split('\n', mArgs);
  
  size_t i = 0;
  while (i < mArgs.size())
  {
    if (mArgs[i].length() == 0)
    {
      mArgs.erase(mArgs.begin() + i);
      continue;
    }
    if (mArgs[i].length() >= 2 &&
        ((mArgs[i][0] == '\'' && mArgs[i][mArgs[i].length()-1] == '\'') ||
         (mArgs[i][0] == '\"' && mArgs[i][mArgs[i].length()-1] == '\"')))
    {
      mArgs[i] = mArgs[i].substr(1, mArgs[i].length()-2);
    }
    ++i;
  }
  
  return run();
}

gcore::ProcessID gcore::Process::run(const String &progPath, char **argv) {
  mArgs.clear();
  mArgs.push(progPath);
  char **carg = argv;
  while (*carg) {
    String argi = *carg;
    mArgs.push(argi);
    carg++;
  }
  return run();
}

gcore::ProcessID gcore::Process::run(const String &progPath, int argc, ...) {
  mArgs.clear();
  mArgs.push(progPath);
  va_list va;
  va_start(va, argc);
  for (int i=0; i<argc; ++i) {
    String argi = va_arg(va,char*);
    mArgs.push(argi);
  }
  va_end(va);
  return run();
}

int gcore::Process::wait(bool blocking) {
  static char mess[64] = {0};

  if (! IsValidProcessID(mPID)) {
    return -1;
  }

#ifndef _WIN32  

  int status;
  
  ProcessID rpid = waitpid(mPID, &status, (blocking ? 0 : WNOHANG));
  
  if (!blocking && rpid==0) {
    if (mVerbose) {
      sprintf(mess,"Wait(%d): Process still running\n", mPID);
      (*mOutFunc)(mess);
    }
    return 0;
  }
  if (rpid == mPID) {
    if (mVerbose) {
      if (WIFEXITED(status)) {
        int ecode = WEXITSTATUS(status);
        sprintf(mess,"Wait(%d): Process exited with status %d\n", mPID, ecode);
      } else if (WIFSIGNALED(status)) {
        int scode = WTERMSIG(status);
        sprintf(mess,"Wait(%d): Process exited on signal %d\n", mPID, scode);
      } else if (WIFSTOPPED(status)) {
        int scode = WSTOPSIG(status);
        sprintf(mess,"Wait(%d): Process stopped on signal %d\n", mPID, scode);
      } else {
        sprintf(mess,"Wait(%d): Process terminated\n", mPID);
      }
      (*mOutFunc)(mess);
    }
    closePipes();
    mPID = INVALID_PID;
    return 1;
  } else {
    if (mVerbose) {
      switch(errno) {
        case ECHILD:
          sprintf(mess,"Wait(%d): Process does not exist\n", mPID); break;
        case EFAULT:
          sprintf(mess,"Wait(%d): Invalid status\n", mPID); break;
        case EINTR:
          sprintf(mess,"Wait(%d): Process aborted\n", mPID); break;
        case EINVAL:
          sprintf(mess,"Wait(%d): Invalid options\n", mPID); break;
        default:
          sprintf(mess,"Wait(%d): Unknown error\n", mPID);
      }
      (*mOutFunc)(mess);
    }
    closePipes();
    mPID = INVALID_PID;
    return -1;
  }
  
#else

  HANDLE phdl = _process_getHandle(mPID);

  if (phdl) {
    DWORD ret = WaitForSingleObject(phdl, (blocking ? INFINITE : 5));
    if (ret == WAIT_TIMEOUT) {
      if (mVerbose) {
        sprintf(mess,"Wait(%ld): Process still running\n", (long)mPID);
        (*mOutFunc)(mess);
      }
      _process_closeHandle(phdl);
      return 0;
    } else if (ret == WAIT_OBJECT_0) {
      if (mVerbose) {
        sprintf(mess,"Wait(%ld): Process terminated\n", (long)mPID);
        (*mOutFunc)(mess);
      }
      _process_closeHandle(phdl);
      closePipes();
      mPID = INVALID_PID;
      return 1;
    } else {
      _process_closeHandle(phdl);
      closePipes();
      mPID = INVALID_PID;
    }
  }
  if (mVerbose) {
    sprintf(mess,"Wait(%ld): Error\n", (long)mPID);
    (*mOutFunc)(mess);
    /*
    DWORD err = GetLastError();
    FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM,
      &err, 0, 0,
      mess, 64, 0);
    (*m_outFunc)(mess);
    */
  }
  return -1;
  
#endif
}

int gcore::Process::kill() {
  if (! IsValidProcessID(mPID)) {
    return -1;
  }
#ifndef _WIN32
  int r = ::kill(mPID,SIGQUIT);
  if (0 == r) {
    if (mVerbose) {
      (*mOutFunc)("Kill success\n");
    }
    closePipes();
    mPID = INVALID_PID;
    return 0;
  } else {
    if (mVerbose) {
      switch(errno) {
        case EINVAL:
          (*mOutFunc)("Kill failed: Invalid signal\n"); break;
        case EPERM:
          (*mOutFunc)("Kill failed: No permission\n"); break;
        case ESRCH:
          (*mOutFunc)("Kill failed: Process does not exist\n"); break;
        default:
          (*mOutFunc)("Kill failed: Unknown reason\n");
      };
    }
    closePipes();
    mPID = INVALID_PID;
    return -1;
  }

#else
  
  HANDLE phdl = _process_getHandle(mPID);
  if (phdl) {
    if (TerminateProcess(phdl,0)) {
      if (mVerbose) {
        mPID = INVALID_PID;
        (*mOutFunc)("Kill success\n");
      }
      mWritePipe.write("exit\r\n");
      WaitForSingleObject(phdl, 10000);
      closePipes();
      mPID = INVALID_PID;
      return 0;
    }
    _process_closeHandle(phdl);
  }
  if (mVerbose) {
    (*mOutFunc)("Kill failed\n");
  }
  closePipes();
  mPID = INVALID_PID;
  return -1;

#endif
}



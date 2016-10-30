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

#include <gcore/process.h>
#include <gcore/platform.h>
#include <gcore/env.h>
#include <gcore/encoding.h>

//------------------------------------------------------------------------------

#ifdef _WIN32

static HANDLE _process_getHandle(gcore::ProcessID pid)
{
   return OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
}

static void _process_closeHandle(HANDLE phdl)
{
   if (phdl)
   {
      CloseHandle(phdl);
   }
}

struct SearchBatOrCom
{
   gcore::String name;
   gcore::String outname;
   bool isbat;

   bool check(const gcore::Path &dir)
   {
      gcore::Path path(dir);

      path.push(name + ".bat");
      if (path.isFile())
      {
         isbat = true;
         outname = path.pop();
         return false;
      }

      path.pop();
      path.push(name + ".com");
      if (path.isFile())
      {
         isbat = false;
         outname = path.pop();
         return false;
      }

      path.pop();
      path.push(name + ".exe");
      if (path.isFile())
      {
         isbat = false;
         // keep name as it is, but stop iterating
         outname = name;
         return false;
      }

      return true;
   }
};

// returns true when 's' has changed
static bool _addBatOrCom(gcore::String &s, bool &isBat)
{
   gcore::Path path(s);

   gcore::String ext = path.extension();

   if (ext == "")
   {
      gcore::String dir = path.dirname('/');
      gcore::String name = path.basename();

      // check for a .bat or .com file first (takes into account current working directory)
      path.pop();
      path.push(name + ".bat");
      if (path.isFile())
      {
         s += ".bat";
         isBat = true;
         return true;
      }

      path.pop();
      path.push(name + ".com");
      if (path.isFile())
      {
         s += ".com";
         isBat = false;
         return true;
      }

      if (dir == "")
      {
         // look in PATH
         gcore::Env::ForEachInPathFunc func;
         SearchBatOrCom search;

         search.name = name;
         search.outname = "";
         search.isbat = false;

         gcore::Bind(&search, METHOD(SearchBatOrCom, check), func);

         gcore::Env::ForEachInPath("PATH", func);

         if (search.outname.length() > 0 && search.outname != search.name)
         {
            isBat = search.isbat;
            s += (isBat ? ".bat" : ".com");
            return true;
         }
      }
   }
   else
   {
      isBat = (ext.casecompare("bat") == 0);
   }

   return false;
}

#endif

//------------------------------------------------------------------------------

namespace gcore
{

void Process::SetDefaultOptions(Process::Options &opts)
{
   opts.redirectOut = false;
   opts.redirectErr = false;
   opts.redirectErrToOut = false;
   opts.redirectIn = false;
   opts.showConsole = true;
   opts.keepAlive = false;
   opts.env.clear();
}

Process::Process()
   : mPID(INVALID_PID)
   , mStdArgs(0)
   , mCmdLine("")
   , mReturnCode(-1)
{
   SetDefaultOptions(mOpts);
}

Process::Process(const String &cmdline, Process::Options *opts, Status *status)
   : mPID(INVALID_PID)
   , mStdArgs(0)
   , mCmdLine("")
   , mReturnCode(-1)
{
   if (opts)
   {
      mOpts = *opts;
   }
   else
   {
      SetDefaultOptions(mOpts);
   }
   Status stat = run(cmdline);
   if (status)
   {
      *status = stat;
   }
}

Process::Process(const StringList &args, Process::Options *opts, Status *status)
   : mPID(INVALID_PID)
   , mStdArgs(0)
   , mCmdLine("")
   , mReturnCode(-1)
{
   if (opts)
   {
      mOpts = *opts;
   }
   else
   {
      SetDefaultOptions(mOpts);
   }
   Status stat = run(args);
   if (status)
   {
      *status = stat;
   }
}

Process::~Process()
{
   closePipes();
   
   if (!mOpts.keepAlive && isRunning())
   {
      kill();
   }
   
   mArgs.clear();
   if (mStdArgs)
   {
      delete[] mStdArgs;
   }
}

void Process::closePipes()
{
   mReadOutPipe.close();
   mReadErrPipe.close();
   mWritePipe.close();
}

bool Process::canReadOut() const
{
   return mReadOutPipe.canRead();
}

int Process::readOut(void *buffer, int size, Status *status) const
{
   return mReadOutPipe.read(buffer, size, status);
}

bool Process::canReadErr() const
{
   return mReadErrPipe.canRead();
}

int Process::readErr(void *buffer, int size, Status *status) const
{
   return mReadErrPipe.read(buffer, size, status);
}

bool Process::canWriteIn() const
{
   return mWritePipe.canWrite();
}

int Process::write(const void *buffer, int size, Status *status) const
{
   return mWritePipe.write(buffer, size, status);
}

int Process::write(const String &str, Status *status) const
{
   return mWritePipe.write(str, status);
}

PipeID Process::readOutID() const
{
   if (mReadOutPipe.canRead())
   {
      return mReadOutPipe.readID();
   }
   else
   {
      return INVALID_PIPE;
   }
}

PipeID Process::readErrID() const
{
   if (mReadErrPipe.canRead())
   {
      return mReadErrPipe.readID();
   }
   else
   {
      return INVALID_PIPE;
   }
}

PipeID Process::writeID() const
{
   if (mWritePipe.canWrite())
   {
      return mWritePipe.writeID();
   }
   else
   {
      return INVALID_PIPE;
   }
}

ProcessID Process::id() const
{
   return mPID;
}

const Process::Options& Process::options() const
{
   return mOpts;
}

void Process::setOptions(const Process::Options &options)
{
   mOpts = options;
}

void Process::setEnv(const String &key, const String &value)
{
   mOpts.env[key] = value;
}

bool Process::isRunning()
{
   if (IsValidProcessID(mPID))
   {
      return (waitNoClose(false) == 0);
   }
   else
   {
      return false;
   }
}

void Process::setKeepAlive(bool ka)
{
   mOpts.keepAlive = ka;
}

bool Process::keepAlive() const
{
   return mOpts.keepAlive;
}

void Process::setRedirectOut(bool ro)
{
   mOpts.redirectOut = ro;
}

bool Process::redirectOut() const
{
   return mOpts.redirectOut;
}

void Process::setRedirectErr(bool re)
{
   mOpts.redirectErr = re;
}

bool Process::redirectErr() const
{
   return mOpts.redirectErr;
}

void Process::setRedirectErrToOut(bool e2o)
{
   mOpts.redirectErrToOut = e2o;
}

bool Process::redirectErrToOut() const
{
   return mOpts.redirectErrToOut;
}

void Process::setRedirectIn(bool ri)
{
   mOpts.redirectIn = ri;
}

bool Process::redirectIn() const
{
   return mOpts.redirectIn;
}

void Process::setShowConsole(bool sc)
{
   mOpts.showConsole = sc;
}

bool Process::showConsole() const
{
   return mOpts.showConsole;
}

Status Process::run()
{
   if (isRunning())
   {
      return Status(false, "gcore::Process::run: Process already running.");
   }
   
   if (mArgs.size() < 1)
   {
      mPID = INVALID_PID;
      return Status(false, "gcore::Process::run: Nothing to run.");
   }

   mReturnCode = -1;
   // m_args[0] must contain program path

   // Build command line
   size_t i = 0;
   
#ifdef _WIN32
   // on windows, we need to fully specify extension for non .exe files
   bool isBat = false;
   _addBatOrCom(mArgs[0], isBat);
#endif

   mCmdLine = mArgs[i++];

   while (i < mArgs.size())
   {
      mCmdLine += " " + mArgs[i++];
   }
   
#ifndef _WIN32

   // Build arguments list
   if (mStdArgs)
   {
      delete[] mStdArgs;
   }

   mStdArgs = new char*[mArgs.size()+1];

   for (i=0; i<mArgs.size(); ++i)
   {
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
   
   if (mPID == 0)
   {
      // Child process
      
      if (mOpts.redirectOut || (mOpts.redirectErr && mOpts.redirectErrToOut))
      {
         oPipe.closeRead();
         mWritePipe = oPipe;
         if (mOpts.redirectOut)
         {
            dup2(mWritePipe.writeID(), 1);
            // writing to stdout will write to mWritePipe
         }
      }
      else
      {
         oPipe.close();
      }
      
      if (mOpts.redirectErr)
      {
         if (mOpts.redirectErrToOut)
         {
            ePipe.close();
            dup2(mWritePipe.writeID(), 2);
            // writing to stderr will write to mWritePipe
         }
         else
         {
            ePipe.closeRead();
            mReadErrPipe = ePipe;
            dup2(mReadErrPipe.writeID(), 2);
            // writing to stderr will write to mReadErrPipe
         }
      }
      else
      {
         ePipe.close();
      }
      
      if (mOpts.redirectIn)
      {
         iPipe.closeWrite();
         mReadOutPipe = iPipe;
         dup2(mReadOutPipe.readID(), 0);
         // reading from stdin will read from this pipe
      }
      else
      {
         iPipe.close();
      }
      
      Env::Set(mOpts.env, true);
      
      int failed = execvp(mArgs[0].c_str(), mStdArgs);
      
      // execution done... callback with error code ?
      //
      // if (mCallback)
      // {
      //   mCallback(failed);
      // }
      
      if (failed == -1)
      {
         exit(-1);
      }
   }
   else
   {
      // Parent process
      
      if (mOpts.redirectOut || (mOpts.redirectErr && mOpts.redirectErrToOut))
      {
         oPipe.closeWrite();
         mReadOutPipe = oPipe;
         // reading from this pipe is reading from child stdout
      }
      else
      {
         oPipe.close();
      }

      if (mOpts.redirectErr && !mOpts.redirectErrToOut)
      {
         ePipe.closeWrite();
         mReadErrPipe = ePipe;
         // reading from this pipe is reading from child stderr
      }
      else
      {
         ePipe.close();
      }

      if (mOpts.redirectIn)
      {
         iPipe.closeRead();
         mWritePipe = iPipe;
         // riting to this pipe is writing to child stdin
      }
      else
      {
         iPipe.close();
      }
   }

   return Status(true);

#else

   // Spawn new process
   mPID = 0;
   
   PROCESS_INFORMATION pinfo;
   STARTUPINFOW sinfo;
   
   ZeroMemory(&sinfo, sizeof(sinfo));
   
   Pipe inPipe;
   Pipe outPipe;
   Pipe errPipe;
   
   sinfo.cb = sizeof(STARTUPINFO);
   sinfo.dwFlags = STARTF_USESHOWWINDOW;
   
   // Child STDIN pipe [if redirect only] --> parent write to this pipe
   if (mOpts.redirectIn)
   {
      inPipe.create();
      SetHandleInformation(inPipe.writeID(), HANDLE_FLAG_INHERIT, 0);
      sinfo.hStdInput = inPipe.readID();
      mWritePipe = inPipe;
      sinfo.dwFlags = sinfo.dwFlags | STARTF_USESTDHANDLES;
   }
   else
   {
      sinfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
   }
   
   // Child STDOUT pipe [if capture only] --> parent read on this pipe
   //if (mOpts.redirectOut) {
   if (mOpts.redirectOut || (mOpts.redirectErr && mOpts.redirectErrToOut))
   {
      outPipe.create();
      SetHandleInformation(outPipe.readID(), HANDLE_FLAG_INHERIT, 0);
      if (mOpts.redirectOut)
      {
         sinfo.hStdOutput = outPipe.writeID();
      }
      //sinfo.hStdError = outPipe.writeID();
      //sinfo.hStdOutput = outPipe.writeID();
      mReadOutPipe = outPipe;
      sinfo.dwFlags = sinfo.dwFlags | STARTF_USESTDHANDLES;
   }
   else
   {
      //sinfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
      sinfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
   }
   
   if (mOpts.redirectErr)
   {
      if (mOpts.redirectErrToOut)
      {
         sinfo.hStdError = outPipe.writeID();
      }
      else
      {
         errPipe.create();
         SetHandleInformation(errPipe.readID(), HANDLE_FLAG_INHERIT, 0);
         sinfo.hStdError = errPipe.writeID();
         mReadErrPipe = errPipe;
      }
      sinfo.dwFlags = sinfo.dwFlags | STARTF_USESTDHANDLES;
   }
   else
   {
      sinfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
   }
   
   // this is to hide console if requested
   sinfo.wShowWindow = (mOpts.showConsole ? SW_SHOW : SW_HIDE);
   
   // on windows, CreateProcess will do the fork internally
   // need to setup the env before
   // OR find the equivalent of fork/exec 
   Env env;
   
   // backup current environment
   env.push();
   
   // modify environment
   env.set(mOpts.env, true);

   if (isBat)
   {
      // for .bat files, we need to prepend cmd.exe /c
      mCmdLine = "cmd.exe /c " + mCmdLine;
   }
   
   // Don't try to inheritHandles when using .bat files
   std::wstring wcmd;
   ToWideString(UTF8Codepage, mCmdLine.c_str(), wcmd);
   if (CreateProcessW(NULL, (wchar_t*)wcmd.c_str(), NULL, NULL,
                      (isBat ? FALSE : TRUE), 0, 0, NULL, &sinfo, &pinfo))
   {
      // In parent only
      
      mPID = pinfo.dwProcessId;
      
      if (mOpts.redirectOut || (mOpts.redirectErr && mOpts.redirectErrToOut))
      {
         outPipe.closeWrite();
      }
      
      if (mOpts.redirectErr && !mOpts.redirectErrToOut)
      {
         errPipe.closeWrite();
      }
      
      if (mOpts.redirectIn)
      {
         inPipe.closeRead();
      }
      
      // restore environment
      env.pop();
      
      return Status(true);
   }
   else
   {
      mPID = INVALID_PID;
      closePipes();
      
      return Status(false, std_errno(), "gcore::Process::run");
   }

#endif

}

Status Process::run(const String &cmdline)
{
   if (cmdline.length() == 0)
   {
      return Status(false, "gcore::Process::run: Invalid command.");
   }
   
   String tmp = cmdline;
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

Status Process::run(const StringList &args)
{
   mArgs = args;
   return run();
}

int Process::wait(bool blocking, Status *status)
{
   int rv = waitNoClose(blocking, status);
   if (rv != 0)
   {
      closePipes();
      mPID = INVALID_PID;
   }
   return rv;
}

int Process::waitNoClose(bool blocking, Status *status)
{
   if (! IsValidProcessID(mPID))
   {
      if (status)
      {
         status->set(false, "gcore::Process::wait: Invalid PID.");
      }
      return -1;
   }

#ifndef _WIN32

   int st;
   
   ProcessID rpid = waitpid(mPID, &st, (blocking ? 0 : WNOHANG));
   
   if (!blocking && rpid == 0)
   {
      if (status)
      {
         status->set(true, "gcore::Process::wait: Process still running.");
      }
      return 0;
   }
   
   if (rpid == mPID)
   {
      if (WIFEXITED(st))
      {
         mReturnCode = WEXITSTATUS(st);
      }
      // else if (WIFSIGNALED(st))
      // {
      //   // Process exited on signal 
      //   int scode = WTERMSIG(st);
      //   if (WCOREDUMP(st))
      //   {
      //   }
      // }
      // else if (WIFSTOPPED(st))
      // {
      //   // Process stopped on signal 
      //   int scode = WSTOPSIG(st);
      // }
      
      if (status)
      {
         status->set(true, "gcore::Process::wait: Process exited.");
      }
      return 1;
   }
   else
   {
      // must be -1: error
      if (status)
      {
         status->set(false, std_errno(), "gcore::Process::wait");
      }
      return -1;
   }
   
#else

   HANDLE phdl = _process_getHandle(mPID);

   if (phdl)
   {
      DWORD ret = WaitForSingleObject(phdl, (blocking ? INFINITE : 5));
      
      if (ret == WAIT_TIMEOUT)
      {
         _process_closeHandle(phdl);
         if (status)
         {
            status->set(true, "gcore::Process::wait: Process still running.");
         }
         return 0;
      }
      else if (ret == WAIT_OBJECT_0)
      {
         DWORD rv;
         if (GetExitCodeProcess(phdl, &rv))
         {
            mReturnCode = (int) rv;
         }
         _process_closeHandle(phdl);
         if (status)
         {
            status->set(true, "gcore::Process::wait: Process exited.");
         }
         return 1;
      }
      else
      {
         _process_closeHandle(phdl);
         if (status)
         {
            status->set(false, std_errno(), "gcore::Process::wait");
         }
         return -1;
      }
   }
   
   if (status)
   {
      status->set(false, "gcore::Process::wait: Could not get process handle from PID.");
   }
   return -1;
   
#endif
}

Status Process::kill()
{
   if (! IsValidProcessID(mPID))
   {
      return Status(false, "gcore::Process::kill: Invalid PID.");
   }

#ifndef _WIN32

   int r = ::kill(mPID,SIGQUIT);
   
   closePipes();
   mPID = INVALID_PID;
   
   if (0 == r)
   {
      return Status(true);
   }
   else
   {
      return Status(false, std_errno(), "gcore::Process::kill");
   }

#else
   
   HANDLE phdl = _process_getHandle(mPID);
   if (phdl)
   {
      if (TerminateProcess(phdl, 0))
      {
         mWritePipe.write("exit\r\n");
         WaitForSingleObject(phdl, 10000);
         closePipes();
         mPID = INVALID_PID;
         return Status(true);
      }
      _process_closeHandle(phdl);
      return Status(false, std_errno(), "gcore::Process::kill");
   }
   closePipes();
   mPID = INVALID_PID;
   return Status(false, "gcore::Process::kill: Could not get process handle from PID.");

#endif
}

} // gcore



/*

Copyright (C) 2010~  Gaetan Guidet

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

#include <gcore/log.h>

#ifdef _WIN32

static WORD gsDefaultOutAttrs = 0;
static WORD gsDefaultErrAttrs = 0;
static bool gsDefaultAttrsSet = false;

#define TERM_COL_BLACK    0
#define TERM_COL_BLUE     1
#define TERM_COL_GREEN    2
#define TERM_COL_CYAN     3
#define TERM_COL_RED      4
#define TERM_COL_MAGENTA  5
#define TERM_COL_YELLOW   6
#define TERM_COL_WHITE    7

static void ChangeTermColors(int fg=-1, int bg=-1)
{
   HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
   HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
   
   if (!gsDefaultAttrsSet)
   {
      CONSOLE_SCREEN_BUFFER_INFO csbi;
      GetConsoleScreenBufferInfo(hStdOut, &csbi);
      gsDefaultOutAttrs = csbi.wAttributes;
      GetConsoleScreenBufferInfo(hStdErr, &csbi);
      gsDefaultErrAttrs = csbi.wAttributes;
      gsDefaultAttrsSet = true;
   }
   
   WORD attrs;
   
   attrs = gsDefaultOutAttrs;
   if (fg >= 0)
   {
      attrs = (WORD)((attrs & 0xF0) | fg);
   }
   if (bg >= 0)
   {
      attrs = (WORD)((attrs & 0x0F) | (bg << 4));
   }
   SetConsoleTextAttribute(hStdOut, attrs);
   
   attrs = gsDefaultErrAttrs;
   if (fg >= 0)
   {
      attrs = (WORD)((attrs & 0xF0) | fg);
   }
   if (bg >= 0)
   {
      attrs = (WORD)((attrs & 0x0F) | (bg << 4));
   }
   SetConsoleTextAttribute(hStdErr, attrs);
}

static void ResetTermColors()
{
   if (gsDefaultAttrsSet)
   {
      HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
      HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
      
      SetConsoleTextAttribute(hStdOut, gsDefaultOutAttrs);
      SetConsoleTextAttribute(hStdErr, gsDefaultErrAttrs);
   }
}

#else

#define TERM_CMD_RESET     0
#define TERM_CMD_BRIGHT    1
#define TERM_CMD_DIM       2
#define TERM_CMD_UNDERLINE 3
#define TERM_CMD_BLINK     4
#define TERM_CMD_REVERSE   7
#define TERM_CMD_HIDDEN    8

#define TERM_COL_BLACK     0
#define TERM_COL_RED       1
#define TERM_COL_GREEN     2
#define TERM_COL_YELLOW    3
#define TERM_COL_BLUE      4
#define TERM_COL_MAGENTA   5
#define TERM_COL_CYAN      6
#define TERM_COL_WHITE     7

#define TERM_COL_FGBASE    30
#define TERM_COL_BGBASE    40

static std::string MakeTermCode(int cmd, int fg, int bg)
{
   //char tmp[13];
   //sprintf(tmp, "%c[%d;%d;%dm", 0x1B, cmd, TERM_COL_FGBASE+fg, TERM_COL_BGBASE+bg);
   //return tmp;
   char tmp[16];
   
   std::string code;
   bool needSep = false;
   code = "\033[";
   if (cmd >= 0)
   {
      sprintf(tmp, "%d", cmd);
      code += tmp;
      needSep = true;
   }
   if (fg >= 0)
   {
      if (needSep)
      {
         code += ";";
      }
      sprintf(tmp, "%d", TERM_COL_FGBASE + fg);
      code += tmp;
      needSep = true;
   }
   if (bg >= 0)
   {
      if (needSep)
      {
         code += ";";
      }
      sprintf(tmp, "%d", TERM_COL_BGBASE + bg);
      code += tmp;
   }
   code += "m";
   return code;
}

#endif

static void PrintStdout(const char *msg)
{
   fprintf(stdout, "%s", msg);
}

namespace gcore
{

Log Log::msSharedLog;
char Log::msBuffer[2048];

Log& Log::Get()
{
   return msSharedLog;
}

void Log::SetLevelMask(unsigned int mask)
{
   msSharedLog.setLevelMask(mask);
}

unsigned int Log::LevelMask()
{
   return msSharedLog.levelMask();
}

void Log::SetOutputFunc(OutputFunc func)
{
   msSharedLog.setOutputFunc(func);
}

void Log::PrintError(const char *fmt, ...)
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(msBuffer, 2048, fmt, args);
   va_end(args);
   msSharedLog.print(LOG_ERROR, msBuffer);
}

void Log::PrintWarning(const char *fmt, ...)
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(msBuffer, 2048, fmt, args);
   va_end(args);
   msSharedLog.print(LOG_WARNING, msBuffer);
}

void Log::PrintDebug(const char *fmt, ...)
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(msBuffer, 2048, fmt, args);
   va_end(args);
   msSharedLog.print(LOG_DEBUG, msBuffer);
}

void Log::PrintInfo(const char *fmt, ...)
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(msBuffer, 2048, fmt, args);
   va_end(args);
   msSharedLog.print(LOG_INFO, msBuffer);
}

void Log::SetIndentLevel(unsigned int n)
{
   msSharedLog.setIndentLevel(n);
}

unsigned int Log::IndentLevel()
{
   return msSharedLog.indentLevel();
}

void Log::Indent()
{
   msSharedLog.indent();
}

void Log::UnIndent()
{
   msSharedLog.unIndent();
}

void Log::SetIndentWidth(unsigned int w)
{
   msSharedLog.setIndentWidth(w);
}

unsigned int Log::IndentWidth()
{
   return msSharedLog.indentWidth();
}

void Log::SetColorOutput(bool onoff)
{
   msSharedLog.setColorOutput(onoff);
}

bool Log::ColorOutput()
{
   return msSharedLog.colorOutput();
}

void Log::SetShowTimeStamps(bool onoff)
{
   msSharedLog.setShowTimeStamps(onoff);
}

bool Log::ShowTimeStamps()
{
   return msSharedLog.showTimeStamps();
}

// ---

Log::Log()
   : mLevelMask(LOG_ERROR|LOG_WARNING|LOG_INFO)
   , mColorOutput(true)
   , mShowTimeStamps(true)
   , mIndentLevel(0)
   , mIndentWidth(2)
   , mToFile(false)
{
#ifdef _DEBUG
   mLevelMask |= LOG_DEBUG;
#endif
   Bind(PrintStdout, mOutFunc);
}

Log::Log(const Path &path)
   : mLevelMask(LOG_ERROR|LOG_WARNING|LOG_INFO)
   , mColorOutput(true)
   , mShowTimeStamps(true)
   , mIndentLevel(0)
   , mIndentWidth(2)
   , mToFile(true)
   , mFilePath(path)
{
#ifdef _DEBUG
   mLevelMask |= LOG_DEBUG;
#endif
   mOutFile.open(path.fullname().c_str(), std::ofstream::app|std::ofstream::out);
}

Log::Log(const Log &rhs)
   : mOutFunc(rhs.mOutFunc)
   , mLevelMask(rhs.mLevelMask)
   , mColorOutput(rhs.mColorOutput)
   , mShowTimeStamps(rhs.mShowTimeStamps)
   , mIndentLevel(rhs.mIndentLevel)
   , mIndentWidth(rhs.mIndentWidth)
   , mToFile(rhs.mToFile)
   , mFilePath(rhs.mFilePath)
{
   if (mToFile)
   {
      mOutFile.open(mFilePath.fullname().c_str(), std::ofstream::app|std::ofstream::out);
   }
}

Log::~Log()
{
   if (mToFile && mOutFile.is_open())
   {
      mOutFile.close();
   }
}

Log& Log::operator=(const Log &rhs)
{
   if (this != &rhs)
   {
      mOutFunc = rhs.mOutFunc;
      mLevelMask = rhs.mLevelMask;
      mColorOutput = rhs.mColorOutput;
      mShowTimeStamps = rhs.mShowTimeStamps;
      mIndentLevel = rhs.mIndentLevel;
      mIndentWidth = rhs.mIndentWidth;
      if (mToFile && mOutFile.is_open())
      {
         mOutFile.close();
      }
      mToFile = rhs.mToFile;
      mFilePath = rhs.mFilePath;
      if (mToFile)
      {
         mOutFile.open(mFilePath.fullname().c_str(), std::ofstream::app|std::ofstream::out);
      }
   }
   return *this;
}

void Log::setOutputFunc(Log::OutputFunc func)
{
   if (mToFile && mOutFile.is_open())
   {
      mOutFile.close();
   }
   mToFile = false;
   mOutFunc = func;
}

void Log::setLevelMask(unsigned int mask)
{
   mLevelMask = mask;
}

unsigned int Log::levelMask() const
{
   return mLevelMask;
}

void Log::print(LogLevel lvl, const char *msg) const
{
   if ((mLevelMask & lvl) == 0 ||
       ( mToFile && !mOutFile.good()) ||
       (!mToFile &&  mOutFunc == 0))
   {
      return;
   }
   
   String heading = "";
   String ts = "";
   String trailing = "";
   StringList lines;
   
   if (mShowTimeStamps)
   {
      Date now;
      ts = now.format("%Y/%m/%d %H:%M:%S");
      ts += " ";
   }
   
   bool useColors = (!mToFile && mColorOutput);
   
   switch (lvl)
   {
   case LOG_ERROR:
      if (useColors)
      {
#ifdef _WIN32
         ChangeTermColors(TERM_COL_RED, -1);
#else
         heading += MakeTermCode(-1, TERM_COL_RED, -1);
         trailing = MakeTermCode(TERM_CMD_RESET, -1, -1);
#endif
      }
      heading += ts + "[  ERROR  ] ";
      break;
   case LOG_WARNING:
      if (useColors)
      {
#ifdef _WIN32
         ChangeTermColors(TERM_COL_YELLOW, -1);
#else
         heading += MakeTermCode(-1, TERM_COL_YELLOW, -1);
         trailing = MakeTermCode(TERM_CMD_RESET, -1, -1);
#endif
      }
      heading += ts + "[ WARNING ] ";
      break;
   case LOG_DEBUG:
      if (useColors)
      {
#ifdef _WIN32
         ChangeTermColors(TERM_COL_CYAN, -1);
#else
         heading += MakeTermCode(-1, TERM_COL_CYAN, -1);
         trailing = MakeTermCode(TERM_CMD_RESET, -1, -1);
#endif
      }
      heading += ts + "[  DEBUG  ] ";
      break;
   case LOG_INFO:
   default:
      heading += ts + "[ MESSAGE ] ";
      break;
   }
   
   for (unsigned int i=0; i<mIndentLevel; ++i)
   {
      for (unsigned int j=0; j<mIndentWidth; ++j)
      {
         heading += " ";
      }
   }
   
   String(msg).split('\n', lines);
   
   if (mToFile)
   {
      for (size_t i=0; i<lines.size(); ++i)
      {
         mOutFile << heading;
         mOutFile << lines[i];
         mOutFile << trailing;
         mOutFile << std::endl;
      }
   }
   else
   {
      for (size_t i=0; i<lines.size(); ++i)
      {
         mOutFunc(heading.c_str());
         mOutFunc(lines[i].c_str());
         mOutFunc(trailing.c_str());
         mOutFunc("\n");
      }
   }
   
#ifdef _WIN32
   if (useColors)
   {
      ResetTermColors();
   }
#endif
}

void Log::printError(const char *fmt, ...) const
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(mBuffer, 2048, fmt, args);
   va_end(args);
   print(LOG_ERROR, mBuffer);
}

void Log::printWarning(const char *fmt, ...) const
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(mBuffer, 2048, fmt, args);
   va_end(args);
   print(LOG_WARNING, mBuffer);
}

void Log::printDebug(const char *fmt, ...) const
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(mBuffer, 2048, fmt, args);
   va_end(args);
   print(LOG_DEBUG, mBuffer);
}

void Log::printInfo(const char *fmt, ...) const
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(mBuffer, 2048, fmt, args);
   va_end(args);
   print(LOG_INFO, mBuffer);
}

void Log::setIndentLevel(unsigned int l)
{
   mIndentLevel = l;
}

unsigned int Log::indentLevel() const
{
   return mIndentLevel;
}

void Log::indent()
{
   mIndentLevel += 1;
}

void Log::unIndent()
{
   if (mIndentLevel > 0)
   {
      mIndentLevel -= 1;
   }
}

void Log::setIndentWidth(unsigned int w)
{
   mIndentWidth = w;
}

unsigned int Log::indentWidth() const
{
   return mIndentWidth;
}

void Log::setColorOutput(bool onoff)
{
   mColorOutput = onoff;
}

bool Log::colorOutput() const
{
   return mColorOutput;
}

void Log::setShowTimeStamps(bool onoff)
{
   mShowTimeStamps = onoff;
}

bool Log::showTimeStamps() const
{
   return mShowTimeStamps;
}

}


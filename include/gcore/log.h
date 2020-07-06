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

#ifndef __gcore_log_h__
#define __gcore_log_h__

#include <gcore/date.h>
#include <gcore/path.h>
#include <gcore/functor.h>

namespace gcore
{
   enum LogLevel
   {
      LOG_ERROR = 0x01,
      LOG_WARNING = 0x02,
      LOG_DEBUG = 0x04,
      LOG_INFO = 0x08,
      LOG_ALL = LOG_ERROR|LOG_WARNING|LOG_DEBUG|LOG_INFO
   };
   
   
   class GCORE_API Log
   {
   public:
      
      typedef Functor1<const char*> OutputFunc;
      
   public:
      
      static Log& Get();
      
      // a bit wise combination of LOG_* constants from LogLevel enum
      static void SetLevelMask(unsigned int mask);
      static unsigned int LevelMask();
      
      static void SetOutputFunc(OutputFunc func);
      
      static void PrintError(const char *fmt, ...);
      static void PrintWarning(const char *fmt, ...);
      static void PrintDebug(const char *fmt, ...);
      static void PrintInfo(const char *fmt, ...);
      
      static void SetIndentLevel(unsigned int n);
      static unsigned int IndentLevel();
      static void Indent();
      static void UnIndent();
      
      static void SetIndentWidth(unsigned int w);
      static unsigned int IndentWidth();
      
      static void SetColorOutput(bool onoff);
      static bool ColorOutput();
      
      static void SetShowTimeStamps(bool onoff);
      static bool ShowTimeStamps();
      
   public:
      
      Log();
      Log(const Path &path);
      Log(const Log &rhs);
      ~Log();
      
      Log& operator=(const Log &rhs);
      
      void setOutputFunc(OutputFunc func);
      
      void setLevelMask(unsigned int mask);
      unsigned int levelMask() const;
      
      void printError(const char *fmt, ...) const;
      void printWarning(const char *fmt, ...) const;
      void printDebug(const char *fmt, ...) const;
      void printInfo(const char *fmt, ...) const;
      
      void setIndentLevel(unsigned int l);
      unsigned int indentLevel() const;
      void indent();
      void unIndent();
      
      void setIndentWidth(unsigned int w);
      unsigned int indentWidth() const;
      
      void setColorOutput(bool onoff);
      bool colorOutput() const;
      
      void setShowTimeStamps(bool onoff);
      bool showTimeStamps() const;
      
   private:
      
      void print(LogLevel lvl, const char *msg) const;
      
   private:
      
      mutable OutputFunc mOutFunc;
      unsigned int mLevelMask;
      bool mColorOutput;
      bool mShowTimeStamps;
      unsigned int mIndentLevel;
      unsigned int mIndentWidth;
      bool mToFile;
      Path mFilePath;
      mutable std::ofstream mOutFile;
      mutable char *mBuffer;
      mutable size_t mBufferSize;
      
      static Log msSharedLog;
      static char *msBuffer;
      static size_t msBufferSize;
   };
}

#endif

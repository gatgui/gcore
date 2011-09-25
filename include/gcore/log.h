#ifndef __gcore_log_h__
#define __gcore_log_h__

#include <gcore/date.h>
#include <gcore/path.h>
#include <gcore/functor.h>

namespace gcore
{
   class GCORE_API Log
   {
   public:
      
      typedef Functor1<const char*> OutputFunc;
      
      enum Level
      {
         ERROR = 0x01,
         WARNING = 0x02,
         DEBUG = 0x04,
         INFO = 0x08,
         ALL = ERROR|WARNING|DEBUG|INFO
      };
      
   public:
      
      static void SelectOutputs(unsigned int flags);
      static unsigned int SelectedOutputs();
      
      static void SetOutputFunc(OutputFunc func);
      
      static void PrintError(const char *fmt, ...);
      static void PrintWarning(const char *fmt, ...);
      static void PrintDebug(const char *fmt, ...);
      static void PrintInfo(const char *fmt, ...);
      
      static void SetIndentLevel(unsigned int n);
      static unsigned int GetIndentLevel();
      
      static void SetIndentWidth(unsigned int w);
      static unsigned int GetIndentWidth();
      
      static void EnableColors(bool onoff);
      static bool ColorsEnabled();
      
      static void ShowTimeStamps(bool onoff);
      static bool TimeStampsShown();
      
   public:
      
      Log();
      Log(const Path &path);
      Log(const Log &rhs);
      ~Log();
      
      Log& operator=(const Log &rhs);
      
      void setOutputFunc(OutputFunc func);
      
      void selectOutputs(unsigned int flags);
      unsigned int selectedOutputs() const;
      
      void printError(const char *fmt, ...) const;
      void printWarning(const char *fmt, ...) const;
      void printDebug(const char *fmt, ...) const;
      void printInfo(const char *fmt, ...) const;
      
      void setIndentLevel(unsigned int l);
      unsigned int getIndentLevel() const;
      
      void setIndentWidth(unsigned int w);
      unsigned int getIndentWidth() const;
      
      void enableColors(bool onoff);
      bool colorsEnabled() const;
      
      void showTimeStamps(bool onoff);
      bool timeStampsShown() const;
      
   private:
      
      void print(Level lvl, const char *msg) const;
      
   private:
      
      mutable OutputFunc mOutFunc;
      unsigned int mOutputs;
      bool mColors;
      bool mTimeStamps;
      unsigned int mIndentLevel;
      unsigned int mIndentWidth;
      bool mToFile;
      Path mFilePath;
      mutable std::ofstream mOutFile;
      mutable char mBuffer[2048];
      
      static Log msSharedLog;
      static char msBuffer[2048];
   };
}

#endif

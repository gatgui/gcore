#include <gcore/log.h>

#ifdef _WIN32

// TODO

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

namespace gcore
{

Log Log::msSharedLog;
char Log::msBuffer[2048];

void Log::SelectOutputs(unsigned int flags)
{
   msSharedLog.selectOutputs(flags);
}

unsigned int Log::SelectedOutputs()
{
   return msSharedLog.selectedOutputs();
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
   msSharedLog.print(ERROR, msBuffer);
}

void Log::PrintWarning(const char *fmt, ...)
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(msBuffer, 2048, fmt, args);
   va_end(args);
   msSharedLog.print(WARNING, msBuffer);
}

void Log::PrintDebug(const char *fmt, ...)
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(msBuffer, 2048, fmt, args);
   va_end(args);
   msSharedLog.print(DEBUG, msBuffer);
}

void Log::PrintInfo(const char *fmt, ...)
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(msBuffer, 2048, fmt, args);
   va_end(args);
   msSharedLog.print(INFO, msBuffer);
}

void Log::SetIndentLevel(unsigned int n)
{
   msSharedLog.setIndentLevel(n);
}

unsigned int Log::GetIndentLevel()
{
   return msSharedLog.getIndentLevel();
}

void Log::SetIndentWidth(unsigned int w)
{
   msSharedLog.setIndentWidth(w);
}

unsigned int Log::GetIndentWidth()
{
   return msSharedLog.getIndentWidth();
}

void Log::EnableColors(bool onoff)
{
   msSharedLog.enableColors(onoff);
}

bool Log::ColorsEnabled()
{
   return msSharedLog.colorsEnabled();
}

void Log::ShowTimeStamps(bool onoff)
{
   msSharedLog.showTimeStamps(onoff);
}

bool Log::TimeStampsShown()
{
   return msSharedLog.timeStampsShown();
}

// ---

Log::Log()
   : mOutputs(ERROR|WARNING|INFO)
   , mColors(true)
   , mTimeStamps(true)
   , mIndentLevel(0)
   , mIndentWidth(2)
   , mToFile(false)
{
#ifdef _DEBUG
   mOutputs |= DEBUG
#endif
}

Log::Log(const Path &path)
   : mOutputs(ERROR|WARNING|INFO)
   , mColors(true)
   , mTimeStamps(true)
   , mIndentLevel(0)
   , mIndentWidth(2)
   , mToFile(true)
   , mFilePath(path)
{
#ifdef _DEBUG
   mOutputs |= DEBUG
#endif
   mOutFile.open(path.fullname().c_str(), std::ofstream::app|std::ofstream::out);
}

Log::Log(const Log &rhs)
   : mOutFunc(rhs.mOutFunc)
   , mOutputs(rhs.mOutputs)
   , mColors(rhs.mColors)
   , mTimeStamps(rhs.mTimeStamps)
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
      mOutputs = rhs.mOutputs;
      mColors = rhs.mColors;
      mTimeStamps = rhs.mTimeStamps;
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

void Log::selectOutputs(unsigned int flags)
{
   mOutputs = flags;
}

unsigned int Log::selectedOutputs() const
{
   return mOutputs;
}

void Log::print(Level lvl, const char *msg) const
{
   if ((mOutputs & lvl) == 0 || (mToFile && !mOutFile.good()))
   {
      return;
   }
   
   Date now;
   String heading, trailing;
   StringList lines;
   
   if (mTimeStamps)
   {
      heading += now.format("%y/%m/%d %H:%M:%S");
      heading += " ";
   }
   
   bool useColors = (!mToFile && mColors);
   
   switch (lvl)
   {
   case ERROR:
      heading += "[  ERROR  ] ";
      if (useColors)
      {
#ifdef _WIN32
         // TODO
#else
         heading += MakeTermCode(-1, TERM_COL_RED, -1);
         trailing = MakeTermCode(TERM_CMD_RESET, -1, -1);
#endif
      }
      break;
   case WARNING:
      heading += "[ WARNING ] ";
#ifdef _WIN32
         // TODO
#else
         heading += MakeTermCode(-1, TERM_COL_YELLOW, -1);
         trailing = MakeTermCode(TERM_CMD_RESET, -1, -1);
#endif
      break;
   case DEBUG:
      heading += "[  DEBUG  ] ";
#ifdef _WIN32
         // TODO
#else
         heading += MakeTermCode(-1, TERM_COL_CYAN, -1);
         trailing = MakeTermCode(TERM_CMD_RESET, -1, -1);
#endif
      break;
   case INFO:
   default:
      heading += "[         ] ";
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
      if (mOutFunc != 0)
      {
         for (size_t i=0; i<lines.size(); ++i)
         {
            mOutFunc(heading.c_str());
            mOutFunc(lines[i].c_str());
            mOutFunc(trailing.c_str());
            mOutFunc("\n");
         }
      }
      else
      {
         for (size_t i=0; i<lines.size(); ++i)
         {
            std::cout << heading;
            std::cout << lines[i];
            std::cout << trailing;
            std::cout << std::endl;
         }
      }
   }
}

void Log::printError(const char *fmt, ...) const
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(mBuffer, 2048, fmt, args);
   va_end(args);
   print(ERROR, mBuffer);
}

void Log::printWarning(const char *fmt, ...) const
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(mBuffer, 2048, fmt, args);
   va_end(args);
   print(WARNING, mBuffer);
}

void Log::printDebug(const char *fmt, ...) const
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(mBuffer, 2048, fmt, args);
   va_end(args);
   print(DEBUG, mBuffer);
}

void Log::printInfo(const char *fmt, ...) const
{
   va_list args;
   
   va_start(args, fmt);
   vsnprintf(mBuffer, 2048, fmt, args);
   va_end(args);
   print(INFO, mBuffer);
}

void Log::setIndentLevel(unsigned int l)
{
   mIndentLevel = l;
}

unsigned int Log::getIndentLevel() const
{
   return mIndentLevel;
}

void Log::setIndentWidth(unsigned int w)
{
   mIndentWidth = w;
}

unsigned int Log::getIndentWidth() const
{
   return mIndentWidth;
}

void Log::enableColors(bool onoff)
{
   mColors = onoff;
}

bool Log::colorsEnabled() const
{
   return mColors;
}

void Log::showTimeStamps(bool onoff)
{
   mTimeStamps = onoff;
}

bool Log::timeStampsShown() const
{
   return mTimeStamps;
}

}


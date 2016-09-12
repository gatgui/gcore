from libcpp.map cimport map
from cpython cimport PyObject
import sys


cdef extern from "<gcore/platform.h>":
   
   cdef char DIR_SEP 
   cdef char PATH_SEP
   

cdef extern from "<gcore/status.h>" namespace "gcore":
   
   cdef cppclass Status:
      Status()
      Status(bint)
      Status(bint, char*)
      Status(bint, int)
      Status(bint, int, char*)
      
      void clear()
      void set(bint success)
      void set(bint success, int errcode)
      void set(bint success, char *msg)
      void set(bint success, int errcode, char *msg)
      
      bint succeeded()
      bint failed()
      int errcode()
      char* message()
   

cdef extern from "<gcore/list.h>" namespace "gcore":
   
   cdef cppclass List[T]:
      cppclass iterator:
         T operator*()
         iterator operator++()
         bint operator==(iterator)
         bint operator!=(iterator)
      List()
      void push_back(T&)
      T& operator[](int)
      T& at(int)
      iterator begin()
      iterator end()
   

cdef extern from "<gcore/string.h>" namespace "gcore":
  
   cdef cppclass String:
      String()
      String(char*)
      String(String &)
      char* c_str()
   
   cdef cppclass StringList:
      StringList()
      void push_back(String&)
      String& operator[](int)
      String& at(int)
      List[String].iterator begin()
      List[String].iterator end()
   

cdef extern from "<gcore/path.h>" namespace "gcore":
   
   cdef cppclass Path:
      Path()
      Path(char*)
      Path(Path&)
      
      Path& assign "operator=" (Path&)
      Path& plus_eq "operator+=" (Path&)
      
      bint operator==(Path&)
      bint operator!=(Path&)
      
      String& to_string "operator gcore::String&" ()
      
      int depth()
      String& operator[](int)
      
      bint isAbsolute()
      Path& makeAbsolute()
      Path& normalize()
      
      String basename()
      String dirname(char) # char sep=DIR_SEP
      String fullname(char) # char sep=DIR_SEP
      String extension()
      bint checkExtension(String&)
      size_t fileSize()
      
      bint createDir(bint) # bool recursive=false
      bint removeFile()
      
      String pop()
      Path& push(String&)
      
      bint isDir()
      bint isFile()
      bint exists()
      # Date lastModification()
      
      size_t listDir(List[Path]&, bint, int) # as above
   

cdef extern from "<gcore/path.h>" namespace "gcore::Path":
   
   cdef Path CurrentDir()
   
   cdef enum ForEachTarget:
      FE_FILE, FE_DIRECTORY, FE_HIDDEN, FE_ALL


cdef extern from "pathenumerator.h":
   
   cdef cppclass PathEnumerator:
      PathEnumerator()
      PathEnumerator(PyObject*)
      void setPyFunc(PyObject*)
      void apply(Path, bint, int)


cdef extern from "<gcore/xml.h>" namespace "gcore":
   
   cdef cppclass XMLElement:
      XMLElement()
      XMLElement(String&)
      
      bint addChild(XMLElement*)
      void removeChild(XMLElement*)
      void removeChild(size_t)
      
      XMLElement* getParent()
      size_t numChildren()
      XMLElement* getChild(size_t)
      
      bint setAttribute(String&, String&)
      void removeAttribute(String&)
      bint hasAttribute(String&)
      String& getAttribute(String&)
      size_t getAttributes(map[String,String]&)
      
      bint setText(String&, bint)
      bint addText(String&)
      String& getText()
      
      void setTag(String&)
      String& getTag()
      
      bint hasChildWithTag(String&)
      size_t numChildrenWithTag(String&)
      XMLElement* getChildWithTag(String&, size_t)
      size_t getChildrenWithTag(String&, List[XMLElement*]&)
   
   cdef cppclass XMLDoc:
      XMLDoc()
      
      void setRoot(XMLElement*)
      XMLElement* getRoot(size_t)

      size_t numRoots()
      void addRoot(XMLElement*)
      
      void write(String&)
      bint read(String&)
   


cdef extern from "<gcore/plist.h>" namespace "gcore":
   
   cdef cppclass PropertyList:
      PropertyList()
      PropertyList(PropertyList&)
      
      PropertyList& assign "operator=" (PropertyList&)
      
      void create()
      
      Status read(String&)
      Status read(XMLElement*)
      void write(String&)
      XMLElement* write(XMLElement*)
      
      bint has(String&)
      
      String getString(String&, Status*)
      long getInteger(String&, Status*)
      double getReal(String&, Status*)
      bint getBoolean(String&, Status*)
      
      size_t getSize(String&, Status*)
      size_t getKeys(String&, StringList&, Status*)
      Status clear(String&)
      bint remove(String&)
      
      Status setString(String&, String&)
      Status setInteger(String&, long)
      Status setReal(String&, double)
      Status setBoolean(String&, bint)
   

cdef extern from "<gcore/dirmap.h>" namespace "gcore::dirmap":
   
   void AddMapping(String&, String&)
   void RemoveMapping(String&, String&)
   void ReadMappingsFromFile(Path&)
   String Map(String&)
   

cdef extern from "<gcore/env.h>" namespace "gcore":
   
   cdef cppclass Env:
      Env()
      
      void push()
      void pop()
      
      bint isSet(String&)
      String get(String&)
      void set(String&, String&, bint)
      void set(map[String,String]&, bint)
      
      size_t asDict(map[String,String]&)
   

cdef extern from "<gcore/env.h>" namespace "gcore::Env":
   
   String Username()
   String Hostname()
   String Get(String&)
   void Set(String&, String&, bint)
   void Set(map[String,String]&, bint)
   bint IsSet(String&)
   size_t ListPaths(String&, List[Path]&)
   

cdef extern from "<gcore/argparser.h>" namespace "gcore::FlagDesc":
   
   cdef enum Option:
      FT_OPTIONAL, FT_NEEDED, FT_MULTI


cdef extern from "<gcore/argparser.h>" namespace "gcore":
   
   cdef cppclass FlagDesc:
      int opts
      String longname
      String shortname
      int arity

   cdef cppclass ArgParser:
      ArgParser(FlagDesc*, int)

      size_t argumentCount()
      bint getArgument(size_t, String&)
      bint getArgument(size_t, float&)
      bint getArgument(size_t, double&)
      bint getArgument(size_t, int&)
      bint getArgument(size_t, unsigned int&)
      bint getArgument(size_t, bint&)

      bint isFlagSet(String&)
      size_t flagOccurenceCount(String&)
      size_t flagArgumentCount(String&, size_t)
      bint getFlagArgument(String&, size_t, String&)
      bint getFlagArgument(String&, size_t, float&)
      bint getFlagArgument(String&, size_t, double&)
      bint getFlagArgument(String&, size_t, int&)
      bint getFlagArgument(String&, size_t, unsigned int&)
      bint getFlagArgument(String&, size_t, bint&)
      bint getMultiFlagArgument(String&, size_t, size_t, String&)
      bint getMultiFlagArgument(String&, size_t, size_t, float&)
      bint getMultiFlagArgument(String&, size_t, size_t, double&)
      bint getMultiFlagArgument(String&, size_t, size_t, int&)
      bint getMultiFlagArgument(String&, size_t, size_t, unsigned int&)
      bint getMultiFlagArgument(String&, size_t, size_t, bint&)

      Status parse(int, char **)
   

cdef extern from "<gcore/log.h>" namespace "gcore":
   
   cdef enum LogLevel:
      LOG_ERROR, LOG_WARNING, LOG_DEBUG, LOG_INFO, LOG_ALL
   
   cdef cppclass Log:
      Log()
      Log(Path&)
      Log(Log&)
      
      Log& assign "operator=" (Log&)
      
      void setLevelMask(unsigned int)
      unsigned int levelMask()
      
      void printError(char*)
      void printWarning(char*)
      void printDebug(char*)
      void printInfo(char*)
      
      void setIndentLevel(unsigned int)
      unsigned int indentLevel()
      void indent()
      void unIndent()
      
      void setIndentWidth(unsigned int)
      unsigned int indentWidth()
      
      void setColorOutput(bint)
      bint colorOutput()
      
      void setShowTimeStamps(bint)
      bint showTimeStamps()


cdef extern from "<gcore/log.h>" namespace "gcore::Log":
   
   void SetLevelMask(unsigned int mask)
   unsigned int LevelMask()
   
   void PrintError(char *fmt)
   void PrintWarning(char *fmt)
   void PrintDebug(char *fmt)
   void PrintInfo(char *fmt)
   
   void SetIndentLevel(unsigned int n)
   unsigned int IndentLevel()
   void Indent()
   void UnIndent()
   
   void SetIndentWidth(unsigned int w)
   unsigned int IndentWidth()
   
   void SetColorOutput(bint onoff)
   bint ColorOutput()
   
   void SetShowTimeStamps(bint onoff)
   bint ShowTimeStamps()
   

cdef extern from "log.h":
   
   void PyLog_SetOutputFunc(PyObject*) except +
   
   cdef cppclass LogOutputFunc:
      LogOutputFunc()
      LogOutputFunc(LogOutputFunc&)
      
      LogOutputFunc& assign "operator=" (LogOutputFunc&)
      
      void setPyFunc(PyObject*)
      void assign(Log&)
   

cdef extern from "<gcore/md5.h>" namespace "gcore":
   
   cdef cppclass MD5:
      MD5()
      MD5(MD5&)
      MD5(String&)
      
      void update(char*, int)
      void update(String&)
      void clear()
      
      String asString()
   

cdef extern from "<gcore/time.h>" namespace "gcore::TimeCounter":
   
   cdef enum Units:
      CurrentUnits, NanoSeconds, MicroSeconds, MilliSeconds, Seconds, Minutes, Hours
   
   char* UnitsString(Units)
   double ConvertUnits(double, Units, Units)
   

cdef extern from "<gcore/time.h>" namespace "gcore":
   
   cdef cppclass TimeCounter:
      TimeCounter()
      TimeCounter(Units)
      TimeCounter(double)
      TimeCounter(double, Units)
      TimeCounter(TimeCounter&)
      
      TimeCounter& assign "operator=" (TimeCounter&)
      TimeCounter& plus_eq "operator+=" (TimeCounter&)
      TimeCounter& plus_eq "operator+=" (double)
      TimeCounter& sub_eq "operator-=" (TimeCounter&)
      TimeCounter& sub_eq "operator-=" (double)
      # Compare operator? <= < == != > >=
      
      void restart()
      
      bint setUnits(Units)
      Units units()
      
      bint setValue(double)
      bint setValue(double, Units)
      double value()
      double value(Units)
      
      double nanoseconds()
      double microseconds()
      double milliseconds()
      double seconds()
      double minutes()
      double hours()
      
      TimeCounter elapsed()
   

cdef extern from "<gcore/perflog.h>" namespace "gcore::PerfLog":
   
   cdef enum ShowFlags:
      ShowTotalTime, ShowFuncTime, ShowAvgTotalTime, ShowAvgFuncTime, ShowNumCalls, ShowDetailed, ShowFlag, ShowDefaults, ShowAll
   
   cdef enum SortCriteria:
      SortIdentifier, SortTotalTime, SortFuncTime, SortAvgTotalTime, SortAvgFuncTime, SortNumCalls, SortReverse
   
   cdef enum Output:
      ConsoleOutput, LogOutput
   

cdef extern from "<gcore/perflog.h>" namespace "gcore":
   
   cdef cppclass PerfLog:
      PerfLog()
      PerfLog(TimeCounter.Units)
      PerfLog(PerfLog&)
      
      PerfLog& assign "operator=" (PerfLog&)
      
      void begin(String&)
      void end()
      void _print "print" (Output, int, int, TimeCounter.Units)
      void _print "print" (Log&, int, int, TimeCounter.Units)
      void clear()
   

cdef extern from "<gcore/perflog.h>" namespace "gcore::PerfLog":
   
   PerfLog& Get()
   void Begin(String&)
   void End()
   void Print(Output, int, int, TimeCounter.Units)
   void Print(Log&, int, int, TimeCounter.Units)
   void Clear()


cdef extern from "<gcore/pipe.h>" namespace "gcore":
   
   cdef cppclass Pipe:
      Pipe()
      Pipe(Pipe&)
      
      Pipe& assign "operator=" (Pipe&)
      
      bint isNamed()
      String& name()
      
      bint isOwned()
      bint canRead()
      bint canWrite()
      
      Status create()
      Status open(String&)
      
      void close()
      void closeRead()
      void closeWrite()
      
      int read(char*, int, Status*)
      int write(String&, Status*)
   

cdef extern from "<gcore/process.h>" namespace "gcore":
   
   cdef cppclass Process:
      Process()
      
      void setEnv(String&, String&)
      
      void setRedirectOut(bint)
      bint redirectOut()
      
      void setRedirectErr(bint)
      bint redirectErr()
      
      void setRedirectErrToOut(bint)
      bint redirectErrToOut()
      
      void setRedirectIn(bint)
      bint redirectIn()
      
      void setShowConsole(bint)
      bint showConsole()
      
      void setKeepAlive(bint)
      bint keepAlive()
      
      int id()
      
      Status run(String&)
      Status run(StringList&)
      bint isRunning()
      int wait(bint, Status*)
      Status kill()
      int returnCode()
      String& cmdLine()
      
      bint canReadOut()
      int readOut(char*, int, Status*)
      bint canReadErr()
      int readErr(char*, int, Status*)
      bint canWriteIn()
      int write(String&, Status*)
      
      
   
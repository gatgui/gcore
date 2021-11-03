# MIT License
#
# Copyright (c) 2010 Gaetan Guidet
#
# This file is part of gcore.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


from libcpp.map cimport map
from cpython cimport PyObject

cdef extern from "<gcore/platform.h>":
   
   cdef char DIR_SEP 
   cdef char PATH_SEP


cdef extern from "<string>" namespace "std":
  
   cdef cppclass string:
      string()
      string(string &)
      string(char*)
      char* c_str()
     

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
      
      String& operator[](int)
      
      bint isAbsolute()
      Path& makeAbsolute()
      Path& normalize()
      
      String basename()
      String dirname(char) # char sep=DIR_SEP
      String fullname(char) # char sep=DIR_SEP
      String getExtension()
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
   
   cdef Path GetCurrentDir()
   
   cdef enum EachTarget:
      ET_FILE, ET_DIRECTORY, ET_HIDDEN, ET_ALL


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
      
      bint read(String&)
      bint read(XMLElement*)
      void write(String&)
      XMLElement* write(XMLElement*)
      
      bint has(String&)
      
      #String& getString(String&) except +
      String getString(String&) except +
      long getInteger(String&) except +
      double getReal(String&) except +
      bint getBoolean(String&) except +
      
      size_t getSize(String&) except +
      size_t getKeys(String&, StringList&) except +
      void clear(String&) except +
      bint remove(String&)
      
      void setString(String&, String&) except +
      void setInteger(String&, long) except +
      void setReal(String&, double) except +
      void setBoolean(String&, bint) except +
   

cdef extern from "<gcore/dirmap.h>" namespace "gcore::Dirmap":
   
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
      void setAll(map[String,String]&, bint)
      
      size_t asDict(map[String,String]&)
   

cdef extern from "<gcore/env.h>" namespace "gcore::Env":
   
   String GetUser()
   String GetHost()
   String Get(String&)
   void Set(String&, String&, bint)
   void SetAll(map[String,String]&, bint)
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

      size_t getArgumentCount()
      bint getArgument(size_t, String&)
      bint getArgument(size_t, float&)
      bint getArgument(size_t, double&)
      bint getArgument(size_t, int&)
      bint getArgument(size_t, unsigned int&)
      bint getArgument(size_t, bint&)

      bint isFlagSet(String&)
      size_t getFlagOccurenceCount(String&)
      size_t getFlagArgumentCount(String&, size_t)
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

      void parse(int, char **) except +


cdef extern from "<gcore/log.h>" namespace "gcore":
   
   cdef enum LogLevel:
      LOG_ERROR, LOG_WARNING, LOG_DEBUG, LOG_INFO, LOG_ALL
   
   cdef cppclass Log:
      Log()
      Log(Path&)
      Log(Log&)
      
      Log& assign "operator=" (Log&)
      
      void selectOutputs(unsigned int)
      unsigned int selectedOutputs()
      
      void printError(char*)
      void printWarning(char*)
      void printDebug(char*)
      void printInfo(char*)
      
      void setIndentLevel(unsigned int)
      unsigned int getIndentLevel()
      void indent()
      void unIndent()
      
      void setIndentWidth(unsigned int)
      unsigned int getIndentWidth()
      
      void enableColors(bint)
      bint colorsEnabled()
      
      void showTimeStamps(bint)
      bint timeStampsShown()


cdef extern from "<gcore/log.h>" namespace "gcore::Log":
   
   void SelectOutputs(unsigned int flags)
   unsigned int SelectedOutputs()
   
   void PrintError(char *fmt)
   void PrintWarning(char *fmt)
   void PrintDebug(char *fmt)
   void PrintInfo(char *fmt)
   
   void SetIndentLevel(unsigned int n)
   unsigned int GetIndentLevel()
   void Indent()
   void UnIndent()
   
   void SetIndentWidth(unsigned int w)
   unsigned int GetIndentWidth()
   
   void EnableColors(bint onoff)
   bint ColorsEnabled()
   
   void ShowTimeStamps(bint onoff)
   bint TimeStampsShown()
   

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
      

cdef extern from "<gcore/perflog.h>" namespace "gcore::PerfLog":
   
   cdef enum Units:
      CurrentUnits, NanoSeconds, MilliSeconds, Seconds, Minutes, Hours
   
   cdef enum ShowFlags:
      ShowTotalTime, ShowFuncTime, ShowAvgTotalTime, ShowAvgFuncTime, ShowNumCalls, ShowDetailed, ShowFlag, ShowDefaults, ShowAll
   
   cdef enum SortCriteria:
      SortIdentifier, SortTotalTime, SortFuncTime, SortAvgTotalTime, SortAvgFuncTime, SortNumCalls, SortReverse
   
   cdef enum Output:
      ConsoleOutput, LogOutput


cdef extern from "<gcore/perflog.h>" namespace "gcore":
   
   cdef cppclass PerfLog:
      PerfLog()
      PerfLog(Units)
      PerfLog(PerfLog&)
      
      PerfLog& assign "operator=" (PerfLog&)
      
      void begin(string&)
      void end()
      void _print "print" (Output, int, int, Units)
      void _print "print" (Log&, int, int, Units)
      void clear()
   


cdef extern from "<gcore/perflog.h>" namespace "gcore::PerfLog":
   
   PerfLog& SharedInstance()
   void Begin(string&)
   void End()
   void Print(Output, int, int, Units)
   void Print(Log&, int, int, Units)
   void Clear()
   char* UnitsString(Units)
   double ConvertUnits(double, Units, Units)


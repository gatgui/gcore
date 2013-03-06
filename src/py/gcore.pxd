from libcpp.map cimport map
from cpython cimport PyObject

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
      String fullname(char)
   

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
      XMLElement* getRoot()
      
      void write(String&)
      bint read(String&)
   


cdef extern from "<gcore/plist.h>" namespace "gcore":
   
   cdef cppclass PropertyList:
      PropertyList()
      PropertyList(PropertyList&)
      
      PropertyList& assign "operator=" (PropertyList&)
      
      void create()
      
      bint read(String&)
      void write(String&)
      
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
   

cdef extern from "<gcore/md5.h>" namespace "gcore":
   
   cdef cppclass MD5:
      MD5()
      MD5(MD5&)
      MD5(String&)
      
      void update(char*, int)
      void update(String&)
      void clear()
      
      String asString()
      

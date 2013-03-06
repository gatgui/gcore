cimport gcore
from cython.operator cimport dereference as deref
from libc.stdlib cimport malloc, free
from libcpp.vector cimport vector

ctypedef public class FlagDesc [object PyFlagDesc, type PyFlagDescType]:
   cdef gcore.FlagDesc *_cobj
   cdef bint _own

   FT_OPTIONAL = gcore.FT_OPTIONAL
   FT_NEEDED = gcore.FT_NEEDED
   FT_MULTI = gcore.FT_MULTI

   def __cinit__(self, *args, **kwargs):
      self._cobj = NULL
      self._own = False
   
   def __init__(self, *args, noalloc=False, **kwargs):
      if noalloc:
         self._cobj = NULL
         self._own = False
         return
      self._cobj = new gcore.FlagDesc()
      self._own = True
      if "opts" in kwargs:
         self._cobj.opts = <gcore.Option>kwargs["opts"]
      if "longname" in kwargs:
         self._cobj.longname = gcore.String(<char*>kwargs["longname"])
      if "shortname" in kwargs:
         self._cobj.shortname = gcore.String(<char*>kwargs["shortname"])
      if "arity" in kwargs:
         self._cobj.arity = kwargs["arity"]
   
   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del self._cobj
         self._cobj = NULL

   property opts:
      def __get__(self): return self._cobj.opts
      def __set__(self, v): self._cobj.opts = <gcore.Option>v

   property longname:
      def __get__(self): return self._cobj.longname.c_str()
      def __set__(self, v): self._cobj.longname = gcore.String(<char*>v)

   property shortname:
      def __get__(self): return self._cobj.shortname.c_str()
      def __set__(self, v): self._cobj.shortname = gcore.String(<char*>v)

   property arity:
      def __get__(self): return self._cobj.arity
      def __set__(self, v): self._cobj.arity = v


ctypedef public class ArgParser [object PyArgParser, type PyArgParserType]:
   cdef gcore.ArgParser *_cobj
   cdef bint _own
   
   def __cinit__(self, *args, **kwargs):
      self._cobj = NULL
      self._own = False
   
   def __init__(self, argdesc=[], *args, noalloc=False, **kwargs):
      if noalloc:
         self._cobj = NULL
         self._own = False
         return
      
      cdef vector[gcore.FlagDesc] flags
      cdef int count = len(argdesc)
      cdef int i = 0
      
      if count > 0:
         flags.resize(count)
         while i < count:
            flags[i] = deref((<FlagDesc?> argdesc[i])._cobj)
            i += 1
         self._cobj = new gcore.ArgParser(&flags[0], flags.size())
      else:
         self._cobj = new gcore.ArgParser(NULL, 0)
      
      self._own = True
   
   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del(self._cobj)
         self._cobj = NULL
   
   def getArgumentCount(self):
      return (self._cobj.getArgumentCount() if self._cobj != NULL else 0)
   
   def getStringArgument(self, idx):
      cdef gcore.String rv
      if not self._cobj.getArgument(<size_t?>idx, rv):
         if idx < 0 or idx >= self.getArgumentCount():
            raise Exception("_gcore.ArgParser.getStringArgument: Invalid argument index %d" % idx)
         else:
            raise Exception("_gcore.ArgParser.getStringArgument: Argument %d cannot be converted to string" % idx)
      return rv.c_str()
   
   def getFloatArgument(self, idx):
      cdef float rv = 0
      if not self._cobj.getArgument(<size_t?>idx, rv):
         if idx < 0 or idx >= self.getArgumentCount():
            raise Exception("_gcore.ArgParser.getFloatArgument: Invalid argument index %d" % idx)
         else:
            raise Exception("_gcore.ArgParser.getFloatArgument: Argument %d cannot be converted to float" % idx)
      return rv
   
   def getDoubleArgument(self, idx):
      cdef double rv = 0
      if not self._cobj.getArgument(<size_t?>idx, rv):
         if idx < 0 or idx >= self.getArgumentCount():
            raise Exception("_gcore.ArgParser.getDoubleArgument: Invalid argument index %d" % idx)
         else:
            raise Exception("_gcore.ArgParser.getDoubleArgument: Argument %d cannot be converted to double" % idx)
      return rv
   
   def getIntArgument(self, idx):
      cdef int rv = 0
      if not self._cobj.getArgument(<size_t?>idx, rv):
         if idx < 0 or idx >= self.getArgumentCount():
            raise Exception("_gcore.ArgParser.getIntArgument: Invalid argument index %d" % idx)
         else:
            raise Exception("_gcore.ArgParser.getIntArgument: Argument %d cannot be converted to int" % idx)
      return rv
   
   def getUIntArgument(self, idx):
      cdef unsigned int rv = 0
      if not self._cobj.getArgument(<size_t?>idx, rv):
         if idx < 0 or idx >= self.getArgumentCount():
            raise Exception("_gcore.ArgParser.getUIntArgument: Invalid argument index %d" % idx)
         else:
            raise Exception("_gcore.ArgParser.getUIntArgument: Argument %d cannot be converted to unsigned int" % idx)
      return rv 
   
   def getBoolArgument(self, idx):
      cdef bint rv = False
      if not self._cobj.getArgument(<size_t?>idx, rv):
         if idx < 0 or idx >= self.getArgumentCount():
            raise Exception("_gcore.ArgParser.getBoolArgument: Invalid argument index %d" % idx)
         else:
            raise Exception("_gcore.ArgParser.getBoolArgument: Argument %d cannot be converted to bool" % idx)
      return rv
   
   def isFlagSet(self, name):
      return self._cobj.isFlagSet(gcore.String(<char*?> name))
   
   def getFlagOccurenceCount(self, name):
      return self._cobj.getFlagOccurenceCount(gcore.String(<char*?> name))
   
   def getFlagArgumentCount(self, name, occurence=0):
      return self._cobj.getFlagArgumentCount(gcore.String(<char*?>name), <size_t?>occurence)
   
   def _flagError(self, name, occ, idx, funcname, typename):
      if not self.isFlagSet(name):
         raise Exception("_gcore.ArgParser.%s: Invalid flag argument \"%s\"" % (funcname, name))
      elif occ < 0 or occ >= self.getFlagOccurenceCount(name):
         raise Exception("_gcore.ArgParser.%s: Invalid occurence %d for flag argument \"%s\"" % (funcname, occ, name))
      elif idx < 0 or idx >= self.getFlagArgumentCount(name, occ):
         raise Exception("_gcore.ArgParser.%s: Invalid index %d for flag argument \"%s\" (%d)" % (funcname, idx, name, occ))
      else:
         raise Exception("_gcore.ArgParser.%s: Flag argument \"%s\" %d (%d) cannot be converted to %s" % (funcname, name, idx, occ, typename))
   
   def getFlagStringArgument(self, name, idx):
      cdef gcore.String cname = gcore.String(<char*?> name)
      cdef gcore.String rv
      if not self._cobj.getFlagArgument(cname, <size_t?>idx, rv):
         self._flagError(name, 0, idx, "getFlagStringArgument", "string")
      return rv.c_str()
   
   def getFlagFloatArgument(self, name, idx):
      cdef gcore.String cname = gcore.String(<char*?> name)
      cdef float rv = 0
      if not self._cobj.getFlagArgument(cname, <size_t?>idx, rv):
         self._flagError(name, 0, idx, "getFlagFloatArgument", "float")
      return rv
   
   def getFlagDoubleArgument(self, name, idx):
      cdef gcore.String cname = gcore.String(<char*?> name)
      cdef double rv = 0
      if not self._cobj.getFlagArgument(cname, <size_t?>idx, rv):
         self._flagError(name, 0, idx, "getFlagDoubleArgument", "double")
      return rv
   
   def getFlagIntArgument(self, name, idx):
      cdef gcore.String cname = gcore.String(<char*?> name)
      cdef int rv = 0
      if not self._cobj.getFlagArgument(cname, <size_t?>idx, rv):
         self._flagError(name, 0, idx, "getFlagIntArgument", "int")
      return rv
   
   def getFlagUIntArgument(self, name, idx):
      cdef gcore.String cname = gcore.String(<char*?> name)
      cdef unsigned int rv = 0
      if not self._cobj.getFlagArgument(cname, <size_t?>idx, rv):
         self._flagError(name, 0, idx, "getFlagUIntArgument", "unsigned int")
      return rv 
   
   def getFlagBoolArgument(self, name, idx):
      cdef gcore.String cname = gcore.String(<char*?> name)
      cdef bint rv = False
      if not self._cobj.getFlagArgument(cname, <size_t?>idx, rv):
         self._flagError(name, 0, idx, "getFlagBoolArgument", "bool")
      return rv
   
   def getMultiFlagStringArgument(self, name, occ, idx):
      cdef gcore.String cname = gcore.String(<char*?> name)
      cdef gcore.String rv
      if not self._cobj.getMultiFlagArgument(cname, <size_t?>occ, <size_t?>idx, rv):
         self._flagError(name, occ, idx, "getMultiFlagStringArgument", "string")
      return rv.c_str()
   
   def getMultiFlagFloatArgument(self, name, occ, idx):
      cdef gcore.String cname = gcore.String(<char*?> name)
      cdef float rv = 0
      if not self._cobj.getMultiFlagArgument(cname, <size_t?>occ, <size_t?>idx, rv):
         self._flagError(name, occ, idx, "getMultiFlagFloatArgument", "float")
      return rv
   
   def getMultiFlagDoubleArgument(self, name, occ, idx):
      cdef gcore.String cname = gcore.String(<char*?> name)
      cdef double rv = 0
      if not self._cobj.getMultiFlagArgument(cname, <size_t?>occ, <size_t?>idx, rv):
         self._flagError(name, occ, idx, "getMultiFlagDoubleArgument", "double")
      return rv
   
   def getMultiFlagIntArgument(self, name, occ, idx):
      cdef gcore.String cname = gcore.String(<char*?> name)
      cdef int rv = 0
      if not self._cobj.getMultiFlagArgument(cname, <size_t?>occ, <size_t?>idx, rv):
         self._flagError(name, occ, idx, "getMultiFlagIntArgument", "int")
      return rv
   
   def getMultiFlagUIntArgument(self, name, occ, idx):
      cdef gcore.String cname = gcore.String(<char*?> name)
      cdef unsigned int rv = 0
      if not self._cobj.getMultiFlagArgument(cname, <size_t?>occ, <size_t?>idx, rv):
         self._flagError(name, occ, idx, "getMultiFlagUIntArgument", "unsigned int")
      return rv 
   
   def getMultiFlagBoolArgument(self, name, occ, idx):
      cdef gcore.String cname = gcore.String(<char*?> name)
      cdef bint rv = False
      if not self._cobj.getMultiFlagArgument(cname, <size_t?>occ, <size_t?>idx, rv):
         self._flagError(name, occ, idx, "getMultiFlagBoolArgument", "bool")
      return rv
   
   def parse(self, args):
      cdef int argc = len(args)
      cdef char **argv = <char**> malloc((argc + 1) * sizeof(char*))
      cdef int i = 0
      cdef char *arg = NULL
      
      if not argv:
         raise MemoryError()

      while i < argc:
         argv[i] = <char*?> args[i]
         i += 1
      argv[argc] = NULL

      self._cobj.parse(argc, &argv[0])

      free(argv)


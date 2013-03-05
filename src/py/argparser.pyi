cimport gcore

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
   cdef gcore.Env *_cobj
   cdef bint _own
   

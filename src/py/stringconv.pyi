cimport gcore
import sys

cdef _to_cstring(ps, gcore.String &cs):
   if isinstance(ps, unicode):
      _ps = ps.encode("utf-8")
      cs.assign(gcore.String(<char*?>_ps))
   else:
      try:
         _ps = ps.decode("ascii")
      except:
         try:
            _ps = ps.decode(sys.getfilesystemencoding())
         except:
            try:
               _ps = ps.decode("utf-8")
            except:
               raise Exception("ascii, locale or utf-8 encoded string is expected")
      _ps = _ps.encode("utf-8")
      cs.assign(gcore.String(<char*?>ps))
   
cdef _to_pystring(gcore.String &cs, asUnicode=False):
   cdef gcore.String _tmp
   u = unicode(cs.c_str(), "utf-8")
   try:
      return u.encode("ascii")
   except:
      if asUnicode:
         return u
      else:
         return u.encode(sys.getfilesystemencoding())

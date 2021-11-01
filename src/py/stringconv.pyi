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

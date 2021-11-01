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

class dirmap(object):
   def __init__(self):
      object.__init__(self)

   @staticmethod
   def AddMapping(char* frompath, char* topath):
      gcore.AddMapping(gcore.String(frompath), gcore.String(topath))
   
   @staticmethod
   def RemoveMapping(char* frompath, char* topath):
      gcore.RemoveMapping(gcore.String(frompath), gcore.String(topath))
   
   @staticmethod
   def WriteMappingsToFile(char *path):
      gcore.WriteMappingsToFile(gcore.Path(path))
   
   @staticmethod
   def ReadMappingsFromFile(char* path):
      gcore.ReadMappingsFromFile(gcore.Path(path))
   
   @staticmethod
   def Map(char* path, reverseLookup=False):
      return gcore.Map(gcore.String(path), reverseLookup).c_str()


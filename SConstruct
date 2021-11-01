# MIT License
#
# Copyright (c) 2016 Gaetan Guidet
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

import excons
import os
import sys
import glob
import excons.tools
from excons.tools import threads
from excons.tools import dl
from excons.tools import python
import SCons.Script # pylint: disable=import-error

# pylint: disable=bad-indentation,no-member

excons.InitGlobals()


static = excons.GetArgument("static", 0, int)
debugext = excons.GetArgument("debug-extra", 0, int)
debugrex = excons.GetArgument("debug-rex", 0, int)
plat = str(SCons.Script.Platform())

libdefs = ["GCORE_STATIC"] if static else ["GCORE_EXPORTS"]
if debugext:
   libdefs.append("GCORE_DEBUG")
if debugrex:
   libdefs.append("_DEBUG_REX")
if plat == "win32":
   libdefs.append("_CRT_SECURE_NO_WARNINGS")
liblibs = []
libcustom = []
if not static:
   libcustom = [threads.Require, dl.Require]
   if not plat in ["win32", "darwin"]:
      liblibs = ["rt"]


def RequireGcore(env): # pylint: disable=redefined-outer-name
   # Don't need to set CPPPATH, headers are now installed in output directory
   # Don't need to set LIBPATH, library output directory is automatically added by excons
   env.Append(LIBS=["gcore"])

   if static:
     env.Append(CPPDEFINES=["GCORE_STATIC"])
     threads.Require(env)
     dl.Require(env)

   p = str(SCons.Script.Platform())

   if not p in ["win32", "darwin"]:
     env.Append(LIBS=["rt"])
  
   if p == "win32":
     env.Append(CPPDEFINES=["_CRT_SECURE_NO_WARNINGS"])

SCons.Script.Export("RequireGcore")


prjs = [
   {  "name"         : "gcore",
      "type"         : "staticlib" if static else "sharedlib",
      "version"      : "1.0.0",
      "soname"       : "libgcore.so.1",
      "install_name" : "libgcore.1.dylib",
      "srcs"         : glob.glob("src/lib/*.cpp") + glob.glob("src/lib/rex/*.cpp"),
      "install"      : {"include": ["include/gcore", "include/half.h"]},
      "defs"         : libdefs,
      "custom"       : libcustom,
      "libs"         : liblibs
   },
   {  "name"      : "_gcore",
      "type"      : "dynamicmodule",
      "alias"     : "gcorepy",
      "rpaths"    : ["../.."],
      "prefix"    : python.ModulePrefix() + "/" + python.Version(),
      "ext"       : python.ModuleExtension(),
      "bldprefix" : python.Version(),
      "srcs"      : ["src/py/_gcore.cpp", "src/py/log.cpp", "src/py/pathenumerator.cpp"],
      "deps"      : ["gcore"],
      "custom"    : [RequireGcore, python.SoftRequire, python.SilentCythonWarnings],
      "install"   : {python.ModulePrefix(): ["src/py/gcore.py", "src/py/tests"]}
   },
   {  "name"    : "testmodule",
      "type"    : "dynamicmodule",
      "prefix"  : "bin",
      "srcs"    : ["src/tests/modules/module.cpp"]
   },
   {  "name"    : "gcore_tests",
      "type"    : "testprograms",
      "srcs"    : glob.glob("src/tests/*.cpp"),
      "deps"    : ["gcore", "testmodule"],
      "custom"  : [RequireGcore],
   }
]

env = excons.MakeBaseEnv()

# Setup cython
buildpy = ("gcorepy" in SCons.Script.BUILD_TARGETS or
           "_gcore" in SCons.Script.BUILD_TARGETS or
           "all" in SCons.Script.BUILD_TARGETS) 
if buildpy and python.RequireCython(env): 
  if excons.GetArgument("cython-gen", 1, int): 
    python.CythonGenerate(env, "src/py/_gcore.pyx", incdirs=["include"], cpp=True) 
  elif not os.path.isfile("src/py/_gcore.cpp") or not os.path.isfile("src/py/_gcore.h"): 
    print("Cannot build gcore python module: cython sources not generated") 
    sys.exit(1) 
else: 
  # Remove gcorepy target
  buildpy = False
  prjs = filter(lambda x: x.get("name", "") != "_gcore", prjs)

# Declare targets
excons.DeclareTargets(env, prjs)

SCons.Script.Alias("all", "gcore")
SCons.Script.Alias("all", "testmodule")
SCons.Script.Alias("all", "gcore_tests")
if buildpy:
  SCons.Script.Alias("all", "gcorepy")

SCons.Script.Default(["gcore"])

# MIT License
#
# Copyright (c) 2009 Gaetan Guidet
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

import re
import excons
import excons.tools
from excons.tools import threads
from excons.tools import dl
from excons.tools import python
import SCons.Script

# pylint: disable=bad-indentation,unused-argument,no-member


cython = excons.GetArgument("with-cython", "cython")
static = excons.GetArgument("static", 0, int)
debugrex = excons.GetArgument("debug-rex", 0, int)

libdefs = ["GCORE_STATIC"] if static else ["GCORE_EXPORTS"]
if debugrex:
  libdefs.append("_DEBUG_REX")
liblibs = []
libcustom = []
if not static:
  libcustom = [threads.Require, dl.Require]
  if not str(SCons.Script.Platform()) in ["win32", "darwin"]:
    liblibs = ["rt"]

def SilentCythonWarnings(env): # pylint: disable=redefined-outer-name
  if str(SCons.Script.Platform()) == "darwin":
    env.Append(CPPFLAGS=" -Wno-unused-function -Wno-unneeded-internal-declaration")

def RequireGcore(env): # pylint: disable=redefined-outer-name
  # Don't need to set CPPPATH, headers are now installed in output directory
  # Don't need to set LIBPATH, library output directory is automatically added by excons
  env.Append(LIBS=["gcore"])

  if static:
    env.Append(CPPDEFINES=["GCORE_STATIC"])
    threads.Require(env)
    dl.Require(env)

  if not str(SCons.Script.Platform()) in ["win32", "darwin"]:
    env.Append(LIBS=["rt"])

SCons.Script.Export("RequireGcore")


prjs = [
  { "name"         : "gcore",
    "type"         : "staticlib" if static else "sharedlib",
    "version"      : "0.3.4",
    "soname"       : "libgcore.so.0",
    "install_name" : "libgcore.0.dylib",
    "srcs"         : excons.glob("src/lib/*.cpp") + excons.glob("src/lib/rex/*.cpp"),
    "install"      : {"include": ["include/gcore", "include/half.h"]},
    "defs"         : libdefs,
    "custom"       : libcustom,
    "libs"         : liblibs
  },
  { "name"      : "_gcore",
    "type"      : "dynamicmodule",
    "alias"     : "gcorepy",
    "rpaths"    : ["../.."],
    "prefix"    : python.ModulePrefix() + "/" + python.Version(),
    "ext"       : python.ModuleExtension(),
    "bldprefix" : python.Version(),
    "srcs"      : ["src/py/_gcore.cpp", "src/py/log.cpp", "src/py/pathenumerator.cpp"],
    "deps"      : ["gcore"],
    "custom"    : [RequireGcore, python.SoftRequire, SilentCythonWarnings],
    "install"   : {python.ModulePrefix(): ["src/py/gcore.py", "src/py/tests"]}
  },
  { "name"    : "testmodule",
    "type"    : "dynamicmodule",
    "prefix"  : "bin",
    "srcs"    : ["src/tests/modules/module.cpp"]
  },
  { "name"    : "gcore_tests",
    "type"    : "testprograms",
    "srcs"    : excons.glob("src/tests/*.cpp"),
    "deps"    : ["gcore", "testmodule"],
    "custom"  : [RequireGcore],
  }
]

env = excons.MakeBaseEnv()

# Setup cython
cython_include_re = re.compile(r"^include\s+([\"'])(\S+)\1", re.MULTILINE)

def scan_cython_includes(node, _, path):
   if hasattr(node, "get_text_contents"):
      lst = [m[1] for m in cython_include_re.findall(node.get_text_contents())]
      return lst
   elif hasattr(node, "get_contents"):
      lst = [m[1] for m in cython_include_re.findall(str(node.get_contents()))]
      return lst
   else:
      return []

cython_scanner = SCons.Script.Scanner(function=scan_cython_includes, skeys=".pyx")

env.Append(SCANNERS=cython_scanner)

# Declare targets
excons.DeclareTargets(env, prjs)

# Generate cpp files from cythin
cygen = env.Command(["src/py/_gcore.cpp", "src/py/_gcore.h"], "src/py/_gcore.pyx", "%s -I include --cplus --embed-positions -o $TARGET $SOURCE" % cython)


SCons.Script.Default(["gcore"])

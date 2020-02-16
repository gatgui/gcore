import excons
import os
import sys
import glob
import excons.tools
from excons.tools import threads
from excons.tools import dl
from excons.tools import python


excons.InitGlobals()

static = excons.GetArgument("static", 0, int)
debugext = excons.GetArgument("debug-extra", 0, int)
debugrex = excons.GetArgument("debug-rex", 0, int)
plat = str(Platform())

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


def RequireGcore(env):
   # Don't need to set CPPPATH, headers are now installed in output directory
   # Don't need to set LIBPATH, library output directory is automatically added by excons
   env.Append(LIBS=["gcore"])

   if static:
     env.Append(CPPDEFINES=["GCORE_STATIC"])
     threads.Require(env)
     dl.Require(env)

   p = str(Platform())

   if not p in ["win32", "darwin"]:
     env.Append(LIBS=["rt"])
  
   if p == "win32":
     env.Append(CPPDEFINES=["_CRT_SECURE_NO_WARNINGS"])

Export("RequireGcore")


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
   {  "name"   : "gcore_utils",
      "type"   : "testprograms",
      "srcs"   : glob.glob("src/bin/*.cpp"),
      "deps"   : ["gcore"],
      "custom" : [RequireGcore]
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
buildpy = ("gcorepy" in BUILD_TARGETS or "_gcore" in BUILD_TARGETS or "all" in BUILD_TARGETS) 
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

Alias("all", "gcore")
Alias("all", "gcore_utils")
Alias("all", "testmodule")
Alias("all", "gcore_tests")
if buildpy:
  Alias("all", "gcorepy")

Default(["gcore"])

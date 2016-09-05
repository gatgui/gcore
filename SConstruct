import excons
import os
import re
import glob
import excons.tools
from excons.tools import threads
from excons.tools import dl
from excons.tools import python

cython = excons.GetArgument("with-cython", "cython")
static = excons.GetArgument("static", 0, int)
debugrex = excons.GetArgument("debug-rex", 0, int)

libdefs = ["GCORE_STATIC"] if static else ["GCORE_EXPORTS"]
if debugrex:
  libdefs.append("_DEBUG_REX")
if str(Platform()) == "win32":
  libdefs.append("_CRT_SECURE_NO_WARNINGS")
liblibs = []
libcustom = []
if not static:
  libcustom = [threads.Require, dl.Require]
  if not str(Platform()) in ["win32", "darwin"]:
    liblibs = ["rt"]

def SilentCythonWarnings(env):
  if str(Platform()) == "darwin":
    env.Append(CPPFLAGS=" -Wno-unused-function -Wno-unneeded-internal-declaration")

def RequireGcore(env):
  # Don't need to set CPPPATH, headers are now installed in output directory
  # Don't need to set LIBPATH, library output directory is automatically added by excons
  env.Append(LIBS=["gcore"])

  if static:
    env.Append(CPPDEFINES=["GCORE_STATIC"])
    threads.Require(env)
    dl.Require(env)

  plat = str(Platform())

  if not plat in ["win32", "darwin"]:
    env.Append(LIBS=["rt"])
  
  if plat == "win32":
    env.Append(CPPDEFINES=["_CRT_SECURE_NO_WARNINGS"])

Export("RequireGcore")


prjs = [
  { "name"         : "gcore",
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
    "srcs"    : glob.glob("src/tests/*.cpp"),
    "deps"    : ["gcore", "testmodule"],
    "custom"  : [RequireGcore],
  }
]

env = excons.MakeBaseEnv()

# Setup cython
cython_include_re = re.compile(r"^include\s+([\"'])(\S+)\1", re.MULTILINE)

def scan_cython_includes(node, env, path):
   if hasattr(node, "get_text_contents"):
      lst = [m[1] for m in cython_include_re.findall(node.get_text_contents())]
      return lst
   elif hasattr(node, "get_contents"):
      lst = [m[1] for m in cython_include_re.findall(str(node.get_contents()))]
      return lst
   else:
      return []

cython_scanner = Scanner(function=scan_cython_includes, skeys=".pyx")

env.Append(SCANNERS=cython_scanner)

# Declare targets
excons.DeclareTargets(env, prjs)

# Generate cpp files from cythin
cygen = env.Command(["src/py/_gcore.cpp", "src/py/_gcore.h"], "src/py/_gcore.pyx", "%s -I include --cplus --embed-positions -o $TARGET $SOURCE" % cython)


Default(["gcore"])

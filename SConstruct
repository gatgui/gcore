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
liblibs = []
libcustom = []
if not static:
  libcustom = [threads.Require, dl.Require]
  if not str(Platform()) in ["win32", "darwin"]:
    liblibs = ["rt"]


def RequireGcore(subdir=None):
  if subdir and type(subdir) in (str, unicode):
    if not (subdir.endswith("/") or subdir.endswith("\\")):
      subdir += "/"
  else:
    subdir = ""

  def _Require(env):
    env.Append(CPPPATH=[subdir+"include"])
    # Don't need to set LIBPATH, library output directory is automatically added by excons
    env.Append(LIBS=["gcore"])
    
    if static:
      env.Append(CPPDEFINES=["GCORE_STATIC"])
      threads.Require(env)
      dl.Require(env)

    if not str(Platform()) in ["win32", "darwin"]:
      env.Append(LIBS=["rt"])

  return _Require

Export("RequireGcore")


prjs = [
  { "name"         : "gcore",
    "type"         : "staticlib" if static else "sharedlib",
    "version"      : "0.3.0",
    "soname"       : "libgcore.so.0",
    "install_name" : "libgcore.0.dylib",
    "incdirs"      : ["include"],
    "srcs"         : glob.glob("src/lib/*.cpp") + glob.glob("src/lib/rex/*.cpp"),
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
    "custom"    : [RequireGcore(), python.SoftRequire],
    "install"   : {python.ModulePrefix(): ["src/py/gcore.py", "src/py/tests"]}
  },
  { "name"    : "testmodule",
    "type"    : "dynamicmodule",
    "prefix"  : "bin",
    "incdirs" : ["include"],
    "srcs"    : ["src/tests/modules/module.cpp"]
  },
  { "name"    : "gcore_tests",
    "type"    : "testprograms",
    "srcs"    : glob.glob("src/tests/*.cpp"),
    "deps"    : ["gcore", "testmodule"],
    "custom"  : [RequireGcore()],
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




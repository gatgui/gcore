import glob
import excons
import excons.tools
from excons.tools import threads
from excons.tools import dl

static = int(ARGUMENTS.get("static", "0"))
debugrex = int(ARGUMENTS.get("debugRex", "0"))

libdefs   = ["GCORE_STATIC"] if static else ["GCORE_EXPORT"]
if debugrex:
  libdefs.append("_DEBUG_REX")
libcustom = []
depcustom = []
liblibs   = []
deplibs   = []
if static:
  depcustom = [threads.Require, dl.Require]
  if not str(Platform()) in ["win32", "darwin"]:
    deplibs = ["rt"]
else:
  libcustom = [threads.Require, dl.Require]
  if not str(Platform()) in ["win32", "darwin"]:
    liblibs = ["rt"]

prjs = [
  { "name"    : "gcore",
    "type"    : "staticlib" if static else "sharedlib",
    "incdirs" : ["include"],
    "srcs"    : glob.glob("src/lib/*.cpp") + glob.glob("src/lib/rex/*.cpp"),
    "defs"    : libdefs,
    "custom"  : libcustom,
    "libs"    : liblibs
  },
  { "name"    : "testmodule",
    "type"    : "dynamicmodule",
    "prefix"  : "bin",
    "incdirs" : ["include"],
    "srcs"    : ["src/tests/modules/module.cpp"]
  },
  { "name"    : "gcore_tests",
    "type"    : "testprograms",
    "incdirs" : ["include"],
    "srcs"    : glob.glob("src/tests/*.cpp"),
    "defs"    : ["GCORE_STATIC"] if static else [],
    "libs"    : ["gcore"]+deplibs,
    "deps"    : ["testmodule"],
    "custom"  : depcustom,
  }
]

env = excons.MakeBaseEnv()
excons.DeclareTargets(env, prjs)





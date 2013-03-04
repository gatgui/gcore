import re
import glob
import excons
import excons.tools
from excons.tools import threads
from excons.tools import dl
from excons.tools import python

cython = ARGUMENTS.get("with-cython", "cython")
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
  { "name"      : "_gcore",
    "type"      : "dynamicmodule",
    "alias"     : "gcorepy",
    "prefix"    : python.ModulePrefix(),
    "ext"       : python.ModuleExtension(),
    "bldprefix" : python.Version(),
    "incdirs"   : ["include"],
    "defs"      : ["GCORE_STATIC"] if static else [],
    "srcs"      : ["src/py/_gcore.cpp", "src/py/log.cpp"],
    "libs"      : ["gcore"],
    "custom"    : [dl.Require, threads.Require, python.Require],
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




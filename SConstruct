import glob
import excons
import excons.tools
from excons.tools import threads
from excons.tools import dl

static = int(ARGUMENTS.get("static", "0"))

prjs = [
  { "name"    : "gcore",
    "type"    : "staticlib" if static else "sharedlib",
    "incdirs" : ["include"],
    "srcs"    : glob.glob("src/lib/*.cpp") + glob.glob("src/lib/regex/*.cpp"),
    "defs"    : ["GCORE_STATIC"] if static else ["GCORE_EXPORTS"],
    "custom"  : [threads.Require, dl.Require]
  },
  { "name"    : "testmodule",
    "type"    : "dynamicmodule",
    "prefix"  : "bin",
    "incdirs" : ["include"],
    "srcs"    : ["src/tests/modules/module.cpp"]
  },
  { "name"    : "tests",
    "type"    : "testprograms",
    "incdirs" : ["include"],
    "srcs"    : glob.glob("src/tests/*.cpp"),
    "defs"    : ["GCORE_STATIC"] if static else [],
    "libs"    : ["gcore"],
    "deps"    : ["testmodule"]
  }
]

env = excons.MakeBaseEnv()
excons.DeclareTargets(env, prjs)





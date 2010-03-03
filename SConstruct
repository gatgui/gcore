import glob
import excons
import excons.tools
from excons.tools import threads
from excons.tools import dl

prjs = [
  { "name"    : "gcore",
    "type"    : "sharedlib",
    "incdirs" : ["include"],
    "srcs"    : glob.glob("src/lib/*.cpp"),
    "defs"    : ["GCORE_EXPORTS"],
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
    "libs"    : ["gcore"],
    "deps"    : ["testmodule"]
  }
]

env = excons.MakeBaseEnv()
excons.DeclareTargets(env, prjs)





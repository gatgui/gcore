# 3.19 is the lowest version I use, it doesn't necessarily mean that I won't work on older version. Adjust and try as necessary.
cmake_minimum_required(VERSION 3.19)

project(gcore VERSION 1.0.0)

option(GCORE_BUILD_PYTHON "Build gcore python bindings" OFF)
option(GCORE_BUILD_TOOLS "Build gcore command line tools" OFF)
option(GCORE_BUILD_TESTS "Build gcore test programs" OFF)
option(GCORE_PARSER_VERBOSE "Enable xml/json parser debug messages" OFF)
option(GCORE_REX_VERBOSE "Enable regular expression engine debug messages " OFF)
option(LINK_STATIC_GCORE "Link tools, tests and python modules to static gcore library" ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

#include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_subdirectory(src)

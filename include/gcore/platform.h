/*
MIT License

Copyright (c) 2009 Gaetan Guidet

This file is part of gcore.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __gcore_platform_h_
#define __gcore_platform_h_

#ifdef _WIN32
# ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
# endif
# ifndef _WIN32_WINNT
#   define _WIN32_WINNT WINVER
# endif
//# define _WIN32_WINNT 0x0500
# include <windows.h>
//# include <winsock2.h> // gethostname, requires linking winsock32.lib
# define DIR_SEP '\\'
# define PATH_SEP ';'
#else
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/time.h>
# include <sys/stat.h>
# include <unistd.h>
# include <errno.h>
# include <fcntl.h>
# include <dlfcn.h>
# include <dirent.h>
# include <pthread.h>
//# include <semaphore.h>
# include <sched.h>
# include <signal.h>
# define DIR_SEP '/'
# define PATH_SEP ':'
# ifdef __APPLE__
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron())
# else
extern char **environ;
# endif
#endif

#endif


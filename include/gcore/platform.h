/*

Copyright (C) 2009  Gaetan Guidet

This file is part of gcore.

gcore is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

gcore is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

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
# include <windows.h>
# define DIR_SEP '\\'
# define PATH_SEP ';'
#else
# include <sys/wait.h>
# include <sys/time.h>
# include <unistd.h>
# include <errno.h>
# include <dlfcn.h>
# include <dirent.h>
# include <pthread.h>
//# include <semaphore.h>
# include <sched.h>
# include <signal.h>
# define DIR_SEP '/'
# define PATH_SEP ':'
#endif

#endif


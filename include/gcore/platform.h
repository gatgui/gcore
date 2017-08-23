/*

Copyright (C) 2009~  Gaetan Guidet

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
# define NOMINMAX
//# define _WIN32_WINNT 0x0500
# include <windows.h>
# include <winsock2.h>
//# pragma comment(lib, "wsock32.lib")
# pragma comment(lib, "ws2_32.lib")
# define NULL_SOCKET INVALID_SOCKET
# define sock_close closesocket
# define sock_errno WSAGetLastError
# ifndef SHUT_RD
#  define SHUT_RD SD_RECEIVE
# endif
# ifndef SHUT_WR
#  define SHUT_WR SD_SEND
# endif
# ifndef SHUT_RDWR
#  define SHUT_RDWR SD_BOTH
# endif
# define DIR_SEP '\\'
# define PATH_SEP ';'
# define std_errno GetLastError
typedef SOCKET sock_t;
typedef int socklen_t;
#else // _WIN32
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/time.h>
# include <sys/stat.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <sys/mman.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
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
# define NULL_SOCKET -1
# define sock_close   ::close
# define sock_errno() errno
# define std_errno() errno
# ifdef __APPLE__
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron())
# else // __APPLE__
extern char **environ;
# endif // __APPLE__
typedef int sock_t;
#endif // _WIN32

#endif


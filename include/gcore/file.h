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

#ifndef __gcore_file_h_
#define __gcore_file_h_

#include <gcore/callbacks.h>

namespace gcore {
 
  enum FileType {
    FT_FILE = 0,
    FT_DIR,
    FT_UNKNOWN,
    FT_MAX
  };
  
  GCORE_API std::string JoinPath(const std::string &path0, const std::string &path1);
  GCORE_API bool IsAbsolutePath(const std::string &path);
  GCORE_API std::string MakeAbsolutePath(const std::string &path);
  // Normalize path will make path absolute only if necessary
  GCORE_API std::string NormalizePath(const std::string &path);
  GCORE_API bool IsSamePath(const std::string &path0, const std::string &path1);
  
  GCORE_API std::string Basename(const std::string &path);
  GCORE_API std::string Dirname(const std::string &path);
  // file extension strings do not include the . character
  GCORE_API std::string FileExtension(const std::string &path);
  GCORE_API bool CheckFileExtension(const std::string &path, const std::string &ext);
  
  GCORE_API std::string GetCurrentDir();
  
  GCORE_API bool FileExists(const std::string &path);
  GCORE_API bool DirExists(const std::string &path);
  
  GCORE_API size_t FileSize(const std::string &path);
  
  GCORE_API bool CreateDir(const std::string &dir);
  
  // should return false to stop iteration
  typedef Callback3wR<bool, const std::string&, const std::string&, FileType> EnumFilesCallback;
  GCORE_API void ForEachInDir(const std::string &d, EnumFilesCallback cb, bool recurse=false);
  
  class FileList : public std::vector<std::string> {
    public:
      inline FileList() {
      }
      inline FileList(const FileList &rhs)
        : std::vector<std::string>(rhs) {
      }
      inline ~FileList() {
      }
      inline FileList& operator=(const FileList &rhs) {
        std::vector<std::string>::operator=(rhs);
        return *this;
      }
      inline bool enumerate(const std::string &dirname, const std::string &filename, FileType) {
        push_back(JoinPath(dirname, filename));
        return true;
      }
  };
  
  inline size_t ForEachInDir(const std::string &d, FileList &l, bool recurse=false) {
    EnumFilesCallback cb;
    MakeCallback(&l, METHOD(FileList, enumerate), cb);
    ForEachInDir(d, cb, recurse);
    return l.size();
  }
}



#endif


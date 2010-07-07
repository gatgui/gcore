/*

Copyright (C) 2009, 2010  Gaetan Guidet

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

#include <gcore/path.h>
#include <gcore/platform.h>

namespace gcore {
  
  Path::Path() {
  }
  
  Path::Path(const char *s) {
    operator=(s);
  }
  
  Path::Path(const String &s) {
    operator=(s);
  }
  
  Path::Path(const Path &rhs)
    : mPaths(rhs.mPaths), mFullName(rhs.mFullName) {
  }
  
  Path::~Path() {
  }
  
  Path& Path::operator=(const Path &rhs) {
    if (this != &rhs) {
      mPaths = rhs.mPaths;
      mFullName = rhs.mFullName;
    }
    return *this;
  }
  
  Path& Path::operator=(const String &s) {
    return operator=(s.c_str());
  }
  
  Path& Path::operator=(const char *s) {
    String tmp(s);
    tmp.replace('\\', '/');
    tmp.split('/', mPaths);
    size_t i = 0;
    while (i < mPaths.size()) {
      mPaths[i].strip();
      if (mPaths[i].length() == 0) {
#ifndef _WIN32
        if (i == 0) {
          ++i;
          continue;
        }
#endif
        mPaths.erase(mPaths.begin()+i);
      } else {
        ++i;
      }
    }
    mFullName = fullname(DIR_SEP);
    return *this;
  }
  
  Path& Path::operator+=(const Path &rhs) {
    if (isDir() && !rhs.isAbsolute()) {
      mPaths.insert(mPaths.end(), rhs.mPaths.begin(), rhs.mPaths.end());
      mFullName = fullname(DIR_SEP);
    }
    return *this;
  }
  
  bool Path::operator==(const Path &rhs) const {
    Path p0(*this);
    Path p1(rhs);
    
    p0.makeAbsolute().normalize();
    p1.makeAbsolute().normalize();
    
#ifdef _WIN32
    return (p0.mFullName.casecompare(p1.mFullName) == 0);
#else
    return (p0.mFullName.compare(p1.mFullName) == 0);
#endif
  }
  
  Path::operator const String& () const {
    return mFullName;
  }
  
  Path::operator String& () {
    return mFullName;
  }
  
  // can use negative numbers -> index from the end
  String& Path::operator[](int idx) {
    if (idx >= 0 && size_t(idx) < mPaths.size()) {
      return mPaths[idx];
    } else {
      idx = int(mPaths.size()) + idx;
      if (idx >= 0 && size_t(idx) < mPaths.size()) {
        return mPaths[idx];
      }
    }
    // nothing...
    static String _sEmpty;
    return _sEmpty;
  }
  
  const String& Path::operator[](int idx) const {
    if (idx >= 0 && size_t(idx) < mPaths.size()) {
      return mPaths[idx];
    } else {
      idx = int(mPaths.size()) + idx;
      if (idx >= 0 && size_t(idx) < mPaths.size()) {
        return mPaths[idx];
      }
    }
    // nothing...
    static String _sEmpty;
    return _sEmpty;
  }
  
  String Path::pop() {
    String rv;
    if (mPaths.size() > 0) {
      rv = mPaths.back();
      mPaths.pop_back();
      mFullName = fullname(DIR_SEP);
    }
    return rv;
  }
  
  Path& Path::push(const String &s) {
    mPaths.push_back(s);
    mFullName = fullname(DIR_SEP);
    return *this;
  }
  
  bool Path::isAbsolute() const {
    if (mPaths.size() == 0) {
      return false;
    }
#ifdef _WIN32
    return (mPaths[0].length() >= 2 && mPaths[0][1] == ':');
#else
    return (mPaths[0] == "");
#endif
  }
  
  // if path is relative, prepend current directory
  Path& Path::makeAbsolute() {
    if (!isAbsolute()) {
      Path cwd = GetCurrentDir();
      mPaths.insert(mPaths.begin(), cwd.mPaths.begin(), cwd.mPaths.end());
      mFullName = fullname(DIR_SEP);
    }
    return *this;
  }
  
  // remove any . or .. and make absolute if necessary
  Path& Path::normalize() {
    size_t i = 0;
    bool wasMadeAbsolute = isAbsolute();
    while (i < mPaths.size()) {
      if (mPaths[i] == ".") {
        mPaths.erase(mPaths.begin()+i);
      } else if (mPaths[i] == "..") {
        if (i == 0) {
          if (wasMadeAbsolute) {
            // invalid path
            mPaths.clear();
            break;
          }
          Path cwd = GetCurrentDir();
          cwd.pop();
          size_t sz = cwd.mPaths.size();
          mPaths.erase(mPaths.begin()+i);
          mPaths.insert(mPaths.begin(), cwd.mPaths.begin(), cwd.mPaths.end());
          i = sz;
          wasMadeAbsolute = true;
        } else {
          mPaths.erase(mPaths.begin()+i);
          mPaths.erase(mPaths.begin()+i-1);
          --i;
        }
      } else {
        ++i;
      }
    }
    mFullName = fullname(DIR_SEP);
    return *this;
  }
  
  String Path::basename() const {
    return (mPaths.size() == 0 ? "" : mPaths[mPaths.size()-1]);
  }
  
  String Path::dirname(char sep) const {
    if (mPaths.size() == 0) {
      return "";
    }
    Path pdir(*this);
    pdir.pop();
    String tmp(sep);
    return tmp.join(pdir.mPaths);
  }
  
  String Path::fullname(char sep) const {
    String tmp(sep);
    return tmp.join(mPaths);
  }
  
  bool Path::isDir() const {
#ifdef _WIN32
    DWORD fa;
    fa = GetFileAttributes(mFullName.c_str());
    if (fa != 0xFFFFFFFF) {
      return ((fa & FILE_ATTRIBUTE_DIRECTORY) != 0);
    }
#else
    struct stat st;
    if (stat(mFullName.c_str(), &st) == 0) {
      return ((st.st_mode & S_IFDIR) != 0);
    }
#endif
    return false;
  }
  
  bool Path::isFile() const {
#ifdef _WIN32
    DWORD fa;
    fa = GetFileAttributes(mFullName.c_str());
    if (fa != 0xFFFFFFFF) {
      return ((fa & FILE_ATTRIBUTE_DIRECTORY) == 0);
    }
#else
    struct stat st;
    if (stat(mFullName.c_str(), &st) == 0) {
      return ((st.st_mode & S_IFREG) != 0);
    }
#endif
    return false;
  }
  
  bool Path::exists() const {
#ifdef _WIN32
    DWORD fa;
    fa = GetFileAttributes(mFullName.c_str());
    if (fa != 0xFFFFFFFF) {
      return true;
    }
#else
    struct stat st;
    if (stat(mFullName.c_str(), &st) == 0) {
      return true;
    }
#endif
    return false;
  }
  
  // file extension without .
  String Path::getExtension() const {
    if (mPaths.size() == 0) {
      return "";
    }
    size_t p0 = mPaths[mPaths.size()-1].rfind('.');
    if (p0 != std::string::npos) {
      return mPaths[mPaths.size()-1].substr(p0+1);
    } else {
      return "";
    }
  }
  
  bool Path::checkExtension(const String &ext) const {
    String pext = getExtension();
    return (pext.casecompare(ext) == 0);
  }
  
  size_t Path::fileSize() const {
    if (isFile()) {
      struct stat fileStat;
      if (stat(mFullName.c_str(), &fileStat) == 0) {
        return fileStat.st_size;
      }
    }
    return 0;
  }
  
  bool Path::createDir(bool recursive) const {
    if (exists()) {
      return true;
    }
    if (mPaths.size() == 0) {
      return false;
    }
    if (recursive) {
      Path pdir(*this);
      pdir.pop();
      if (!pdir.createDir(recursive)) {
        return false;
      }
    }
#ifdef _WIN32
    return (CreateDirectory(mFullName.c_str(), NULL) == TRUE);
#else
    return (mkdir(mFullName.c_str(), S_IRWXU) == 0);
#endif
  }
  
  bool Path::removeFile() const {
    if (isFile()) {
      return (remove(mFullName.c_str()) == 0);
    }
    return false;
  }
  
  void Path::each(EachFunc callback, bool includeSubDirs) const {
    if (!isDir() || callback == 0) {
      return;
    }
    Path path(*this);
#ifdef _WIN32
    WIN32_FIND_DATA fd;
    HANDLE hFile;
    String fffs; // find first file string 
    if (mFullName.length() == 0) {
      fffs = ".\\*.*";
    } else {
      size_t p = mFullName.find("*.*");
      if (p == std::string::npos) {
        if (mFullName[mFullName.length()-1] == '\\' || mFullName[mFullName.length()-1] == '/') {
          fffs = mFullName + "*.*";
        } else {
          fffs = mFullName + "\\*.*";
        }
      } else {
        fffs = mFullName;
      }
    }
    hFile = FindFirstFile(fffs.c_str(), &fd);
    if (hFile != INVALID_HANDLE_VALUE) {
      do {
        String fname = fd.cFileName;
        if (fname == "." ||  fname == "..") {
          continue;
        }
        path.push(fname);
        if (!callback(path)) {
          break;
        }
        if (includeSubDirs && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
          path.each(callback, true);
        }
        path.pop();
      } while (FindNextFile(hFile, &fd));
      FindClose(hFile);
    }
#else
    DIR *d;
    if (mFullName.length() == 0) {
      Path cwd = GetCurrentDir();
      d = opendir(cwd.fullname(DIR_SEP).c_str());
    } else {
      d = opendir(mFullName.c_str());
    }
    if (d) {
      struct dirent *de;
      while ((de = readdir(d)) != 0) {
        String fname = de->d_name;
        if (fname == "." || fname == "..") {
          continue;
        }
        path.push(fname);
        if (!callback(path)) {
          break;
        }
        if (includeSubDirs && path.isDir()) {
          path.each(callback, true);
        }
        path.pop();
      }
      closedir(d);
    }
#endif
  }
  
  namespace details {
    class DirLister {
      public:
        DirLister(PathList &l)
          : mLst(l) {
        }
        bool dirItem(const Path &p) {
          mLst.push_back(p);
          return true;
        }
      private:
        DirLister();
        DirLister& operator=(const DirLister&);
      protected:
        PathList &mLst;
    };
  }
  
  size_t Path::listDir(PathList &l, bool includeSubDirs) const {
    EachFunc func;
    details::DirLister dl(l);
    Bind(&dl, &details::DirLister::dirItem, func);
    l.clear();
    each(func, includeSubDirs);
    return l.size();
  }
  
  Path Path::GetCurrentDir() {
#ifdef _WIN32
    DWORD cwdLen = GetCurrentDirectory(0, NULL);
    char *cwd = (char*)malloc(cwdLen * sizeof(char));
    GetCurrentDirectory(cwdLen, cwd);
#else
    char *cwd = getcwd((char*)NULL, 0);
#endif
    Path rv = Path(cwd);
    free(cwd);
    return rv;
  }
  
  /*
#ifdef _WIN32
  static void NormalizeSeparators(std::string &path) {
    size_t p0 = 0;
    size_t p1 = path.find('/', p0);
    while (p1 != std::string::npos) {
      path[p1] = '\\';
      p0 = p1 + 1;
      p1 = path.find('/', p0);
    } 
  }
#else
  static void NormalizeSeparators(std::string &) {
  }
#endif 
  
  bool FileExists(const std::string &path) {
#ifdef _WIN32
    DWORD fa;
    fa = GetFileAttributes(path.c_str());
    if (fa != 0xFFFFFFFF) {
      return ((fa & FILE_ATTRIBUTE_DIRECTORY) == 0);
    }
#else
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
      return ((st.st_mode & S_IFREG) != 0);
    }
#endif
    return false; 
  }
  
  bool DirExists(const std::string &path) {
#ifdef _WIN32
    DWORD fa;
    fa = GetFileAttributes(path.c_str());
    if (fa != 0xFFFFFFFF) {
      return ((fa & FILE_ATTRIBUTE_DIRECTORY) != 0);
    }
#else
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
      return ((st.st_mode & S_IFDIR) != 0);
    }
#endif
    return false;
  }
  
  size_t FileSize(const std::string &path) {
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == 0) {
      return fileStat.st_size;
    }
    return (size_t)-1;
  }

  bool CreateDir(const std::string &dir) {
    if (! DirExists(dir)) {
#ifdef _WIN32
      return (CreateDirectory(dir.c_str(), NULL) == TRUE);
#else
      return (mkdir(dir.c_str(), S_IRWXU) == 0);
#endif
    }
    return true;
  }
  
  bool CreateDirs(const std::string &dir) {
    if (! DirExists(dir)) {
      if (CreateDirs(Dirname(dir))) {
        return CreateDir(dir);
      }
      return false;
    }
    return true;
  }
  
  bool RemoveFile(const std::string &path) {
    if (FileExists(path)) {
      return (remove(path.c_str()) == 0);
    }
    return true;
  }
  
  std::string GetCurrentDir() {
#ifdef _WIN32
    DWORD cwdLen = GetCurrentDirectory(0, NULL);
    char *cwd = (char*)malloc(cwdLen * sizeof(char));
    GetCurrentDirectory(cwdLen, cwd);
#else
    char *cwd = getcwd((char*)NULL, 0);
#endif
    std::string rv(cwd);
    free(cwd);
    return rv;
  }
  
  std::string FileExtension(const std::string &path) {
    size_t p0 = path.rfind('.');
    if (p0 != std::string::npos) {
#ifdef _WIN32
      size_t p1 = path.find_last_of("\\/");
#else
      size_t p1 = path.rfind('/');
#endif
      if (p1 != std::string::npos && p0 < p1) {
        return "";
      }
      return path.substr(p0+1);
    } else {
      return "";
    }
  }
  
  bool CheckFileExtension(const std::string &path, const std::string &ext) {
    std::string pext = FileExtension(path);
#ifdef _WIN32
#	if _MSC_VER >= 1400
    return (_stricmp(pext.c_str(), ext.c_str()) == 0);
#	else
    return (stricmp(pext.c_str(), ext.c_str()) == 0);
#	endif
#else
    return (strcasecmp(pext.c_str(), ext.c_str()) == 0);
#endif
  }
  
  std::string JoinPath(const std::string &path0, const std::string &path1) {
    size_t l0 = path0.length();
    size_t l1 = path1.length();
    if (l0 == 0 || l1 == 0) {
      return "";
    }
    bool addSep = false;
    size_t lj = l0 + l1;
#ifdef _WIN32
    if (path0[l0-1] != '/' && path0[l0-1] != '\\') {
#else
    if (path0[l0-1] != '/') {
#endif
      addSep = true;
      lj += 1;
    }
#ifdef _WIN32
    if (path1[0] == '/' || path1[0] == '\\') {
#else
    if (path1[0] == '/') {
#endif
      addSep = false;
      lj -= 1;
    }
    std::string path = path0;
    if (addSep) {
      path.push_back(DIR_SEP);
    }
    path += path1;
    return path;
  }
  
  bool IsAbsolutePath(const std::string &path) {
    if (path.length() == 0) {
      return false;
    }
#ifdef _WIN32
    if (path.length() < 2) {
      return false;
    }
    return (path[0] != '\0' && path[1] == ':');
#else
    return (path[0] == '/');
#endif
  }
  
  std::string MakeAbsolutePath(const std::string &path) {
    if (IsAbsolutePath(path) == true) {
      return path;
    } else {
      return JoinPath(GetCurrentDir(), path);
    }
  }
  
  std::string NormalizePath(const std::string &path) {
    std::string npath;
    size_t p0 = 0;
#ifdef _WIN32
    size_t p1 = path.find_first_of("/\\");
#else
    size_t p1 = path.find('/');
#endif
    while (p1 != std::string::npos) {
      if (p0 != p1) {
        std::string tmp = path.substr(p0, p1-p0);
        if (tmp == "..") {
          if (npath.length() == 0) {
            npath = GetCurrentDir();
          }
#ifdef _WIN32
          size_t p2 = npath.find_last_of("/\\");
#else
          size_t p2 = npath.rfind('/');
#endif
          if (p2 != std::string::npos) {
            npath.erase(p2);
          } else {
            npath = "";
          }
        } else if (tmp != ".") {
          if (npath.length() > 1) {
            npath.push_back(DIR_SEP);
          }
          npath += tmp;
        }
      } else {
        npath.push_back(path[p0]);
      }
      p0 = p1 + 1;
#ifdef _WIN32
      p1 = path.find_first_of("/\\", p0);
#else
      p1 = path.find('/', p0);
#endif
    }
    if (p0 < path.length()) {
      // remaining chars
      std::string tmp = path.substr(p0);
      if (tmp == "..") {
        if (npath.length() == 0) {
          npath = GetCurrentDir();
        }
#ifdef _WIN32
        size_t p2 = npath.find_last_of("/\\");
#else
        size_t p2 = npath.rfind('/');
#endif
        if (p2 != std::string::npos) {
          npath.erase(p2);
        } else {
          npath = "";
        }
      } else if (tmp != ".") {
        if (npath.length() > 0) {
          npath.push_back(DIR_SEP);
        }
        npath += tmp;
      }
    }
    return npath;
  }
  
  std::string Basename(const std::string &path) {
    size_t p;
#ifdef _WIN32
    p = path.find_last_of("\\/");
#else
    p = path.rfind('/');
#endif
    if (p == std::string::npos) {
      return "";
    }
    return path.substr(p+1);
  }
  
  std::string Dirname(const std::string &path) {
    size_t p;
#ifdef _WIN32
    p = path.find_last_of("\\/");
#else
    p = path.rfind('/');
#endif
    if (p == std::string::npos) {
      return "";
    }
    return path.substr(0, p);
  }
  
  bool IsSamePath(const std::string &path0, const std::string &path1) {
    size_t l0 = path0.length();
    size_t l1 = path1.length();
    if (l0 == 0 && l1 == 0) {
      return true;
    }
    std::string apath0, apath1;
    if (l0 == 0) {
      apath0 = GetCurrentDir();
    } else {
      apath0 = NormalizePath(MakeAbsolutePath(path0));
    }
    if (l1 == 0) {
      apath1 = GetCurrentDir();
    } else {
      apath1 = NormalizePath(MakeAbsolutePath(path1));
    }
    NormalizeSeparators(apath0);
    NormalizeSeparators(apath1);
#ifdef _WIN32
#	if _MSC_VER >= 1400
    return (_stricmp(apath0.c_str(), apath1.c_str()) == 0);
#	else
    return (stricmp(apath0.c_str(), apath1.c_str()) == 0);
#	endif
#else
    return (strcmp(apath0.c_str(), apath1.c_str()) == 0);
#endif
  }
  
  void ForEachInDir(const std::string &path, EnumFilesCallback callback, bool recurse) {
    if (callback == 0) {
      return;
    }
#ifdef _WIN32
    WIN32_FIND_DATA fd;
    HANDLE hFile;
    std::string fffs; // find first file string 
    if (path.length() == 0) {
      fffs = ".\\*.*";
    } else {
      size_t p = path.find("*.*");
      if (p == std::string::npos) {
        if (path[path.length()-1] == '\\' || path[path.length()-1] == '/') {
          fffs = path + "*.*";
        } else {
          fffs = path + "\\*.*";
        }
      } else {
        fffs = path;
      }
    }
    hFile = FindFirstFile(fffs.c_str(), &fd);
    if (hFile != INVALID_HANDLE_VALUE) {
      do {
        std::string fname = fd.cFileName;
        if (fname == "." ||  fname == "..") {
          continue;
        }
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
          if (!callback(path, fname, FT_DIR)) {
            break;
          }
          if (recurse) {
            std::string tmp = JoinPath(path, fname);
            ForEachInDir(tmp, callback, true);
          }
        } else {
          if (!callback(path, fname, FT_FILE)) {
            break;
          }
        }
      } while (FindNextFile(hFile, &fd));
      FindClose(hFile);
    }
#else
    DIR *d;
    if (path.length() == 0) {
      d = opendir(GetCurrentDir().c_str());
    } else {
      d = opendir(path.c_str());
    }
    if (d) {
      struct dirent *de;
      struct stat st;
      while ((de = readdir(d)) != 0) {
        std::string fname = de->d_name;
        if (fname == "." || fname == "..") {
          continue;
        }
        std::string tmp = JoinPath(path, fname);
        if (stat(tmp.c_str(), &st) == 0) {
          if ((st.st_mode & S_IFREG) != 0) {
            if (!callback(path, fname, FT_FILE)) {
              break;
            }
          } else if ((st.st_mode & S_IFDIR) != 0) {
            if (!callback(path, fname, FT_DIR)) {
              break;
            }
            if (recurse == true) {
              ForEachInDir(tmp, callback, true);
            }
          }
        } else {
          std::cerr << "*** Could not stat \"" << tmp << "\"" << std::endl;
        }
      }
      closedir(d);
    }
#endif
  }
  */
}

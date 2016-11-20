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

#include <gcore/path.h>
#include <gcore/platform.h>
#include <gcore/dirmap.h>
#include <gcore/unicode.h>

namespace gcore
{

namespace details
{

class DirLister
{
public:
   DirLister(PathList &l)
      : mLst(l)
   {
   }
   bool dirItem(const Path &p)
   {
      mLst.push_back(p);
      return true;
   }
private:
   DirLister();
   DirLister& operator=(const DirLister&);
protected:
   PathList &mLst;
};
   
} // details


Path::Path()
{
}

Path::Path(const char *s)
{
   operator=(s);
}

Path::Path(const wchar_t *ws)
{
   operator=(ws);
}

Path::Path(const String &s)
{
   operator=(s);
}

Path::Path(const Path &rhs)
   : mPaths(rhs.mPaths)
   , mFullName(rhs.mFullName)
#ifdef _WIN32
   , mFullNameW(rhs.mFullNameW)
#else
   , mFullNameL(rhs.mFullNameL)
#endif
{
}

Path::~Path() {
}

Path& Path::operator=(const Path &rhs)
{
   if (this != &rhs)
   {
      mPaths = rhs.mPaths;
      mFullName = rhs.mFullName;
#ifdef _WIN32
      mFullNameW = rhs.mFullNameW;
#else
      mFullNameL = rhs.mFullNameL;
#endif
   }
   return *this;
}

Path& Path::operator=(const String &s)
{
   return operator=(s.c_str());
}

Path& Path::operator=(const char *s)
{
   String tmp(s);
   //tmp = Dirmap::Map(tmp);
   tmp.replace('\\', '/');
   tmp.split('/', mPaths);
   size_t i = 0;
   while (i < mPaths.size())
   {
      mPaths[i].strip();
      if (mPaths[i].length() == 0)
      {
         if (i == 0)
         {
            ++i;
            continue;
         }
         mPaths.erase(mPaths.begin()+i);
      }
      else
      {
         ++i;
      }
   }
   _updateFullName();
   return *this;
}

Path& Path::operator=(const wchar_t *ws)
{
   String tmp(ws);
   //tmp = Dirmap::Map(tmp);
   tmp.replace('\\', '/');
   tmp.split('/', mPaths);
   size_t i = 0;
   while (i < mPaths.size())
   {
      mPaths[i].strip();
      if (mPaths[i].length() == 0)
      {
         if (i == 0)
         {
            ++i;
            continue;
         }
         mPaths.erase(mPaths.begin()+i);
      }
      else
      {
         ++i;
      }
   }
   _updateFullName();
   return *this;
}

Path& Path::operator+=(const Path &rhs)
{
   if (isDir() && !rhs.isAbsolute())
   {
      mPaths.insert(mPaths.end(), rhs.mPaths.begin(), rhs.mPaths.end());
      _updateFullName();
   }
   return *this;
}

bool Path::operator==(const Path &rhs) const
{
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

Path::operator const String& () const
{
   return mFullName;
}

Path::operator String& ()
{
   return mFullName;
}

// can use negative numbers -> index from the end
String& Path::operator[](int idx)
{
   if (idx >= 0 && size_t(idx) < mPaths.size())
   {
      return mPaths[idx];
   }
   else
   {
      idx = int(mPaths.size()) + idx;
      if (idx >= 0 && size_t(idx) < mPaths.size())
      {
         return mPaths[idx];
      }
   }
   // nothing...
   static String _sEmpty;
   return _sEmpty;
}

const String& Path::operator[](int idx) const
{
   if (idx >= 0 && size_t(idx) < mPaths.size())
   {
      return mPaths[idx];
   }
   else
   {
      idx = int(mPaths.size()) + idx;
      if (idx >= 0 && size_t(idx) < mPaths.size())
      {
         return mPaths[idx];
      }
   }
   // nothing...
   static String _sEmpty;
   return _sEmpty;
}

void Path::_updateFullName()
{
   mFullName = fullname('/');
#ifdef _WIN32
   mFullNameW.clear();
#else
   mFullNameL.clear();
#endif
}

void Path::_updateInternals() const
{
#ifdef _WIN32
   if (mFullNameW.empty())
   {
      DecodeUTF8(mFullName.c_str(), mFullNameW);
   }
#else
   if (mFullNameL.empty())
   {
      UTF8ToLocale(mFullName.c_str(), mFullNameL);
   }
#endif
}

String Path::pop()
{
   String rv;
   if (mPaths.size() > 0)
   {
      rv = mPaths.back();
      mPaths.pop_back();
      _updateFullName();
   }
   return rv;
}

Path& Path::push(const String &s)
{
   mPaths.push_back(s);
   _updateFullName();
   return *this;
}

bool Path::isAbsolute() const
{
   if (mPaths.size() == 0)
   {
      return false;
   }
#ifdef _WIN32
   return (mPaths[0].length() >= 2 && mPaths[0][1] == ':');
#else
   return (mPaths[0] == "");
#endif
}

// if path is relative, prepend current directory
Path& Path::makeAbsolute()
{
   if (!isAbsolute())
   {
      Path cwd = CurrentDir();
      mPaths.insert(mPaths.begin(), cwd.mPaths.begin(), cwd.mPaths.end());
      _updateFullName();
   }
   return *this;
}

// remove any . or .. and make absolute if necessary
Path& Path::normalize()
{
   size_t i = 0;
   bool wasMadeAbsolute = isAbsolute();
   while (i < mPaths.size())
   {
      if (mPaths[i] == ".")
      {
         mPaths.erase(mPaths.begin()+i);
      }
      else if (mPaths[i] == "..")
      {
         if (i == 0)
         {
            if (wasMadeAbsolute)
            {
               // invalid path
               mPaths.clear();
               break;
            }
            Path cwd = CurrentDir();
            cwd.pop();
            size_t sz = cwd.mPaths.size();
            mPaths.erase(mPaths.begin()+i);
            mPaths.insert(mPaths.begin(), cwd.mPaths.begin(), cwd.mPaths.end());
            i = sz;
            wasMadeAbsolute = true;
         }
         else
         {
            mPaths.erase(mPaths.begin()+i);
            mPaths.erase(mPaths.begin()+i-1);
            --i;
         }
      }
      else
      {
         ++i;
      }
   }
   _updateFullName();
   return *this;
}

String Path::basename() const
{
   return (mPaths.size() == 0 ? "" : mPaths[mPaths.size()-1]);
}

String Path::dirname(char sep) const
{
   if (mPaths.size() == 0)
   {
      return "";
   }
   Path pdir(*this);
   pdir.pop();
   String tmp(sep);
   return tmp.join(pdir.mPaths);
}

String Path::fullname(char sep) const
{
   String tmp(sep);
   return tmp.join(mPaths);
}

Date Path::lastModification() const
{
   Date lm;
   _updateInternals();
#ifdef _WIN32
   struct __stat64 st;
   if (_wstat64(mFullNameW.c_str(), &st) == 0)
#else
   struct stat st;
   if (stat(mFullNameL.c_str(), &st) == 0)
#endif
   {
      lm.set(st.st_mtime, false);
   }
   else
   {
      lm.set(0);
   }
   return lm;
}

bool Path::isDir() const
{
   _updateInternals();
#ifdef _WIN32
   DWORD fa;
   fa = GetFileAttributesW(mFullNameW.c_str());
   if (fa != 0xFFFFFFFF)
   {
      return ((fa & FILE_ATTRIBUTE_DIRECTORY) != 0);
   }
#else
   struct stat st;
   if (stat(mFullNameL.c_str(), &st) == 0)
   {
      return ((st.st_mode & S_IFDIR) != 0);
   }
#endif
   return false;
}

bool Path::isFile() const
{
   _updateInternals();
#ifdef _WIN32
   DWORD fa;
   fa = GetFileAttributesW(mFullNameW.c_str());
   if (fa != 0xFFFFFFFF)
   {
      return ((fa & FILE_ATTRIBUTE_DIRECTORY) == 0);
   }
#else
   struct stat st;
   if (stat(mFullNameL.c_str(), &st) == 0)
   {
      return ((st.st_mode & S_IFREG) != 0);
   }
#endif
   return false;
}

bool Path::exists() const
{
   _updateInternals();
#ifdef _WIN32
   DWORD fa;
   fa = GetFileAttributesW(mFullNameW.c_str());
   if (fa != 0xFFFFFFFF)
   {
      return true;
   }
#else
   struct stat st;
   if (stat(mFullNameL.c_str(), &st) == 0)
   {
      return true;
   }
#endif
   return false;
}

// file extension without .
String Path::extension() const
{
   if (mPaths.size() == 0)
   {
      return "";
   }
   size_t p0 = mPaths[mPaths.size()-1].rfind('.');
   if (p0 != String::npos)
   {
      return mPaths[mPaths.size()-1].substr(p0+1);
   }
   else
   {
      return "";
   }
}

bool Path::checkExtension(const String &ext) const
{
   String pext = extension();
   return (pext.casecompare(ext) == 0);
}

bool Path::setExtension(const String &ext)
{
   if (mPaths.size() == 0)
   {
      return false;
   }
   
   size_t bni = mPaths.size() - 1;
   size_t p = mPaths[bni].rfind('.');
   
   if (p != String::npos)
   {
      if (ext.length() == 0)
      {
         mPaths[bni] = mPaths[bni].substr(0, p);
      }
      else
      {
         mPaths[bni] = mPaths[bni].substr(0, p + 1) + ext;
      }
   }
   else
   {
      // current file path has no extension
      if (ext.length() > 0)
      {
         mPaths[bni] += "." + ext;
      }
      else
      {
         return true;
      }
   }
   
   _updateFullName();
   
   return true;
}

size_t Path::fileSize() const
{
   if (isFile())
   {
      _updateInternals();
#ifdef _WIN32
      struct __stat64 fileStat;
      if (_wstat64(mFullNameW.c_str(), &fileStat) == 0)
#else
      struct stat fileStat;
      if (stat(mFullNameL.c_str(), &fileStat) == 0)
#endif
      {
         return fileStat.st_size;
      }
   }
   return 0;
}

Status Path::createDir(bool recursive) const
{
   if (exists())
   {
      return Status(true);
   }
   if (mPaths.size() == 0)
   {
      return Status(false, "gcore::Path::createDir: Empty directory name.");
   }
   if (recursive)
   {
      Path pdir(*this);
      pdir.pop();
      Status stat = pdir.createDir(recursive);
      if (!stat)
      {
         return stat;
      }
   }
   _updateInternals();
#ifdef _WIN32
   if (CreateDirectoryW(mFullNameW.c_str(), NULL) == TRUE)
#else
   if (mkdir(mFullNameL.c_str(), S_IRWXU) == 0)
#endif
   {
      return Status(true);
   }
   else
   {
      return Status(false, std_errno(), "gcore::Path::createDir: Failed to create directory '%s'.", mFullName.c_str());
   }
}

Status Path::_removeDir(bool recursive) const
{
   if (isDir())
   {
      if (!recursive)
      {
         _updateInternals();
#ifdef _WIN32
         if (RemoveDirectoryW(mFullNameW.c_str()) == TRUE)
#else
         if (rmdir(mFullNameL.c_str()) == 0)
#endif
         {
            return Status(true);
         }
         else
         {
            return Status(false, std_errno(), "gcore::Path::_removeDir: Failed to remove directory '%s'.", mFullName.c_str());
         }
      }
      else
      {
         Status stat;
         List<Path> contents;
         size_t n = listDir(contents);
         
         for (size_t i=0; i<n; ++i)
         {
            stat = contents[i].remove(true);
            if (!stat)
            {
               return stat;
            }
         }
         
         return Status(true);
      }
   }
   else
   {
      return Status(false, "gcore::Path::_removeDir: '%s' is not a valid directory.", mFullName.c_str());
   }
}

Status Path::_copyDir(const Path &to, bool recursive, bool createMissingDirs, bool overwrite) const
{
   if (isDir())
   {
      Status stat;
      
      if (!to.exists())
      {
         if (!createMissingDirs)
         {
            return Status(false, "gcore::Path::_copyDir: Destination '%s' doesn't exist.", to.fullname('/').c_str());
         }
         else
         {
            stat = to.createDir(true);
            if (!stat)
            {
               return stat;
            }
         }
      }
      else
      {
         if (to.isFile())
         {
            return Status(false, "gcore::Path::_copyDir: Destination '%s' is a file.", to.fullname('/').c_str());
         }
      }
      
      Path dst(to);
      dst.push(basename());
      
      if (dst.exists())
      {
         if (!overwrite)
         {
            return Status(false, "gcore::Path::_copyDir: Destination directory '%s' already exists.", dst.fullname('/').c_str());
         }
         else if (!dst.isDir())
         {
            return Status(false, "gcore::Path::_copyDir: Destination '%s' is not a directory.", dst.fullname('/').c_str());
         }
      }
      else
      {
         stat = dst.createDir(false);
         if (!stat)
         {
            return stat;
         }
      }
      
      List<Path> contents;
      size_t n = listDir(contents);
      
      for (size_t i=0; i<n; ++i)
      {
         if (contents[i].isDir() && !recursive)
         {
            // don't even create empty directory
            continue;
         }
         stat = contents[i].copy(dst, recursive, false, overwrite);
         if (!stat)
         {
            return stat;
         }
      }
      
      return Status(true);
   }
   else
   {
      return Status(false, "gcore::Path::_copyDir: '%s' is not a valid directory.", mFullName.c_str());
   }
}

Status Path::_removeFile() const
{
   if (isFile())
   {
      _updateInternals();
#ifdef _WIN32
      if (DeleteFileW(mFullNameW.c_str()) == TRUE)
#else
      if (::remove(mFullNameL.c_str()) == 0)
#endif
      {
         return Status(true);
      }
      else
      {
         return Status(false, std_errno(), "gcore::Path::_removeFile: '%s' is not a file.", mFullName.c_str());
      }
   }
   else
   {
      return Status(false, "gcore::Path::_removeFile: '%s' is not a valid file.", mFullName.c_str());
   }
}

Status Path::_copyFile(const Path &to, bool createMissingDirs, bool overwrite) const
{
   if (isFile())
   {
      Path toDir(to);
      Path toFile(to);
      
      if (to.exists())
      {
         if (to.isFile())
         {
            if (!overwrite)
            {
               return Status(false, "gcore::Path::_copyFile: '%s' already exists.", to.fullname('/').c_str());
            }
            else
            {
               toDir.pop();
            }
         }
         else
         {
            // append basename to 'to'
            toFile.push(basename());
            
            if (toFile.isFile())
            {
               if (!overwrite)
               {
                  return Status(false, "gcore::Path::_copyFile: '%s' already exists.", toFile.fullname('/').c_str());
               }
            }
            else if (toFile.isDir())
            {
               return Status(false, "gcore::Path::_copyFile: Directory with the same name '%s' already exists.", toFile.fullname('/').c_str());
            }
         }
      }
      else
      {
         toDir.pop();
         
         if (!toDir.exists())
         {
            if (!createMissingDirs)
            {
               return Status(false, "gcore::Path::_copyFile: Destination's directory '%s' doesn't exist.", toDir.fullname('/').c_str());
            }
            else
            {
               Status stat = toDir.createDir(true);
               if (!stat)
               {
                  return stat;
               }
            }
         }
         else if (toDir.isFile())
         {
            return Status(false, "gcore::Path::_copyFile: '%s' is not a directory.", toDir.fullname('/').c_str());
         }
      }
      
      Status stat(true);
      FILE *src = open("rb");
      FILE *dst = toFile.open("wb");
      
      // Copy by chunks or 8 Mb
      size_t chunkSize = 8 * 1024 * 1024;
      unsigned char *chunk = (unsigned char*) malloc(chunkSize);
      
      while (!feof(src))
      {
         size_t readSize = fread(chunk, 1, chunkSize, src);
         if (readSize > 0)
         {
            fwrite(chunk, 1, readSize, dst);
         }
         if (ferror(src))
         {
            stat = Status(false, "gcore::Path::_copyFile: Failed to copy file.", toFile.fullname('/').c_str());
            break;
         }
      }
      
      free(chunk);
      fclose(src);
      fclose(dst);
      
      return stat;
   }
   else
   {
      return Status(false, "gcore::Path::_copyFile: '%s' is not a valid file.", mFullName.c_str());
   }
}

void Path::forEach(ForEachFunc callback, bool recurse, unsigned short flags) const
{
   if (!isDir() || callback == 0)
   {
      return;
   }
   Path path(*this);
#ifdef _WIN32
   WIN32_FIND_DATAW fd;
   HANDLE hFile;
   String fffs; // find first file string 
   if (mFullName.length() == 0)
   {
      fffs = "./*.*";
   }
   else
   {
      size_t p = mFullName.find("*.*");
      if (p == String::npos)
      {
         if (mFullName[mFullName.length()-1] == '/')
         {
            fffs = mFullName + "*.*";
         }
         else
         {
            fffs = mFullName + "/*.*";
         }
      }
      else
      {
         fffs = mFullName;
      }
   }
   std::wstring wfffs;
   DecodeUTF8(fffs.c_str(), wfffs);
   hFile = FindFirstFileW(wfffs.c_str(), &fd);
   if (hFile != INVALID_HANDLE_VALUE)
   {
      String fname;
      do
      {
         EncodeUTF8(fd.cFileName, fname);
         if (fname == "." ||  fname == "..")
         {
            continue;
         }
         if ((flags & FE_HIDDEN) == 0 && fname.startswith("."))
         {
            continue;
         }
         path.push(fname);
         if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
         {
            if ((flags & FE_DIRECTORY) != 0 && !callback(path))
            {
               break;
            }
            if (recurse)
            {
               path.forEach(callback, true, flags);
            }
         }
         else
         {
            if ((flags & FE_FILE) != 0 && !callback(path))
            {
               break;
            }
         }
         path.pop();
      } while (FindNextFileW(hFile, &fd));
      FindClose(hFile);
   }
#else
   DIR *d;
   String ldir;
   if (mFullName.length() == 0)
   {
      Path cwd = CurrentDir();
      UTF8ToLocale(cwd.fullname('/').c_str(), ldir);
   }
   else
   {
      UTF8ToLocale(mFullName.c_str(), ldir);
   }
   d = opendir(ldir.c_str());
   if (d)
   {
      struct dirent *de;
      while ((de = readdir(d)) != 0)
      {
         String fname;
         LocaleToUTF8(de->d_name, fname);
         if (fname == "." || fname == "..")
         {
            continue;
         }
         if ((flags & FE_HIDDEN) == 0 && fname.startswith("."))
         {
            continue;
         }
         path.push(fname);
         if (path.isDir())
         {
            if ((flags & FE_DIRECTORY) != 0 && !callback(path))
            {
               break;
            }
            if (recurse)
            {
               path.forEach(callback, true, flags);
            }
         }
         else
         {
            if ((flags & FE_FILE) != 0 && !callback(path))
            {
               break;
            }
         }
         path.pop();
      }
      closedir(d);
   }
#endif
}

size_t Path::listDir(PathList &l, bool recurse, unsigned short flags) const
{
   ForEachFunc func;
   details::DirLister dl(l);
   Bind(&dl, &details::DirLister::dirItem, func);
   l.clear();
   forEach(func, recurse, flags);
   return l.size();
}

FILE* Path::open(const char *mode) const
{
   _updateInternals();
#ifdef _WIN32
   std::wstring wmode;
   // supposes mode and ASCII string, thus UTF-8 (should be)
   DecodeUTF8(mode, wmode);
   return _wfopen(mFullNameW.c_str(), wmode.c_str());
#else
   return fopen(mFullNameL.c_str(), mode);
#endif
}

bool Path::open(std::ifstream &inf, std::ios::openmode mode) const
{
   _updateInternals();
#ifdef _WIN32
   inf.open(mFullNameW.c_str(), std::ios::in | mode);
#else
   inf.open(mFullNameL.c_str(), std::ios::in | mode);
#endif
   return inf.is_open();
}

bool Path::open(std::ofstream &outf, std::ios::openmode mode) const
{
   _updateInternals();
#ifdef _WIN32
   outf.open(mFullNameW.c_str(), std::ios::out | mode);
#else
   outf.open(mFullNameL.c_str(), std::ios::out | mode);
#endif
   return outf.is_open();
}

Path Path::CurrentDir()
{
   Path rv(".");
#ifdef _WIN32
   DWORD cwdLen = GetCurrentDirectoryW(0, NULL);
   wchar_t *cwd = (wchar_t*) malloc(cwdLen * sizeof(wchar_t));
   if (GetCurrentDirectoryW(cwdLen, cwd))
   {
      rv = cwd;
   }
   free(cwd);
#else
   char *cwd = getcwd(NULL, 0);
   if (cwd != NULL)
   {
      std::string ucwd;
      if (LocaleToUTF8(cwd, ucwd))
      {
         rv = ucwd;
      }
      free(cwd);
   }
#endif
   return rv;
}


} // gcore

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

Path::Path(const String &s)
{
   operator=(s);
}

Path::Path(const Path &rhs)
   : mPaths(rhs.mPaths)
   , mFullName(rhs.mFullName)
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
//#ifndef _WIN32
         if (i == 0)
         {
            ++i;
            continue;
         }
//#endif
         mPaths.erase(mPaths.begin()+i);
      }
      else
      {
         ++i;
      }
   }
   mFullName = fullname('/');
   return *this;
}

Path& Path::operator+=(const Path &rhs)
{
   if (isDir() && !rhs.isAbsolute())
   {
      mPaths.insert(mPaths.end(), rhs.mPaths.begin(), rhs.mPaths.end());
      mFullName = fullname('/');
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

String Path::pop()
{
   String rv;
   if (mPaths.size() > 0)
   {
      rv = mPaths.back();
      mPaths.pop_back();
      mFullName = fullname('/');
   }
   return rv;
}

Path& Path::push(const String &s)
{
   mPaths.push_back(s);
   mFullName = fullname('/');
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
      mFullName = fullname('/');
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
   mFullName = fullname('/');
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
   struct stat st;
   if (stat(mFullName.c_str(), &st) == 0)
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
#ifdef _WIN32
   DWORD fa;
   fa = GetFileAttributes(mFullName.c_str());
   if (fa != 0xFFFFFFFF)
   {
      return ((fa & FILE_ATTRIBUTE_DIRECTORY) != 0);
   }
#else
   struct stat st;
   if (stat(mFullName.c_str(), &st) == 0)
   {
      return ((st.st_mode & S_IFDIR) != 0);
   }
#endif
   return false;
}

bool Path::isFile() const
{
#ifdef _WIN32
   DWORD fa;
   fa = GetFileAttributes(mFullName.c_str());
   if (fa != 0xFFFFFFFF)
   {
      return ((fa & FILE_ATTRIBUTE_DIRECTORY) == 0);
   }
#else
   struct stat st;
   if (stat(mFullName.c_str(), &st) == 0)
   {
      return ((st.st_mode & S_IFREG) != 0);
   }
#endif
   return false;
}

bool Path::exists() const
{
#ifdef _WIN32
   DWORD fa;
   fa = GetFileAttributes(mFullName.c_str());
   if (fa != 0xFFFFFFFF)
   {
      return true;
   }
#else
   struct stat st;
   if (stat(mFullName.c_str(), &st) == 0)
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

size_t Path::fileSize() const
{
   if (isFile())
   {
      struct stat fileStat;
      if (stat(mFullName.c_str(), &fileStat) == 0)
      {
         return fileStat.st_size;
      }
   }
   return 0;
}

bool Path::createDir(bool recursive) const
{
   if (exists())
   {
      return true;
   }
   if (mPaths.size() == 0)
   {
      return false;
   }
   if (recursive)
   {
      Path pdir(*this);
      pdir.pop();
      if (!pdir.createDir(recursive))
      {
         return false;
      }
   }
#ifdef _WIN32
   return (CreateDirectory(mFullName.c_str(), NULL) == TRUE);
#else
   return (mkdir(mFullName.c_str(), S_IRWXU) == 0);
#endif
}

bool Path::removeFile() const
{
   if (isFile())
   {
      return (remove(mFullName.c_str()) == 0);
   }
   return false;
}

void Path::forEach(ForEachFunc callback, bool recurse, unsigned short flags) const
{
   if (!isDir() || callback == 0)
   {
      return;
   }
   Path path(*this);
#ifdef _WIN32
   WIN32_FIND_DATA fd;
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
   hFile = FindFirstFile(fffs.c_str(), &fd);
   if (hFile != INVALID_HANDLE_VALUE)
   {
      do
      {
         String fname = fd.cFileName;
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
      } while (FindNextFile(hFile, &fd));
      FindClose(hFile);
   }
#else
   DIR *d;
   if (mFullName.length() == 0)
   {
      Path cwd = CurrentDir();
      d = opendir(cwd.fullname('/').c_str());
   }
   else
   {
      d = opendir(mFullName.c_str());
   }
   if (d)
   {
      struct dirent *de;
      while ((de = readdir(d)) != 0)
      {
         String fname = de->d_name;
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

Path Path::CurrentDir()
{
#ifdef _WIN32
   DWORD cwdLen = GetCurrentDirectory(0, NULL);
   char *cwd = (char*)malloc(cwdLen * sizeof(char));
   GetCurrentDirectory(cwdLen, cwd);
#else
   size_t bufLen = 1024;
   char *buf = (char*)malloc(bufLen * sizeof(char));
   char *cwd = getcwd(buf, bufLen);
   while (cwd == NULL)
   {
      if (errno == ERANGE)
      {
         free(buf);
         bufLen *= 2;
         buf = (char*)malloc(bufLen * sizeof(char));
         cwd = getcwd(buf, bufLen);
      }
      else
      {
         buf[0] = '\0';
         cwd = buf;
         break;
      }
   }
#endif
   Path rv = Path(cwd);
   free(cwd);
   return rv;
}

// ---

size_t MemoryMappedFile::PageSize()
{
#ifdef _WIN32
   SYSTEM_INFO si;
   ZeroMemory(&si, sizeof(si));
   GetSystemInfo(&si);
   return size_t(si.dwAllocationGranularity);
#else
   //return size_t(getpagesize());
   //return sysconf(_SC_PAGE_SIZE);
   return sysconf(_SC_PAGESIZE);
#endif
}

MemoryMappedFile::MemoryMappedFile()
   : mFlags(0)
   , mOffset(0)
   , mSize(0)
   , mMapSize(0)
   , mPtr(0)
#ifdef _WIN32
   , mFD(INVALID_HANDLE_VALUE)
   , mMH(INVALID_HANDLE_VALUE)
#else
   , mFD(-1)
#endif
{
}

MemoryMappedFile::MemoryMappedFile(const Path &path, unsigned char flags, size_t offset, size_t size)
   : mFlags(0)
   , mOffset(0)
   , mSize(0)
   , mMapSize(0)
   , mPtr(0)
#ifdef _WIN32
   , mFD(INVALID_HANDLE_VALUE)
   , mMH(INVALID_HANDLE_VALUE)
#else
   , mFD(-1)
#endif
{
   open(path, flags, offset, size);
}

MemoryMappedFile::~MemoryMappedFile()
{
   close();
}

size_t MemoryMappedFile::size() const
{
   return mSize;
}

size_t MemoryMappedFile::fileOffset() const
{
   return mOffset;
}

size_t MemoryMappedFile::mappedSize() const
{
   return mMapSize;
}

void* MemoryMappedFile::data()
{
   return mPtr;
}

const void* MemoryMappedFile::data() const
{
   return mPtr;
}

bool MemoryMappedFile::valid() const
{
   return (mPtr != 0);
}

Status MemoryMappedFile::open(const Path &path, unsigned char flags, size_t offset, size_t size)
{
   close();
   
   if (!path.isFile())
   {
      return Status(false, "gcore::MemoryMappedFile::open: Invalid file path.");
   }
   
   if (size == 0)
   {
      if ((flags & WRITE) == 0 && path.fileSize() == 0)
      {
         return Status(false, "gcore::MemoryMappedFile::open: 'size' argument cannot be zero when file is empty.");
      }
   }
   
#ifdef _WIN32
   
   DWORD access = 0;
   DWORD sharemode = 0;
   
   if ((flags & READ) != 0)
   {
      access = access | GENERIC_READ;
      if ((flags & SHARED) != 0)
      {
         sharemode = sharemode | FILE_SHARE_READ;
      }
   }
   if ((flags & WRITE) != 0)
   {
      access = access | GENERIC_WRITE;
      if ((flags & SHARED) != 0)
      {
         sharemode = sharemode | FILE_SHARE_WRITE;
      }
   }
   
   if (access == 0)
   {
      access = GENERIC_READ;
      if ((flags & SHARED) != 0)
      {
         sharemode = sharemode | FILE_SHARE_READ;
      }
   }
   
   HANDLE fd = CreateFile(path.fullname('/').c_str(), access, sharemode, NULL, OPEN_EXISTING, 0, NULL);
   
   if (fd == INVALID_HANDLE_VALUE)
   {
      return Status(false, std_errno(), "gcore::MemoryMappedFile::open");
   }

   mFD = fd;
   mPath = path;
   mFlags = flags;
   
#else
   
   int oflags = 0;
   
   if ((flags & READ) != 0)
   {
      if ((flags & WRITE) != 0)
      {
         oflags = oflags | O_RDWR;
      }
      else
      {
         oflags = oflags | O_RDONLY;
      }
   }
   else if ((flags & WRITE) != 0)
   {
      oflags = oflags | O_WRONLY;
   }
   
   if (oflags == 0)
   {
      flags = flags | READ;
      oflags = oflags | O_RDONLY;
   }
   
   int fd = ::open(path.fullname('/').c_str(), oflags);
   
   if (fd == -1)
   {
      return Status(false, std_errno(), "gcore::MemoryMappedFile::open");
   }
   
   mFD = fd;
   mPath = path;
   mFlags = flags;
   
#endif
   
   return reopen(offset, size);
}

Status MemoryMappedFile::reopen(size_t offset, size_t size)
{
   if (!mPath.isFile())
   {
      return Status(false, "gcore::MemoryMappedFile::reopen: Invalid file path.");
   }
   
   if (mPtr)
   {
#ifdef _WIN32
      if (!UnmapViewOfFile(mPtr))
#else
      if (munmap(mPtr, mSize) != 0)
#endif
      {
         return Status(false, std_errno(), "gcore::MemoryMappedFile::reopen");
      }
      
      mPtr = 0;
      mOffset = 0;
      mSize = 0;
      mMapSize = 0;
   }
   
   size_t ps = PageSize();
   size_t fs = mPath.fileSize();
   
   if (size == 0)
   {
      size = fs;
   }
   
   offset = ps * ((offset / ps) + (offset % ps ? 1 : 0));
   size = ps * ((size / ps) + (size % ps ? 1 : 0));
   
   // keep internal offset to match given one,
   // round lower
   
   if (offset > fs)
   {
      return Status(false, "gcore::MemoryMappedFile::reopen: Offset beyond file end.");
   }
   
#ifdef _WIN32
   
   if (mMH == INVALID_HANDLE_VALUE || size > mMapSize)
   {
      DWORD prot = ((mFlags & WRITE) != 0) ? PAGE_READWRITE : PAGE_READONLY;
      DWORD hsz = DWORD((size & 0xFFFFFFFF00000000) >> 32);
      DWORD lsz = DWORD(size & 0x00000000FFFFFFFF);
      
      if (mMH != INVALID_HANDLE_VALUE)
      {
         CloseHandle(mMH);
         mMH = INVALID_HANDLE_VALUE;
      }
      
      std::cout << "Create file mapping (hi: " << hsz << ", lo: " << lsz << ")" << std::endl;
      HANDLE mh = CreateFileMapping(mFD, NULL, prot, hsz, lsz, NULL);
      
      if (mh == NULL) //INVALID_HANDLE_VALUE)
      {
         return Status(false, std_errno(), "gcore::MemoryMappedFile::open");
      }
      else
      {
         mMH = mh;
      }
   }
   
   DWORD prot = ((mFlags & WRITE) != 0) ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ;
   DWORD hoff = DWORD((offset & 0xFFFFFFFF00000000) >> 32);
   DWORD loff = DWORD(offset & 0x00000000FFFFFFFF);
   
   std::cout << "Map view" << std::endl;
   mPtr = MapViewOfFile(mMH, prot, hoff, loff, size);
   
   if (mPtr == NULL)
   {
      mPtr = 0;
      return Status(false, std_errno(), "gcore::MemoryMappedFile::reopen");
   }
   
#else
   
   int prot = 0;
   int flags = MAP_FILE;
   
   if ((mFlags & READ) != 0)
   {
      prot = prot | PROT_READ;
   }
   if ((mFlags & WRITE) != 0)
   {
      prot = prot | PROT_WRITE;
   }
   if ((mFlags & SHARED) != 0)
   {
      flags = flags | MAP_SHARED;
   }
   else
   {
      flags = flags | MAP_PRIVATE;
   }
   
   mPtr = mmap(NULL, size, prot, flags, mFD, offset);
   
   if (mPtr == (void*)-1)
   {
      mPtr = 0;
      return Status(false, std_errno(), "gcore::MemoryMappedFile::reopen");
   }
   
#endif
   
   mOffset = offset;
   mMapSize = size;
   
   // on read-only mode, adjust mSize to actual file size
   if ((mFlags & WRITE) == 0 && offset + size > fs)
   {
      mSize = fs - offset;
   }
   else
   {
      mSize = mMapSize;
   }
   
   return Status(true);
}

Status MemoryMappedFile::sync(bool block)
{
   if (mPtr && (mFlags & SHARED) != 0)
   {
#ifdef _WIN32
      if (!FlushViewOfFile(mPtr, mMapSize))
#else
      // Other flags? MS_INVALIDATE
      if (msync(mPtr, mMapSize, (block ? MS_SYNC : MS_ASYNC)) != 0)
#endif
      {
         return Status(false, std_errno(), "gcore::MemoryMappedFile::sync");
      }
#ifdef _WIN32
      if (block)
      {
         // FlushViewOfFile is non-blocking
         if (!FlushFileBuffers(mFD))
         {
            return Status(false, std_errno(), "gcore::MemoryMappedFile::sync");
         }
      }
#endif
   }
   return Status(true);
}

void MemoryMappedFile::close()
{
   sync(true);
   
   if (mPtr)
   {
#ifdef _WIN32
      UnmapViewOfFile(mPtr);
#else
      munmap(mPtr, mMapSize);
#endif
   }
   
#ifdef _WIN32
   if (mMH != INVALID_HANDLE_VALUE)
   {
      CloseHandle(mMH);
   }
   if (mFD != INVALID_HANDLE_VALUE)
   {
      CloseHandle(mFD);
   }
#else
   if (mFD != -1)
   {
      ::close(mFD);
   }
#endif
   
   mPtr = 0;
   mSize = 0;
   mOffset = 0;
   mMapSize = 0;
   mPath = "";
   mFlags = 0;
#ifdef _WIN32
   mFD = INVALID_HANDLE_VALUE;
   mMH = INVALID_HANDLE_VALUE;
#else
   mFD = -1;
#endif
}

} // gcore

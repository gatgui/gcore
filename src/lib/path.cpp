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
   if (mPaths[0] == "")
   {
      return true;
   }
#ifdef _WIN32
   return (mPaths[0].length() >= 2 && mPaths[0][1] == ':');
#else
   return false;
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

size_t MMap::PageSize()
{
#ifdef _WIN32
   SYSTEM_INFO si;
   ZeroMemory(&si, sizeof(si));
   GetSystemInfo(&si);
   return size_t(si.dwAllocationGranularity);
#else
   return sysconf(_SC_PAGESIZE);
#endif
}

MMap::MMap()
   : mFlags(0)
   , mOffset(0)
   , mSize(0)
   , mMapOffset(0)
   , mMapSize(0)
   , mPtr(0)
#ifdef _WIN32
   , mFD(INVALID_HANDLE_VALUE)
   , mMH(NULL)
#else
   , mFD(-1)
#endif
{
}

MMap::MMap(const Path &path, unsigned char flags, size_t offset, size_t size)
   : mFlags(0)
   , mOffset(0)
   , mSize(0)
   , mMapOffset(0)
   , mMapSize(0)
   , mPtr(0)
#ifdef _WIN32
   , mFD(INVALID_HANDLE_VALUE)
   , mMH(NULL)
#else
   , mFD(-1)
#endif
{
   open(path, flags, offset, size);
}

MMap::~MMap()
{
   close();
}

size_t MMap::size() const
{
   return ((mFlags & READ_ONLY) != 0 ? mSize : mMapSize);
}

unsigned char* MMap::data()
{
   return mPtr + (mOffset - mMapOffset);
}

const unsigned char* MMap::data() const
{
   return mPtr + (mOffset - mMapOffset);
}

bool MMap::valid() const
{
   return (mPtr != 0);
}

Status MMap::open(const Path &path, unsigned char flags, size_t offset, size_t size)
{
   close();
   
   if (!path.isFile())
   {
      return Status(false, "gcore::MMap::open: Invalid file path.");
   }
   
   if (size == 0)
   {
      if ((flags & READ_ONLY) != 0 && path.fileSize() == 0)
      {
         return Status(false, "gcore::MMap::open: 'size' argument cannot be zero when file is empty.");
      }
   }
   
#ifdef _WIN32
   
   DWORD access = GENERIC_READ;
   DWORD sharemode = FILE_SHARE_READ;
   DWORD hint = FILE_ATTRIBUTE_NORMAL;
   
   if ((flags & READ_ONLY) == 0)
   {
      access = access | GENERIC_WRITE;
      sharemode = sharemode | FILE_SHARE_WRITE;
   }
   
   if ((flags & RANDOM_ACCESS) != 0)
   {
      if ((flags & SEQUENTIAL_ACCESS) == 0)
      {
         hint = FILE_FLAG_RANDOM_ACCESS;
      }
   }
   else if ((flags & SEQUENTIAL_ACCESS) != 0)
   {
      hint = FILE_FLAG_SEQUENTIAL_SCAN;
   }
   
   HANDLE fd = CreateFile(path.fullname('/').c_str(), access, sharemode, NULL, OPEN_EXISTING, hint, NULL);
   
   if (fd == INVALID_HANDLE_VALUE)
   {
      return Status(false, std_errno(), "gcore::MMap::open");
   }

   mFD = fd;
   mPath = path;
   mFlags = flags;
   
#else
   
   int oflags = ((flags & READ_ONLY) == 0 ? O_RDONLY : O_RDWR);
   
   int fd = ::open(path.fullname('/').c_str(), oflags);
   
   if (fd == -1)
   {
      return Status(false, std_errno(), "gcore::MMap::open");
   }
   
   mFD = fd;
   mPath = path;
   mFlags = flags;
   
#endif
   
   return remap(offset, size);
}

Status MMap::remap(size_t offset, size_t size)
{
   if (!mPath.isFile())
   {
      return Status(false, "gcore::MMap::remap: Invalid file path.");
   }
   
   if (mPtr)
   {
#ifdef _WIN32
      if (!UnmapViewOfFile(mPtr))
#else
      if (munmap(mPtr, mMapSize) != 0)
#endif
      {
         return Status(false, std_errno(), "gcore::MMap::remap");
      }
      
      mPtr = 0;
      mOffset = 0;
      mSize = 0;
      mMapOffset = 0;
      mMapSize = 0;
   }
   
   size_t ps = PageSize();
   size_t fs = mPath.fileSize();
   
   if (size == 0)
   {
      size = fs;
   }
   
   if ((mFlags & READ_ONLY) != 0)
   {
      // if (size == 0 && fs == 0)
      if (size == 0)
      {
         return Status(false, "gcore::MMap::remap: 'size' argument cannot be zero when file is empty.");
      }
      if (offset > fs)
      {
         return Status(false, "gcore::MMap::remap: 'offset' argument pointing after file end.");
      }
   }
   
   // round down offset
   size_t moffset = ps * (offset / ps);
   
   // add remaining bytes to required size
   //size_t msize = (size == 0 ? fs : size) + (offset % ps);
   size_t msize = size + (offset % ps);
   
   // round up size to page size
   msize = ps * ((msize / ps) + (msize % ps ? 1 : 0));
   
#ifdef _WIN32
   
   // On windows, in readonly mode, msize must not go beyond file size
   if ((mFlags & READ_ONLY) != 0)
   {
      if (msize > fs)
      {
         msize = fs;
      }
   }
   
   if (mMH == NULL || msize > mMapSize)
   {
      DWORD prot = ((mFlags & READ_ONLY) != 0 ? PAGE_READONLY : PAGE_READWRITE);
      DWORD hsz = DWORD(msize >> 32);
      DWORD lsz = DWORD(msize & 0xFFFFFFFF);
      
      if (mMH != NULL)
      {
         CloseHandle(mMH);
         mMH = NULL;
      }
      
      HANDLE mh = CreateFileMapping(mFD, NULL, prot, hsz, lsz, NULL);
      
      if (mh == NULL)
      {
         return Status(false, std_errno(), "gcore::MMap::open");
      }
      else
      {
         mMH = mh;
      }
   }
   
   DWORD prot = ((mFlags & READ_ONLY) != 0 ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS);
   DWORD hoff = DWORD(moffset >> 32);
   DWORD loff = DWORD(moffset & 0xFFFFFFFF);
   
   mPtr = (unsigned char*) MapViewOfFile(mMH, prot, hoff, loff, msize);
   
   if (mPtr == NULL)
   {
      mPtr = 0;
      return Status(false, std_errno(), "gcore::MMap::remap");
   }
   
#else
   
   int prot = PROT_READ;
   int flags = MAP_FILE | MAP_SHARED;
   
   if ((mFlags & READ_ONLY) == 0)
   {
      prot = prot | PROT_WRITE;
   }
   
   mPtr = (unsigned char*) mmap(NULL, msize, prot, flags, mFD, moffset);
   
   if ((void*)mPtr == MAP_FAILED)
   {
      mPtr = 0;
      return Status(false, std_errno(), "gcore::MMap::remap");
   }
   
   int hint = MADV_NORMAL;
   
   if ((mFlags & RANDOM_ACCESS) != 0)
   {
      if ((mFlags & SEQUENTIAL_ACCESS) == 0)
      {
         hint = MADV_RANDOM;
      }
   }
   else if ((mFlags & SEQUENTIAL_ACCESS) != 0)
   {
      hint = MADV_SEQUENTIAL;
   }
   
   madvise(mPtr, msize, hint);
   
#endif
   
   mMapOffset = moffset;
   mMapSize = msize;
   
   mOffset = offset;
   mSize = size;
   
   // In read-only mode, adjust mSize to actual file size
   if ((mFlags & READ_ONLY) != 0)
   {
      if ((mOffset + mSize) > fs)
      {
         mSize = fs - mOffset;
      }
   }
   
#ifdef _DEBUG
   std::cout << "Required: offset=" << mOffset << ", size=" << mSize << std::endl;
   std::cout << "Mapped: offset=" << mMapOffset << ", size=" << mMapSize << std::endl;
#endif
   
   return Status(true);
}

Status MMap::sync(bool block)
{
   if (mPtr)
   {
#ifdef _WIN32
      if (!FlushViewOfFile(mPtr, mMapSize))
#else
      // Other flags? MS_INVALIDATE
      if (msync(mPtr, mMapSize, (block ? MS_SYNC : MS_ASYNC)) != 0)
#endif
      {
         return Status(false, std_errno(), "gcore::MMap::sync");
      }
#ifdef _WIN32
      if (block)
      {
         // FlushViewOfFile is non-blocking
         if (!FlushFileBuffers(mFD))
         {
            return Status(false, std_errno(), "gcore::MMap::sync");
         }
      }
#endif
   }
   return Status(true);
}

void MMap::close()
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
   if (mMH != NULL)
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
   mMapOffset = 0;
   mMapSize = 0;
   mPath = "";
   mFlags = 0;
#ifdef _WIN32
   mFD = INVALID_HANDLE_VALUE;
   mMH = NULL;
#else
   mFD = -1;
#endif
}

} // gcore

/*
MIT License

Copyright (c) 2010 Gaetan Guidet

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

#ifndef __gcore_md5_h_
#define __gcore_md5_h_

#include <gcore/string.h>

namespace gcore
{
   class GCORE_API MD5
   {
   public:
   
      MD5();
      MD5(const MD5 &rhs);
      MD5(const String &str, bool final=true);
      ~MD5();

      MD5& operator=(const MD5 &rhs);

      bool operator==(const MD5 &rhs) const;
      bool operator!=(const MD5 &rhs) const;
      bool operator<(const MD5 &rhs) const;
      bool operator<=(const MD5 &rhs) const;
      bool operator>(const MD5 &rhs) const;
      bool operator>=(const MD5 &rhs) const;

      void update(const char *buf, long len=-1);
      void update(const String &str);
      void clear();
   
      String asString();
      String asString() const;

      bool isFinal() const;
      void finalize();

   private:
   
      void transform();
      
   private:
   
      unsigned long mBuf[4];
      unsigned long mBits[2];
      unsigned char mIn[64]; // temp buffer for input data
      bool mFinalized;
   };
   
}

#endif

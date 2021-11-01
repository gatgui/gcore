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
#include <gcore/md5.h>

namespace gcore
{

inline unsigned long Get32(const unsigned char *addr)
{
   return (((((unsigned long)addr[3] << 8) | addr[2]) << 8) | addr[1]) << 8 | addr[0];
}

inline void Put32(unsigned long data, unsigned char *addr)
{
   addr[0] = (unsigned char)data;
   addr[1] = (unsigned char)(data >> 8);
   addr[2] = (unsigned char)(data >> 16);
   addr[3] = (unsigned char)(data >> 24);
}

#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))
#define MD5STEP(f, w, x, y, z, data, s) (w += f(x, y, z) + data, w &= 0xffffffff, w = w<<s | w>>(32-s), w += x)

MD5::MD5()
{
   clear();
}

MD5::MD5(const MD5 &rhs)
   : mFinalized(rhs.mFinalized)
{
   memcpy(mBits, rhs.mBits, 2*sizeof(unsigned long));
   memcpy(mBuf, rhs.mBuf, 4*sizeof(unsigned long));
   memcpy(mIn, rhs.mIn, 64*sizeof(unsigned char));
}

MD5::MD5(const String &str, bool final)
{
   clear();
   update(str);
   if (final)
   {
      finalize();
   }
}

MD5::~MD5() {
}

MD5& MD5::operator=(const MD5 &rhs)
{
   if (this != &rhs)
   {
      mFinalized = rhs.mFinalized;
      memcpy(mBits, rhs.mBits, 2*sizeof(unsigned long));
      memcpy(mBuf, rhs.mBuf, 4*sizeof(unsigned long));
      memcpy(mIn, rhs.mIn, 64*sizeof(unsigned char));
   }
   return *this;
}

bool MD5::operator==(const MD5 &rhs) const
{
   return (mBuf[0] == rhs.mBuf[0] &&
           mBuf[1] == rhs.mBuf[1] &&
           mBuf[2] == rhs.mBuf[2] &&
           mBuf[3] == rhs.mBuf[3]);
}

bool MD5::operator!=(const MD5 &rhs) const
{
   return !operator==(rhs);
}

bool MD5::operator<(const MD5 &rhs) const
{
   for (int i=0; i<4; ++i)
   {
      if (mBuf[i] < rhs.mBuf[i])
      {
         return true;
      }
      else if (mBuf[i] > rhs.mBuf[i])
      {
         return false;
      }
   }
   // all equal
   return false;
}

bool MD5::operator<=(const MD5 &rhs) const
{
   for (int i=0; i<4; ++i)
   {
      if (mBuf[i] < rhs.mBuf[i])
      {
         return true;
      }
      else if (mBuf[i] > rhs.mBuf[i])
      {
         return false;
      }
   }
   // all equal
   return true;
}

bool MD5::operator>(const MD5 &rhs) const
{
   for (int i=0; i<4; ++i)
   {
      if (mBuf[i] > rhs.mBuf[i])
      {
         return true;
      }
      else if (mBuf[i] < rhs.mBuf[i])
      {
         return false;
      }
   }
   // all equal
   return false;
}

bool MD5::operator>=(const MD5 &rhs) const
{
   for (int i=0; i<4; ++i)
   {
      if (mBuf[i] > rhs.mBuf[i])
      {
         return true;
      }
      else if (mBuf[i] < rhs.mBuf[i])
      {
         return false;
      }
   }
   // all equal
   return true;
}

bool MD5::isFinal() const
{
   return mFinalized;
}

void MD5::clear()
{
   mBuf[0] = 0x67452301;
   mBuf[1] = 0xefcdab89;
   mBuf[2] = 0x98badcfe;
   mBuf[3] = 0x10325476;
   mBits[0] = 0;
   mBits[1] = 0;
   memset(mIn, 0, 64*sizeof(unsigned char));
   mFinalized = false;
}

void MD5::update(const String &str)
{
   update(str.c_str(), long(str.length()));
}

void MD5::update(const char *buf, long len)
{
   
   if (mFinalized)
   {
      return;
   }
   
   if (len < 0)
   {
      len = (long) strlen(buf);
   }
   
   // mIn is used a temporary buffer for the transform method
   
   unsigned long t;

   t = mBits[0];
   if ((mBits[0] = (t + ((unsigned long)len << 3)) & 0xffffffff) < t)
   {
      mBits[1]++;
   }
   mBits[1] += len >> 29;

   t = (t >> 3) & 0x3f;

   if (t > 0)
   {
      unsigned char *p = mIn + t;

      t = 64 - t;
      if ((unsigned long)len < t)
      {
         memcpy(p, buf, len);
         return;
      }
      memcpy(p, buf, t);
      transform();
      buf += t;
      len -= t;
   }

   while (len >= 64)
   {
      memcpy(mIn, buf, 64);
      transform();
      buf += 64;
      len -= 64;
   }

   memcpy(mIn, buf, len);
}

void MD5::transform()
{
   register unsigned long a, b, c, d;
   unsigned long in[16];
   
   for (int i=0; i<16; ++i)
   {
      in[i] = Get32(mIn + 4 * i);
   }

   a = mBuf[0];
   b = mBuf[1];
   c = mBuf[2];
   d = mBuf[3];

   MD5STEP(F1, a, b, c, d, in[ 0]+0xd76aa478,  7);
   MD5STEP(F1, d, a, b, c, in[ 1]+0xe8c7b756, 12);
   MD5STEP(F1, c, d, a, b, in[ 2]+0x242070db, 17);
   MD5STEP(F1, b, c, d, a, in[ 3]+0xc1bdceee, 22);
   MD5STEP(F1, a, b, c, d, in[ 4]+0xf57c0faf,  7);
   MD5STEP(F1, d, a, b, c, in[ 5]+0x4787c62a, 12);
   MD5STEP(F1, c, d, a, b, in[ 6]+0xa8304613, 17);
   MD5STEP(F1, b, c, d, a, in[ 7]+0xfd469501, 22);
   MD5STEP(F1, a, b, c, d, in[ 8]+0x698098d8,  7);
   MD5STEP(F1, d, a, b, c, in[ 9]+0x8b44f7af, 12);
   MD5STEP(F1, c, d, a, b, in[10]+0xffff5bb1, 17);
   MD5STEP(F1, b, c, d, a, in[11]+0x895cd7be, 22);
   MD5STEP(F1, a, b, c, d, in[12]+0x6b901122,  7);
   MD5STEP(F1, d, a, b, c, in[13]+0xfd987193, 12);
   MD5STEP(F1, c, d, a, b, in[14]+0xa679438e, 17);
   MD5STEP(F1, b, c, d, a, in[15]+0x49b40821, 22);

   MD5STEP(F2, a, b, c, d, in[ 1]+0xf61e2562,  5);
   MD5STEP(F2, d, a, b, c, in[ 6]+0xc040b340,  9);
   MD5STEP(F2, c, d, a, b, in[11]+0x265e5a51, 14);
   MD5STEP(F2, b, c, d, a, in[ 0]+0xe9b6c7aa, 20);
   MD5STEP(F2, a, b, c, d, in[ 5]+0xd62f105d,  5);
   MD5STEP(F2, d, a, b, c, in[10]+0x02441453,  9);
   MD5STEP(F2, c, d, a, b, in[15]+0xd8a1e681, 14);
   MD5STEP(F2, b, c, d, a, in[ 4]+0xe7d3fbc8, 20);
   MD5STEP(F2, a, b, c, d, in[ 9]+0x21e1cde6,  5);
   MD5STEP(F2, d, a, b, c, in[14]+0xc33707d6,  9);
   MD5STEP(F2, c, d, a, b, in[ 3]+0xf4d50d87, 14);
   MD5STEP(F2, b, c, d, a, in[ 8]+0x455a14ed, 20);
   MD5STEP(F2, a, b, c, d, in[13]+0xa9e3e905,  5);
   MD5STEP(F2, d, a, b, c, in[ 2]+0xfcefa3f8,  9);
   MD5STEP(F2, c, d, a, b, in[ 7]+0x676f02d9, 14);
   MD5STEP(F2, b, c, d, a, in[12]+0x8d2a4c8a, 20);

   MD5STEP(F3, a, b, c, d, in[ 5]+0xfffa3942,  4);
   MD5STEP(F3, d, a, b, c, in[ 8]+0x8771f681, 11);
   MD5STEP(F3, c, d, a, b, in[11]+0x6d9d6122, 16);
   MD5STEP(F3, b, c, d, a, in[14]+0xfde5380c, 23);
   MD5STEP(F3, a, b, c, d, in[ 1]+0xa4beea44,  4);
   MD5STEP(F3, d, a, b, c, in[ 4]+0x4bdecfa9, 11);
   MD5STEP(F3, c, d, a, b, in[ 7]+0xf6bb4b60, 16);
   MD5STEP(F3, b, c, d, a, in[10]+0xbebfbc70, 23);
   MD5STEP(F3, a, b, c, d, in[13]+0x289b7ec6,  4);
   MD5STEP(F3, d, a, b, c, in[ 0]+0xeaa127fa, 11);
   MD5STEP(F3, c, d, a, b, in[ 3]+0xd4ef3085, 16);
   MD5STEP(F3, b, c, d, a, in[ 6]+0x04881d05, 23);
   MD5STEP(F3, a, b, c, d, in[ 9]+0xd9d4d039,  4);
   MD5STEP(F3, d, a, b, c, in[12]+0xe6db99e5, 11);
   MD5STEP(F3, c, d, a, b, in[15]+0x1fa27cf8, 16);
   MD5STEP(F3, b, c, d, a, in[ 2]+0xc4ac5665, 23);

   MD5STEP(F4, a, b, c, d, in[ 0]+0xf4292244,  6);
   MD5STEP(F4, d, a, b, c, in[ 7]+0x432aff97, 10);
   MD5STEP(F4, c, d, a, b, in[14]+0xab9423a7, 15);
   MD5STEP(F4, b, c, d, a, in[ 5]+0xfc93a039, 21);
   MD5STEP(F4, a, b, c, d, in[12]+0x655b59c3,  6);
   MD5STEP(F4, d, a, b, c, in[ 3]+0x8f0ccc92, 10);
   MD5STEP(F4, c, d, a, b, in[10]+0xffeff47d, 15);
   MD5STEP(F4, b, c, d, a, in[ 1]+0x85845dd1, 21);
   MD5STEP(F4, a, b, c, d, in[ 8]+0x6fa87e4f,  6);
   MD5STEP(F4, d, a, b, c, in[15]+0xfe2ce6e0, 10);
   MD5STEP(F4, c, d, a, b, in[ 6]+0xa3014314, 15);
   MD5STEP(F4, b, c, d, a, in[13]+0x4e0811a1, 21);
   MD5STEP(F4, a, b, c, d, in[ 4]+0xf7537e82,  6);
   MD5STEP(F4, d, a, b, c, in[11]+0xbd3af235, 10);
   MD5STEP(F4, c, d, a, b, in[ 2]+0x2ad7d2bb, 15);
   MD5STEP(F4, b, c, d, a, in[ 9]+0xeb86d391, 21);

   mBuf[0] += a;
   mBuf[1] += b;
   mBuf[2] += c;
   mBuf[3] += d;
}

void MD5::finalize()
{
   unsigned count;
   unsigned char *p;

   count = (mBits[0] >> 3) & 0x3F;

   p = mIn + count;
   *p++ = 0x80;

   count = 64 - 1 - count;

   if (count < 8)
   {
      memset(p, 0, count);
      transform();
      memset(mIn, 0, 56);
   }
   else
   {
      memset(p, 0, count-8);
   }

   Put32(mBits[0], mIn + 56);
   Put32(mBits[1], mIn + 60);
   
   transform();
   
   mFinalized = true;
}

String MD5::asString()
{
   unsigned char digest[16];
   
   if (!mFinalized)
   {
      finalize();
   }
   
   Put32(mBuf[0], digest);
   Put32(mBuf[1], digest + 4);
   Put32(mBuf[2], digest + 8);
   Put32(mBuf[3], digest + 12);
   
   char md5s[33];
   sprintf(md5s,
           "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
           digest[ 0], digest[ 1], digest[ 2], digest[ 3],
           digest[ 4], digest[ 5], digest[ 6], digest[ 7],
           digest[ 8], digest[ 9], digest[10], digest[11],
           digest[12], digest[13], digest[14], digest[15]);
   
   return String(md5s);
}

String MD5::asString() const
{
   if (!mFinalized)
   {
      return "";
   }
   else
   {
      unsigned char digest[16];

      Put32(mBuf[0], digest);
      Put32(mBuf[1], digest + 4);
      Put32(mBuf[2], digest + 8);
      Put32(mBuf[3], digest + 12);
         
      char md5s[33];
      sprintf(md5s,
              "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
              digest[ 0], digest[ 1], digest[ 2], digest[ 3],
              digest[ 4], digest[ 5], digest[ 6], digest[ 7],
              digest[ 8], digest[ 9], digest[10], digest[11],
              digest[12], digest[13], digest[14], digest[15]);
      
      return String(md5s);
   }
}

} // gcore


/*
MIT License

Copyright (c) 2013 Gaetan Guidet

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

#include <gcore/base64.h>

#define MASK0  0x00FC0000
#define MASK1  0x0003F000
#define MASK2  0x00000FC0
#define MASK3  0x0000003F

#define SHIFT0 18
#define SHIFT1 12
#define SHIFT2 6
#define SHIFT3 0

namespace gcore
{

namespace base64
{

class Tables
{
public:
   enum Role
   {
      Encoding = 0,
      Decoding
   };

   ~Tables()
   {
      delete[] mDecTable;
   }

   static const char* Get(Role role)
   {
      static Tables sTheTables;

      switch (role)
      {
      case Encoding:
         return sTheTables.mEncTable;
      case Decoding:
         return sTheTables.mDecTable;
      default:
         return NULL;
      }
   }

private:
   Tables()
      : mEncTable("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")
      , mDecTable(0)
   {
      mDecTable = new char[256];
      for (int i=0; i<64; ++i)
      {
         mDecTable[int(mEncTable[i])] = char(i);
      }
   }

   const char *mEncTable;
   char *mDecTable;
};

// ---

size_t EncodeLength(size_t inlen)
{
   return (4 * ((inlen / 3) + (inlen % 3 ? 1 : 0)));
}

static bool _Encode(const void *data, size_t len, char *&out, size_t &outlen)
{
   if (!data)
   {
      if (!out)
      {
         outlen = 0;
      }
      return false;
   }
   
   size_t enclen = EncodeLength(len);
   if (enclen == 0)
   {
      if (!out)
      {
         outlen = 0;
      }
      return false;
   }

   if (!out)
   {
      outlen = enclen;
      out = (char*) malloc(outlen+1);
      out[outlen] = '\0';
   }
   else if (outlen < enclen)
   {
      return false;

   }

   const unsigned char *bytes = (const unsigned char*) data;
   unsigned long tmp;
   size_t outp = 0;
   size_t p = 0;
   const char *encTable = Tables::Get(Tables::Encoding);
   
   while ((len - p) >= 3)
   {
      tmp = (bytes[p] << 16) | (bytes[p+1] << 8) | bytes[p+2];
      out[outp++] = encTable[(tmp & MASK0) >> SHIFT0];
      out[outp++] = encTable[(tmp & MASK1) >> SHIFT1];
      out[outp++] = encTable[(tmp & MASK2) >> SHIFT2];
      out[outp++] = encTable[(tmp & MASK3) >> SHIFT3];
      p += 3;
   }
   
   if ((len - p) == 2)
   {
      tmp = (bytes[p] << 16) | (bytes[p+1] << 8);
      out[outp++] = encTable[(tmp & MASK0) >> SHIFT0];
      out[outp++] = encTable[(tmp & MASK1) >> SHIFT1];
      out[outp++] = encTable[(tmp & MASK2) >> SHIFT2];
      out[outp++] = '=';
   }
   else if ((len - p) == 1)
   {
      tmp = (bytes[p] << 16);
      out[outp++] = encTable[(tmp & MASK0) >> SHIFT0];
      out[outp++] = encTable[(tmp & MASK1) >> SHIFT1];
      out[outp++] = '=';
      out[outp++] = '=';
   }

   return true;
}

bool Encode(const void *data, size_t len, char *out, size_t outlen)
{
   return (out ? _Encode(data, len, out, outlen) : false);
}

char* Encode(const void *data, size_t len, size_t &outlen)
{
   char *out = 0;
   outlen = 0;
   if (!_Encode(data, len, out, outlen))
   {
      return 0;
   }
   else
   {
      return out;
   }
}

bool Encode(const void *data, size_t len, String &out)
{
   size_t outlen = EncodeLength(len);
   if (outlen == 0)
   {
      out = "";
      return true;
   }
   else
   {
      out.resize(outlen);
      char *outbytes = (char*) out.data();
      if (!outbytes)
      {
         out = "";
         return false;
      }
      if (!_Encode(data, len, outbytes, outlen))
      {
         out = "";
         return false;
      }
      else
      {
         return true;
      }
   }
}

bool Encode(const String &in, char *out, size_t outlen)
{
   return (out ? _Encode(in.c_str(), in.length(), out, outlen) : false);
}

char* Encode(const String &in, size_t &outlen)
{
   return Encode(in.c_str(), in.length(), outlen);
}

bool Encode(const String &in, String &out)
{
   return Encode(in.c_str(), in.length(), out);
}

String Encode(const void *data, size_t len)
{
   String rv;
   Encode(data, len, rv);
   return rv;
}

String Encode(const String &in)
{
   return Encode(in.c_str(), in.length());
}

// ---

size_t DecodeLength(const char *in, size_t len)
{
   if (!in || len == 0 || (len % 4) != 0)
   {
      return 0;
   }
   else
   {
      return (3 * (len / 4) - (in[len-1] == '=' ? (in[len-2] == '=' ? 2 : 1) : 0));
   }
}

static bool _Decode(const char *in, size_t len, void* &out, size_t &outlen)
{
   size_t declen = DecodeLength(in, len);
   if (declen == 0)
   {
      if (!out)
      {
         outlen = 0;
      }
      return false;
   }

   bool allocated = false;

   if (!out)
   {
      outlen = declen;
      out = malloc(outlen);
      allocated = true;
   }
   else if (outlen < declen)
   {
      return false;
   }
   
   unsigned long tmp;
   int npad;
   size_t p = 0;
   size_t outp = 0;
   unsigned char *bytes = (unsigned char*) out;
   const char *decTable = Tables::Get(Tables::Decoding);
   
   while ((len - p) > 0)
   {
      tmp = 0;
      npad = 0;
      
      for (int i=0, o=18; i<4; ++i, o-=6)
      {
         char c = in[p+i];
         if (c != '=')
         {
            tmp |= (decTable[int(c)] << o);
         }
         else
         {
            ++npad;
         }
      }
      
      if (npad == 0)
      {
         bytes[outp++] = (unsigned char) ((tmp & 0x00FF0000) >> 16);
         bytes[outp++] = (unsigned char) ((tmp & 0x0000FF00) >> 8);
         bytes[outp++] = (unsigned char) (tmp & 0x000000FF);
      }
      else if (npad == 1)
      {
         bytes[outp++] = (unsigned char) ((tmp & 0x00FF0000) >> 16);
         bytes[outp++] = (unsigned char) ((tmp & 0x0000FF00) >> 8);
      }
      else if (npad == 2)
      {
         bytes[outp++] = (unsigned char) ((tmp & 0x00FF0000) >> 16);
      }
      else
      {
         if (allocated)
         {
            free(out);
            out = 0;
            outlen = 0;
         }
         return false;
      }
      
      p += 4;
   }
   
   return true;
}

bool Decode(const char *in, size_t len, void *out, size_t outlen)
{
   return (out ? _Decode(in, len, out, outlen) : false);
}

void* Decode(const char *in, size_t len, size_t &outlen)
{
   void *out = 0;
   outlen = 0;
   if (!_Decode(in, len, out, outlen))
   {
      return 0;
   }
   else
   {
      return out;
   }
}

bool Decode(const char *in, size_t len, String &out)
{
   size_t outlen = DecodeLength(in, len);
   if (outlen == 0)
   {
      out = "";
      return true;
   }
   else
   {
      out.resize(outlen);
      void *outbytes = (void*) out.data();
      if (!outbytes)
      {
         out = "";
         return false;
      }
      // Note: _Decode won't allocate as outbytes is a not null
      if (!_Decode(in, len, outbytes, outlen))
      {
         out = "";
         return false;
      }
      else
      {
         return true;
      }
   }
}

bool Decode(const String &in, void *out, size_t outlen)
{
   return (out ? _Decode(in.c_str(), in.length(), out, outlen) : false);
}

void* Decode(const String &in, size_t &outlen)
{
   return Decode(in.c_str(), in.length(), outlen);
}

bool Decode(const String &in, String &out)
{
   return Decode(in.c_str(), in.length(), out);
}

String Decode(const char *in, size_t len)
{
   String rv;
   Decode(in, len, rv);
   return rv;
}

String Decode(const String &in)
{
   return Decode(in.c_str(), in.length());
}

} // base64

} // gcore

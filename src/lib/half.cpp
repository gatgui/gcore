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

#include "half.h"

float half::HalfToFloatLUT[65536];

class LUTInitializer
{
public:
   LUTInitializer()
   {
      half::uif x;
      
      int s, e, m;
      
      for (int i=0; i<65536; ++i)
      {
         s = (i >> 15) & 0x0001; //  1 bit   sign
         e = (i >> 10) & 0x001F; //  5 bits  exponent
         m =  i        & 0x03FF; // 10 bits  mantissa
         
         if (e == 0)
         {
            if (m == 0)
            {
               // +/- 0
               x.i = (s << 31);
            }
            else
            {
               // Denormalized half -> normalized float
               // NOTE: Floating point represention have an additional implicit bit
               //       in the mantissa. Its value is 1 for normalized float and
               //       0 for denormalized.
               //       Because we convert from a denormalized half to a normalized float
               //       we need to take this implicit bit into account.
               //       We don't necessarily have to shift by 13 bits. Just shift by enough
               //       bits so we have a normalized half mantissa.
               while ((m & 0x0400) == 0)
               {
                  m = m << 1;
                  e -= 1;
               }
               // mask off the implicit mantissa bit
               m = m & ~0x0400;
               // as we masked off the implicit bit, we need to adjust back the exponent
               e += 1;
               // now the half is normalized, recombine everything into float
               x.i = ((s << 31) | ((e - 15 + 127) << 23) | (m << 13));
            }
         }
         else if (e == 31)
         {
            if (m == 0)
            {
               // +/- inf
               x.i = ((s << 31) | (0x000000FF << 23));
            }
            else
            {
               // NaN
               x.i = ((s << 31) | (0x000000FF << 23) | (m << 13));
            }
         }
         else
         {
            // Normalized half -> normalized float
            x.i = ((s << 31) | ((e - 15 + 127) << 23) | (m << 13));
         }
         
         half::HalfToFloatLUT[i] = x.f;
      }
   }
   
   ~LUTInitializer()
   {
   }
};

static LUTInitializer initLUT = LUTInitializer();

inline static int BitConvert(int val, int srcBits, int dstBits)
{
   int bitshift = srcBits - dstBits;
   
   if (bitshift == 0)
   {
      return val;
   }
   else if (bitshift < 0)
   {
      // target number has more precision
      bitshift = -bitshift;
      return (val << bitshift);
   }
   else
   {
      // target number has less precision, need rounding
      int odd  = (val >> bitshift) & 1;
      // create a mask that propagates the odd bit up to the first bit after truncation
      int mask = (1 << (bitshift - 1)) - 1;
      // combine and truncate
      // combine is done using + so that overflow is propagated to upper bits
      return ((val + mask + odd) >> bitshift);
      
      // Example:
      // 23 bits -> 10 bits (this is completely random!)
      // mask = (1 << 12) - 1
      //      = 0x1000 - 0x0001
      //      = 0x1000 + 0xFFFF
      //      = 0x0FFF
      // if odd, mask + odd = 0x1000
      // else    mask + odd = 0x0FFF
   }
}

unsigned short half::fromFloatBits(int i)
{
   int s = (i >> 31) & 0x1;
   int e = (i >> 23) & 0xFF;
   int m =  i        & 0x007FFFFF;
   
   if (e == 0)
   {
      if (m == 0)
      {
         // zero, keep sign
         return (unsigned short)(s << 15);
      }
   }
   else if (e == 0xFF)
   {
      if (m == 0)
      {
         // infinity, keep sign
         return (s ? NegInf().bits : PosInf().bits);
      }
      else
      {
         // NaN, keep sign, try to keep mantissa, once truncated, mantissa should not be 0
         m = (m >> 13) | (m == 0);
         return (unsigned short)((s << 15) | (31 << 10) | m);
      }
   }
   
   // at this point we have a normalized or denormalized float
   
   e = e - 127 + 15;
   
   if (e <= 0)
   {
      // we still have 10 bit precision in the mantissa, that might help
      if (e < -10)
      {
         // definitely too small, return 0
         return (unsigned short)(s << 15);
      }
      // ok now, we know the float is normalized and the resulting half will
      // be denormalized
      // we need to account for the implicit 24th bit
      m = m | 0x00800000;
      // shift the sign bit to the right place
      s <<= 15;
      // convert the expanded mantissa (24 bits) to 10+e bits (value between 0 and 10)
      m = BitConvert(m, 24, 10+e);
      // recombine using + instead of | so that overflow in mantissa gets propagated to the exponent bits
      return (unsigned short)((s + m) & 0xFFFF);
   }
   else if (e >= 31)
   {
      // overflow, number to big for half
      return (s ? NegInf().bits : PosInf().bits);
   }
   else
   {
      // convert to normalized half (the float is of course itself normalized)
      // shift the sign bit to the right place
      s <<= 15;
      // shift the exponent bits to the right place
      e <<= 10;
      // convert mantissa from 23 bits to 10 bits
      m = BitConvert(m, 23, 10);
      return (unsigned short)((s | (e + m)) & 0xFFFF);
   }
}

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

// References:
//   http://en.wikipedia.org/wiki/Machine_epsilon
//   http://en.wikipedia.org/wiki/Half_precision_floating-point_format
//   http://en.wikipedia.org/wiki/IEEE_754-2008
//   http://en.wikipedia.org/wiki/Floating_point

#ifndef __half_h_
#define __half_h_

#include <iostream>
#include <limits>

class half
{
public:
   
   union uif
   {
      int i;
      float f;
   };
   
   static float HalfToFloatLUT[65536];
   
   static int MaxExponent();
   static int MinExponent();
   static half Max();
   static half Min();
   static half NrmMin();
   static half PosInf();
   static half NegInf();
   static half QNaN(); // quiet NaN
   static half SNaN(); // signaling NaN
   static half Epsilon();
   static half UnitRoundOff(); // machine epsilon = b^-p / 2, with b=2 and p=10

public:
   
   template <class T>
   class CachedFunction
   {
   public:
      
      template <class Function>
      CachedFunction(Function f,
                     half domainMin = -half::Max(),
                     half domainMax =  half::Max(),
                     T defaultValue = 0,
                     T posInfValue  = 0,
                     T negInfValue  = 0,
                     T nanValue     = 0) 
      {
         half x;
         for (int i=0; i<65536; ++i)
         {
            x.setBits((unsigned short)i);
            if (x.isNaN())
            {
               mLUT[i] = nanValue;
            }
            else if (x.isInfinity())
            {
               mLUT[i] = (x.isNegative() ? negInfValue : posInfValue);
            }
            else if (x < domainMin || x > domainMax)
            {
               mLUT[i] = defaultValue;
            }
            else
            {
               mLUT[i] = f (x);
            }
         }
      }
      
      inline T operator()(half x) const
      {
         return mLUT[x.getBits()];
      }
      
   private:
      
      T mLUT[1 << 16];
   };

public:
   
   half();
   half(float f);
   half(const half &rhs);
   ~half();
   
   half& operator=(const half &rhs);
   half& operator=(float f);
   
   half operator - () const;
   half& operator+=(const half &rhs);
   half& operator-=(const half &rhs);
   half& operator*=(const half &rhs);
   half& operator/=(const half &rhs);
   
   operator float () const;
   
   bool isNaN() const;
   bool isFinite() const;
   bool isInfinity() const;
   bool isZero() const;
   bool isNormalized() const;
   bool isDenormalized() const;
   bool isNegative() const;
   
   unsigned short getBits() const;
   void setBits(unsigned short b);
   
   half round(unsigned int n) const;
   
   friend std::ostream& operator<<(std::ostream &os, const half &h);
   friend std::istream& operator>>(std::istream &is, half &h);
   
private:
   
   unsigned short fromFloatBits(int i);
   
private:
   
   unsigned short bits;
};

namespace std
{
   template <>
   class numeric_limits<half>
   {
   public:
      
      static const bool is_specialized = true;
      
      static half min() throw() {return half::NrmMin();}
      static half max() throw() {return half::Max();}
      
      static const int digits = 11; // radix bits precision (mantissa bits + implicit)
      static const int digits10 = 2;
      static const bool is_signed = true;
      static const bool is_integer = false;
      static const bool is_exact = false;
      static const int radix = 2;
      static half epsilon() throw() {return half::Epsilon();}
      static half round_error() throw() {return half::UnitRoundOff();}
      
      static const int min_exponent = -13;
      static const int max_exponent = 16;
      static const int min_exponent10 = -4;
      static const int max_exponent10 = 4;
      
      static const bool has_infinity = true;
      static const bool has_quiet_NaN = true;
      static const bool has_signaling_NaN = true;
      static const float_denorm_style has_denorm = denorm_present;
      static const bool has_denorm_loss = false;
      
      static half infinity() throw() {return half::PosInf();}
      static half quiet_NaN() throw() {return half::QNaN();}
      static half signaling_NaN() throw() {return half::SNaN();}
      static half denorm_min() throw() {return half::Min();}
      
      static const bool is_iec559 = false;
      static const bool is_bounded = false;
      static const bool is_modulo = false;
      
      static const bool traps = true;
      static const bool tinyness_before = false;
      static const float_round_style round_style = round_to_nearest;
   };
}

// --- inline members ---

inline half::half()
   : bits(0)
{
}

inline half::half(float f)
   : bits(0)
{
   uif x;
   x.f = f;
   bits = fromFloatBits(x.i);
}

inline half::half(const half &rhs) : bits(rhs.bits)
{
}

inline half::~half() {
}

inline half& half::operator=(const half &rhs)
{
   bits = rhs.bits;
   return *this;
}

inline half& half::operator=(float f)
{
   uif x;
   x.f = f;
   bits = fromFloatBits(x.i);
   return *this;
}

inline half half::operator - () const
{
   half h;
   h.bits = bits ^ 0x8000;
   return h;
}

inline half& half::operator+=(const half &rhs)
{
   *this = half(float(*this) + float(rhs));
   return *this;
}

inline half& half::operator-=(const half &rhs)
{
   *this = half(float(*this) - float(rhs));
   return *this;
}

inline half& half::operator*=(const half &rhs)
{
   *this = half(float(*this) * float(rhs));
   return *this;
}

inline half& half::operator/=(const half &rhs)
{
   *this = half(float(*this) / float(rhs));
   return *this;
}

inline half::operator float () const
{
   return HalfToFloatLUT[bits];
}

inline bool half::isNaN() const
{
   unsigned short e = (bits >> 10) & 0x001F;
   unsigned short m =  bits        & 0x03FF;
   return ((e == 31) && (m != 0));
}

inline bool half::isInfinity() const
{
   short e = (bits >> 10) & 0x001F;
   short m =  bits        & 0x03FF;
   return ((e == 31) && (m == 0));
}

inline bool half::isFinite() const
{
   unsigned short e = (bits >> 10) & 0x001F;
   return (e < 31);
}

inline bool half::isZero() const
{
   return ((bits & 0x7FFF) == 0);
}

inline bool half::isNormalized() const
{
   unsigned short e = (bits >> 10) & 0x001F;
   return ((e > 0) && (e < 31));
}

inline bool half::isDenormalized () const
{
   unsigned short e = (bits >> 10) & 0x001F;
   unsigned short m =  bits        & 0x03FF;
   return ((e == 0) && (m != 0));
}

inline bool half::isNegative() const
{
   return ((bits & 0x8000) != 0);
}

inline unsigned short half::getBits() const
{
   return bits;
}

inline void half::setBits(unsigned short b)
{
   bits = b;
}

inline half half::round(unsigned int n) const
{
   if (n >= 10)
   {
      return *this;
   }
   unsigned short s = bits & 0x8000;
   unsigned short e = bits & 0x7FFF;
   e >>= 9 - n;
   e  += e & 1;
   e <<= 9 - n;
   if (e >= 0x7C00)
   {
      e = bits;
      e >>= 10 - n;
      e <<= 10 - n;
   }
   half h;
   h.bits = s | e;
   return h;
}

inline std::ostream& operator<<(std::ostream &os, const half &h)
{
   os << float(h);
   return os;
}

inline std::istream& operator>>(std::istream &is, half &h)
{
   float f;
   is >> f;
   h = half(f);
   return is;
}

inline int half::MaxExponent()
{
   // 30-15  = 15, but we have to account for the implicit bit in mantissa
   return 16;
}

inline int half::MinExponent()
{
   // 1-15  = -14, but we have to account for the implicit bit in mantissa
   return -13;
}

inline half half::Max()
{
   half h;
   // normalized half max exponent and mantissa
   h.bits = (unsigned short)((30 << 10) | 0x03FF);
   return h;
}

inline half half::Min()
{
   half h;
   // denormalized min (non-zero) mantissa
   h.bits = (unsigned short)(0x0001);
   return h;
}

inline half half::NrmMin()
{
   half h;
   // normalized half min exponent and null mantissa
   h.bits = (unsigned short)(1 << 10);
   return h;
}

inline half half::PosInf()
{
   half h;
   // exponent bits all to one, sign bit to zero
   h.bits = (unsigned short)(31 << 10);
   return h;
}

inline half half::NegInf()
{
   half h;
   // exponent bits all to one, sign bit to one
   h.bits = (unsigned short)((31 << 10) | 0x8000);
   return h;
}

inline half half::QNaN()
{
   half h;
   h.bits = (unsigned short)((31 << 10) | 0x03FF);
   return h;
}

inline half half::SNaN()
{
   half h;
   h.bits = (unsigned short)((31 << 10) | 0x01FF);
   return h;
}

inline half half::Epsilon()
{
   half h;
   // 2^-10
   // e - 15 = -10
   // e = 5
   h.bits = (unsigned short)(0x05 << 10);
   return h;
}

inline half half::UnitRoundOff()
{
   half h;
   // 2^-11
   // e - 15 = -11
   // e = 4
   h.bits = (unsigned short)(0x04 << 10);
   return h;
}

#endif


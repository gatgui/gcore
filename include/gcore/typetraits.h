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

#ifndef __gcore_typetraits_h__
#define __gcore_typetraits_h__

#include <gcore/config.h>

namespace gcore
{
   template <typename T> struct NoRef
   {
      typedef T Type;
   };
   template <typename T> struct NoRef<const T&>
   {
      typedef const T Type;
   };
   template <typename T> struct NoRef<T&>
   {
      typedef T Type;
   };
   
   template <typename T> struct NoPtr
   {
      typedef T Type;
   };
   template <typename T> struct NoPtr<const T*>
   {
      typedef const T Type;
   };
   template <typename T> struct NoPtr<T*>
   {
      typedef T Type;
   };
   
   template <typename T> struct NoConst
   {
      typedef T Type;
   };
   template <typename T> struct NoConst<const T>
   {
      typedef T Type;
   };
   template <typename T> struct NoConst<const T&>
   {
      typedef T& Type;
   };
   template <typename T> struct NoConst<const T*>
   {
      typedef T* Type;
   };
   
   template <typename T> struct NoRefOrConst
   {
      typedef typename NoRef<typename NoConst<T>::Type>::Type Type;
   };
   
   template <typename T> struct TypeTraits
   {
      typedef const T& ConstRef;
      typedef const T* ConstPtr;
      typedef T& Ref;
      typedef T* Ptr;
      typedef T Value;
      typedef const T ConstValue;
      typedef T Self;
      enum {IsConst = 0};
      enum {IsPtr   = 0};
      enum {IsRef   = 0};
   };
   
   template <typename T> struct TypeTraits<const T>
   {
      typedef const T& ConstRef;
      typedef const T* ConstPtr;
      typedef T& Ref;
      typedef T* Ptr;
      typedef T Value;
      typedef const T ConstValue;
      typedef const T Self;
      enum {IsConst = 1};
      enum {IsPtr   = 0};
      enum {IsRef   = 0};
   };
   
   template <typename T> struct TypeTraits<const T&>
   {
      typedef const T& ConstRef;
      typedef const T* ConstPtr;
      typedef T& Ref;
      typedef T* Ptr;
      typedef T Value;
      typedef const T ConstValue;
      typedef const T& Self;
      enum {IsConst = 1};
      enum {IsPtr   = 0};
      enum {IsRef   = 1};
   };
   
   template <typename T> struct TypeTraits<const T*>
   {
      typedef const T& ConstRef;
      typedef const T* ConstPtr;
      typedef T& Ref;
      typedef T* Ptr;
      typedef T Value;
      typedef const T ConstValue;
      typedef const T* Self;
      enum {IsConst = 1};
      enum {IsPtr   = 1};
      enum {IsRef   = 0};
   };
   
   template <typename T> struct TypeTraits<T&>
   {
      typedef const T& ConstRef;
      typedef const T* ConstPtr;
      typedef T& Ref;
      typedef T* Ptr;
      typedef T Value;
      typedef const T ConstValue;
      typedef T& Self;
      enum {IsConst = 0};
      enum {IsPtr   = 0};
      enum {IsRef   = 1};
   };
   
   template <typename T> struct TypeTraits<T*>
   {
      typedef const T& ConstRef;
      typedef const T* ConstPtr;
      typedef T& Ref;
      typedef T* Ptr;
      typedef T Value;
      typedef const T ConstValue;
      typedef T* Self;
      enum {IsConst = 0};
      enum {IsPtr   = 1};
      enum {IsRef   = 0};
   };
   
}

#endif



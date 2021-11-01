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

#ifndef __gcore_list_h_
#define __gcore_list_h_

#include <gcore/config.h>
#include <gcore/functor.h>
#include <gcore/tpl.h>

namespace gcore
{
   
   template <class T, bool Contiguous=true, class Allocator=std::allocator<T> >
   class List : public TCond<Contiguous, std::vector<T, Allocator>, std::deque<T, Allocator> >::Type
   {
   public:
      
      typedef Functor1wR<bool, const T&> FilterFunc;
      typedef Functor1<T&> MapFunc;
      typedef Functor2wR<T, const T&, const T&> ReduceFunc;
      
      typedef typename TCond<Contiguous, std::vector<T, Allocator>, std::deque<T, Allocator> >::Type BaseType;
      typedef List<T, Contiguous, Allocator> ThisType;
      
   public:
      
      List(const Allocator &a=Allocator())
         : BaseType(a)
      {
      }
      
      List(size_t n, const T &value=T(), const Allocator &a=Allocator())
         : BaseType(n, value, a)
      {
      }
      
      template <class InputIterator>
      List(InputIterator first, InputIterator last, const Allocator &a=Allocator())
         : BaseType(first, last, a)
      {
      }
      
      List(const BaseType &rhs)  
         : BaseType(rhs)
      {
      }
      
      virtual ~List()
      {
      }
      
      template <class A>
      ThisType& operator=(const typename TCond<Contiguous, std::vector<T, A>, std::deque<T, A> >::Type &rhs)
      {
         BaseType::operator=(rhs);
         return *this;
      }
      
      ThisType& filter(FilterFunc func)
      {
         typename BaseType::iterator it = BaseType::begin();
         while (it != BaseType::end())
         {
            if (!func(*it))
            {
               it = BaseType::erase(it);
            }
            else
            {
               ++it;
            }
         }
         return *this;
      }
      
      ThisType& map(MapFunc func)
      {
         typename BaseType::iterator it = BaseType::begin();
         while (it != BaseType::end())
         {
            func(*it);
            ++it;
         }
         return *this;
      }
      
      T reduce(ReduceFunc func, const T &initVal=T()) const
      {
         T val = initVal;
         typename BaseType::const_iterator it = BaseType::begin();
         while (it != BaseType::end())
         {
            val = func(val, *it);
            ++it;
         }
         return val;
      }
      
      inline void push(const T &val)
      {
         BaseType::push_back(val);
      }
      
      inline void pop()
      {
         BaseType::pop_back();
      }
      
      const T& operator()(long i) const
      {
         if (i < 0)
         {
            i = long(BaseType::size()) + i;
         }
         return BaseType::operator[](size_t(i));
      }
      
      T& operator()(long i)
      {
         if (i < 0)
         {
            i = long(BaseType::size()) + i;
         }
         return BaseType::operator[](size_t(i));
      }
      
      ThisType operator()(long from, long to) const
      {
         ThisType rv;
         if (from < 0)
         {
            from = long(BaseType::size()) + from;
            if (from < 0)
            {
               return rv;
            }
         }
         if (to < 0)
         {
            to = long(BaseType::size()) + to;
            if (to - from < 0)
            {
               return rv;
            }
         }
         rv.insert(rv.begin(), BaseType::begin()+from, BaseType::begin()+to+1);
         return rv;
      }
   };
   
}

template <class T, class A>
std::ostream& operator<<(std::ostream &os, const std::vector<T, A> &v)
{
   os << "[";
   if (v.size() > 0)
   {
      os << v[0];
      for (size_t i=1; i<v.size(); ++i)
      {
         os << ", " << v[i];
      }
   }
   os << "]";
   return os;
}

template <class T, class A>
std::ostream& operator<<(std::ostream &os, const std::deque<T, A> &v)
{
   os << "[";
   if (v.size() > 0)
   {
      os << v[0];
      for (size_t i=1; i<v.size(); ++i)
      {
         os << ", " << v[i];
      }
   }
   os << "]";
   return os;
}

#endif

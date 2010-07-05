/*

Copyright (C) 2010  Gaetan Guidet

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

#ifndef __gcore_list_h_
#define __gcore_list_h_

#include <gcore/config.h>
#include <gcore/functor.h>

namespace gcore {
  
  template <class T, class Allocator=std::allocator<T> >
  class List : public std::vector<T, Allocator> {
    public:
      
      typedef Functor1wR<bool, const T&> FilterFunc;
      typedef Functor1<T&> MapFunc;
      typedef Functor2wR<T, const T&, const T&> ReduceFunc;
      
    public:
      
      List(const Allocator &a=Allocator())
        : std::vector<T, Allocator>(a) {
      }
      
      List(size_t n, const T &value=T(), const Allocator &a=Allocator())
        : std::vector<T, Allocator>(n, value, a) {
      }
      
      template <class InputIterator>
      List(InputIterator first, InputIterator last, const Allocator &a=Allocator())
        : std::vector<T, Allocator>(first, last, a) {
      }
      
      List(const std::vector<T, Allocator> &rhs)  
        : std::vector<T, Allocator>(rhs) {
      }
      
      virtual ~List() {
      }
      
      List<T, Allocator>& operator=(const std::vector<T> &rhs) {
        std::vector<T, Allocator>::operator=(rhs);
        return *this;
      }
      
      List<T, Allocator>& filter(FilterFunc func) {
        typename std::vector<T, Allocator>::iterator it = std::vector<T, Allocator>::begin();
        while (it != std::vector<T, Allocator>::end()) {
          if (!func(*it)) {
            it = std::vector<T, Allocator>::erase(it);
          } else {
            ++it;
          }
        }
        return *this;
      }
      
      List<T, Allocator>& map(MapFunc func) {
        typename std::vector<T, Allocator>::iterator it = std::vector<T, Allocator>::begin();
        while (it != std::vector<T, Allocator>::end()) {
          func(*it);
          ++it;
        }
        return *this;
      }
      
      T reduce(ReduceFunc func, const T &initVal=T()) {
        T val = initVal;
        typename std::vector<T, Allocator>::iterator it = std::vector<T, Allocator>::begin();
        while (it != std::vector<T, Allocator>::end()) {
          val = func(val, *it);
          ++it;
        }
        return val;
      }
      
      List<T, Allocator> operator()(long from=0, long to=-1) {
        List<T, Allocator> rv;
        if (from < 0) {
          from = long(std::vector<T, Allocator>::size()) + from;
          if (from < 0) {
            return rv;
          }
        }
        if (to < 0) {
          to = long(std::vector<T, Allocator>::size()) + to;
          if (to < from) {
            return rv;
          }
        }
        rv.insert(rv.begin(), std::vector<T, Allocator>::begin()+from, std::vector<T, Allocator>::begin()+to+1);
        return rv;
      }
  };
  
}

template <class T>
std::ostream& operator<<(std::ostream &os, const std::vector<T> &v) {
  os << "[";
  if (v.size() > 0) {
    os << v[0];
    for (size_t i=1; i<v.size(); ++i) {
      os << ", " << v[i];
    }
  }
  os << "]";
  return os;
}

#endif

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
#include <gcore/callbacks.h>

namespace gcore {
  
  template <class T, class Allocator=std::allocator<T> >
  class List : public std::vector<T> {
    public:
      
      typedef Callback1wR<bool, const T&> FilterFunc;
      typedef Callback1<T&> MapFunc;
      
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
      
      void filter(FilterFunc func) {
        iterator it = begin();
        while (it != end()) {
          if (!func(*it)) {
            it = erase(it);
          } else {
            ++it;
          }
        }
      }
      
      void map(MapFunc func) {
        iterator it = begin();
        while (it != end()) {
          func(*it);
          ++it;
        }
      }
  };
  
}

#endif

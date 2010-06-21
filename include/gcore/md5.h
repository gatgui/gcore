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

#ifndef __gcore_md5_h_
#define __gcore_md5_h_

#include <gcore/config.h>

namespace gcore {
  
  class GCORE_API MD5 {
    public:
    
      MD5();
      MD5(const MD5 &rhs);
      MD5(const std::string &str);
      ~MD5();
  
      MD5& operator=(const MD5 &rhs);
  
      void update(const char *buf, long len=-1);
      void update(const std::string &str);
      void clear();
    
      std::string asString();
  
    private:
    
      void transform();
      void final();
  
    private:
    
      unsigned long mBuf[4];
      unsigned long mBits[2];
      unsigned char mIn[64]; // temp buffer for input data
      bool mFinalized;
  };
  
}

#endif

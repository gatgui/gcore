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

#ifndef __gcore_utils_h_
#define __gcore_utils_h_

#include <gcore/config.h>

namespace gcore {
  
  namespace String {
    
    typedef std::vector<std::string> List;
    
    GCORE_API std::string Strip(const std::string &str);
    GCORE_API size_t Split(const std::string &str, char c, List &splits);
    GCORE_API std::string ToLower(const std::string &str);
    GCORE_API std::string ToUpper(const std::string &str);
    GCORE_API std::string Replace(char chr, const std::string &in, char by, int maxCount=-1);
    GCORE_API std::string Replace(const std::string &str, const std::string &in, const std::string &by, int maxCount=-1);
    GCORE_API bool StartsWith(const std::string &s, const std::string &st);
    GCORE_API std::string Join(const std::string &joiner, const List &lst);
    
    GCORE_API bool ToChar(const std::string &s, char &out);
    GCORE_API bool ToUChar(const std::string &s, unsigned char &out);
    GCORE_API bool ToShort(const std::string &s, short &out);
    GCORE_API bool ToUShort(const std::string &s, unsigned short &out);
    GCORE_API bool ToInt(const std::string &s, int &out);
    GCORE_API bool ToUInt(const std::string &s, unsigned int &out);
    GCORE_API bool ToLong(const std::string &s, long &out);
    GCORE_API bool ToULong(const std::string &s, unsigned long &out);
    GCORE_API bool ToFloat(const std::string &s, float &out);
    GCORE_API bool ToDouble(const std::string &s, double &out);
    GCORE_API bool ToBool(const std::string &s, bool &out);
    
    GCORE_API std::string ToString(char in);
    GCORE_API std::string ToString(unsigned char in);
    GCORE_API std::string ToString(short in);
    GCORE_API std::string ToString(unsigned short in);
    GCORE_API std::string ToString(int in);
    GCORE_API std::string ToString(unsigned int in);
    GCORE_API std::string ToString(long in);
    GCORE_API std::string ToString(unsigned long in);
    GCORE_API std::string ToString(float in);
    GCORE_API std::string ToString(double in);
    GCORE_API std::string ToString(bool in);
  }
}

#endif


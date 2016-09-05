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

#include <gcore/list.h>
#include <cmath>

struct Point {
  float x, y, z;
  
  inline Point()
    : x(0.0f), y(0.0f), z(0.0f) {
  }
  
  inline Point(float v)
    : x(v), y(v), z(v) {
  }
  
  inline Point(float _x, float _y, float _z)
    : x(_x), y(_y), z(_z) {
  }
};

typedef gcore::List<Point> PointList;

std::ostream& operator<<(std::ostream &os, const Point &p) {
  os << "(" << p.x << ", " << p.y << ", " << p.z << ")";
  return os;
}

bool FilterPoint(const Point &p) {
  return (sqrt(p.x*p.x + p.y*p.y + p.z*p.z) > 0.000001f);
}

void NormalizePoint(Point &p) {
  float ilen = 1.0f / sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
  p.x *= ilen;
  p.y *= ilen;
  p.z *= ilen;
}

Point SumPoint(const Point &p0, const Point &p1) {
  Point rv;
  rv.x = p0.x + p1.x;
  rv.y = p0.y + p1.y;
  rv.z = p0.z + p1.z;
  return rv;
}

int main(int, char**) {
  
  PointList points, points2;
  
  for (size_t i=0; i<20; ++i) {
    points.push_back(Point(float(i % 5)));
  }
  
  std::cout << points << std::endl;
  
  std::cout << "Filter out null points..." << std::endl;
  PointList::FilterFunc filter;
  gcore::Bind(FilterPoint, filter);
  std::cout << points.filter(filter) << std::endl;
  
  std::cout << "Slice [-5, -1]..." << std::endl;
  points2 = points(-7, -3);
  std::cout << points2 << std::endl;
  
  std::cout << "Normalize points..." << std::endl;
  PointList::MapFunc normalize;
  gcore::Bind(NormalizePoint, normalize);
  std::cout << points2.map(normalize) << std::endl;
  
  std::cout << "Sum points..." << std::endl;
  PointList::ReduceFunc sum;
  gcore::Bind(SumPoint, sum);
  std::cout << points2.reduce(sum) << std::endl;
  
  std::cout << "Negative indices..." << std::endl;
  size_t idx = points.size() - 1;
  std::cout << "[-1] = " << points(-1) << std::endl;
  std::cout << "[" << idx << "] = " << points[idx] << std::endl;
  idx = points.size() - 3;
  std::cout << "[-3] = " << points(-3) << std::endl;
  std::cout << "[" << idx << "] = " << points[idx] << std::endl;
  
  return 0;
}


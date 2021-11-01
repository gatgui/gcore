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

#include <gcore/list.h>

struct Point
{
   float x, y, z;
   
   inline Point()
      : x(0.0f), y(0.0f), z(0.0f)
   {
   }
   
   inline Point(float v)
      : x(v), y(v), z(v)
   {
   }
   
   inline Point(float _x, float _y, float _z)
      : x(_x), y(_y), z(_z)
   {
   }
};

typedef gcore::List<Point> PointList;

std::ostream& operator<<(std::ostream &os, const Point &p)
{
   os << "(" << p.x << ", " << p.y << ", " << p.z << ")";
   return os;
}

bool FilterPoint(const Point &p)
{
   return (sqrt(p.x*p.x + p.y*p.y + p.z*p.z) > 0.000001f);
}

void NormalizePoint(Point &p)
{
   float ilen = 1.0f / sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
   p.x *= ilen;
   p.y *= ilen;
   p.z *= ilen;
}

Point SumPoint(const Point &p0, const Point &p1)
{
   Point rv;
   rv.x = p0.x + p1.x;
   rv.y = p0.y + p1.y;
   rv.z = p0.z + p1.z;
   return rv;
}

int main(int, char**)
{
   PointList points, points2;
   
   for (size_t i=0; i<20; ++i)
   {
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


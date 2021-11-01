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

#include <gcore/functor.h>
#include <iostream>

float add_float(float a, float b)
{
   return a+b;
}

class Operator
{
public:
   Operator() {}
   virtual ~Operator() {}
   virtual float execute(float, float) const = 0;
};

class Add : public Operator
{
public:
   Add()
   {
   }
   virtual ~Add()
   {
   }
   virtual float execute(float a, float b) const
   {
      return a+b;
   }
};

int main(int, char**)
{
   Add add;
   
   gcore::Functor2wR<float, float, float> cb0;
   gcore::Functor2wR<float, float, float> cb1;
   
   gcore::Bind(add_float, cb0);
   gcore::Bind(&add, METHOD(Add, execute), cb1);
   
   std::cout << "Functor 0 result: " << cb0(1, 2) << std::endl;
   std::cout << "Functor 1 result: " << cb1(1, 2) << std::endl;
   
   return 0;
}



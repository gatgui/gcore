/*

Copyright (C) 2009, 2010  Gaetan Guidet

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

#include <gcore/all.h>
#include <iostream>

float add_float(float a, float b) {
  return a+b;
}

class Operator {
  public:
    Operator(){}
    virtual ~Operator() {}
    virtual float execute(float, float) const = 0;
};

class Add : public Operator {
  public:
    Add() {
    }
    virtual ~Add() {
    }
    virtual float execute(float a, float b) const {
      return a+b;
    }
};

int main(int, char**) {
  
  Add add;
  
  gcore::Functor2wR<float, float, float> cb0;
  gcore::Functor2wR<float, float, float> cb1;
  
  gcore::Bind(add_float, cb0);
  gcore::Bind(&add, METHOD(Add, execute), cb1);
  
  std::cout << "Functor 0 result: " << cb0(1, 2) << std::endl;
  std::cout << "Functor 1 result: " << cb1(1, 2) << std::endl;
  
  return 0;
}



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

#ifndef __gcore_functor_h_
#define __gcore_functor_h_

#include <gcore/config.h>

// List of the callback type
// Functor0
// Functor1<P1>
// Functor2<P1,P2>
// Functor3<P1,P2,P3>
// Functor4<P1,P2,P3,P4>
// Functor0wR<R>
// Functor1wR<R,P1>
// Functor2wR<R,P1,P2>
// Functor3wR<R,P1,P2,P3>
// Functor4wR<R,P1,P2,P3,P4>
//
// For any type, call either:
//   - Bind(funcPtr, cb)
//   - Bind(object, methodPtr, cb);
//     |-> use the METHOD macro
// Where cb is a callback of the wanted signature
// (doing so, if the function or method have a different signature but compatible
//  types, the callback can still be created)
// A callback object is typicaly: Functor2wR<bool,int,int>()
// A function with signature: int (*)(double,double) can still fit in the callback

#define METHOD(className,methodName) &className::methodName

namespace gcore {

class GCORE_API Functor {
  
  public:
    
    typedef void (Functor::*_Method)();
    typedef void (*Function)();
    
    inline Functor()
      : callee(0) {
      ptr.func = 0;
    }

    inline Functor(const Functor &rhs)
      : callee(rhs.callee) {
      memcpy(ptr.mem, rhs.ptr.mem, sizeof(_Method));
    }
    
    inline Functor(const void *c, const void *f, size_t sz)
      : callee(0) {
      if (c) {
        callee = (void*)c;
        memcpy(ptr.mem, f, sz);
      } else {
        ptr.func = f;
      }
    }

    inline Functor& operator=(const Functor &rhs) {
      if (this != &rhs) {
        callee = rhs.callee;
        memcpy(ptr.mem, rhs.ptr.mem, sizeof(_Method));
      }
      return *this;
    }

    // Use only if you know what you're doing
    inline void setCallee(void *c) {
      callee = c;
    }
    
    inline operator int () const {
      return (callee || ptr.func);
    }
        
  public:
    
    void *callee; // a weak ref !!
    
    union {
      char mem[sizeof(_Method)];  // is that big enought ?
      const void *func;
    } ptr;
};

// Zero parameter callback

class GCORE_API Functor0 : public Functor {
  
  public:
    
    inline Functor0() : Functor(), translator(0) {}
    
    inline Functor0(const Functor0 &rhs)
      : Functor(rhs), translator(rhs.translator) {
    }
    
    inline Functor0& operator=(const Functor0 &rhs) {
      Functor::operator=(rhs);
      translator = rhs.translator;
      return *this;
    }

    inline void operator()() {
      translator(*this);
    }
    
  protected:
    
    typedef void (*Translator)(const Functor &cb);
    
    inline Functor0(Translator t, const void *c, const void *f, size_t sz)
      : Functor(c,f,sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename Func>
class FunctionTranslator0 : public Functor0 {
  
  public:
    
    FunctionTranslator0(Func f)
      : Functor0(call, 0, FPTR_CAST(const void*, f), 0) {
    }

    FunctionTranslator0(const FunctionTranslator0<Func> &rhs)
      : Functor0(rhs) {
    }
    
    FunctionTranslator0<Func>& operator=(const FunctionTranslator0<Func> &rhs) {
      Functor0::operator=(rhs);
      return *this;
    }
    
    static void call(const Functor &cb) {
      FPTR_CAST(Func, cb.ptr.func)();
    }
};

template <class Callee, typename Method>
class MethodTranslator0 : public Functor0 {
  
  public:
    
    MethodTranslator0(Callee *c, Method &m)
      : Functor0(call, c, &m, sizeof(Method)) {
    }
    
    MethodTranslator0(const MethodTranslator0<Callee,Method> &rhs)
      : Functor0(rhs) {
    }
    
    MethodTranslator0<Callee,Method>& operator=(const MethodTranslator0<Callee,Method> &rhs) {
      Functor0::operator=(rhs);
      return *this;
    }

    static void call(const Functor &cb) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      (callee->*method)();
    }
};

template <typename R>
class Functor0wR : public Functor {
  
  public:
    
    Functor0wR() : Functor(), translator(0) {}
    
    Functor0wR(const Functor0wR<R> &rhs)
      : Functor(rhs), translator(rhs.translator) {
    }
    
    Functor0wR<R>& operator=(const Functor0wR<R> &rhs) {
      Functor::operator=(rhs);
      translator = rhs.translator;
      return *this;
    }

    R operator()() {
      return translator(*this);
    }
    
  protected:
    
    typedef R (*Translator)(const Functor &);
    
    Functor0wR(Translator t, const void *c, const void *f, size_t sz)
      :Functor(c,f,sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename R, typename Func>
class FunctionTranslator0wR : public Functor0wR<R> {
  
  public:
    
    FunctionTranslator0wR(Func f)
      : Functor0wR<R>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    FunctionTranslator0wR(const FunctionTranslator0wR<R,Func> &rhs)
      : Functor0wR<R>(rhs) {
    }
    
    FunctionTranslator0wR<R,Func>& operator=(const FunctionTranslator0wR<R,Func> &rhs) {
      Functor0wR<R>::operator=(rhs);
      return *this;
    }

    static R call(const Functor &cb) {
      return FPTR_CAST(Func, cb.ptr.func)();
    }
};

template <typename R, class Callee, typename Method>
class MethodTranslator0wR : public Functor0wR<R> {
  
  public:
    
    MethodTranslator0wR(Callee *c, Method &m)
      : Functor0wR<R>(call, c, &m, sizeof(Method)) {
    }
    
    MethodTranslator0wR(const MethodTranslator0wR<R,Callee,Method> &rhs)
      : Functor0wR<R>(rhs) {
    }
    
    MethodTranslator0wR<R,Callee,Method>& operator=(const MethodTranslator0wR<R,Callee,Method> &rhs) {
      Functor0wR<R>::operator=(rhs);
      return *this;
    }

    static R call(const Functor &cb) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      return (callee->*method)();
    }
};

// One parameter callback

template <typename P1>
class Functor1 : public Functor {
  public:
    
    Functor1() : Functor(), translator(0) {}
    
    Functor1(const Functor1<P1> &rhs)
      : Functor(rhs), translator(rhs.translator) {
    }
    
    Functor1<P1>& operator=(const Functor1<P1> &rhs) {
      Functor::operator=(rhs);
      translator = rhs.translator;
      return *this;
    }

    void operator()(P1 p1) {
      translator(*this, p1);
    }
    
  protected:
    
    typedef void (*Translator)(const Functor &, P1);
    
    Functor1(Translator t, const void *c,  const void *f, size_t sz)
      : Functor(c,f,sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename P1, typename Func>
class FunctionTranslator1 : public Functor1<P1> {
  
  public:
    
    FunctionTranslator1(Func f)
      :Functor1<P1>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    FunctionTranslator1(const FunctionTranslator1<P1,Func> &rhs)
      : Functor1<P1>(rhs) {
    }
    
    FunctionTranslator1<P1,Func>& operator=(const FunctionTranslator1<P1,Func> &rhs) {
      Functor1<P1>::operator=(rhs);
      return *this;
    }

    static void call(const Functor &cb, P1 p1) {
      FPTR_CAST(Func, cb.ptr.func)(p1);
    } 
};

template <class Callee, typename P1, typename Method>
class MethodTranslator1 : public Functor1<P1> {
  
  public:
    
    MethodTranslator1(Callee *c, Method &m)
      : Functor1<P1>(call, c, &m, sizeof(Method)) {
    }
    
    MethodTranslator1(const MethodTranslator1<Callee,P1,Method> &rhs)
      : Functor1<P1>(rhs) {
    }
    
    MethodTranslator1<Callee,P1,Method>& operator=(const MethodTranslator1<Callee,P1,Method> &rhs) {
      Functor1<P1>::operator=(rhs);
      return *this;
    }

    static void call(const Functor &cb, P1 p1) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      (callee->*method)(p1);
    } 
};



template <typename R, typename P1>
class Functor1wR : public Functor {
  
  public:
    
    Functor1wR() : Functor(), translator(0) {}
    
    Functor1wR(const Functor1wR<R,P1> &rhs)
      : Functor(rhs), translator(rhs.translator) {
    }
    
    Functor1wR<R,P1>& operator=(const Functor1wR<R,P1> &rhs) {
      Functor::operator=(rhs);
      translator = rhs.translator;
      return *this;
    }

    R operator()(P1 p1) {
      return translator(*this, p1);
    }
    
  protected:
    
    typedef R (*Translator)(const Functor &, P1);
    
    Functor1wR(Translator t, const void *c, const void *f, size_t sz)
      : Functor(c,f,sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename R, typename P1, typename Func>
class FunctionTranslator1wR : public Functor1wR<R,P1> {
  
  public:
    
    FunctionTranslator1wR(Func f)
      :Functor1wR<R,P1>(call, 0, FPTR_CAST(const void*, f), 0) {
    }

    FunctionTranslator1wR(const FunctionTranslator1wR<R,P1,Func> &rhs)
      : Functor1wR<R,P1>(rhs) {
    }
    
    FunctionTranslator1wR<R,P1,Func>& operator=(const FunctionTranslator1wR<R,P1,Func> &rhs) {
      Functor1wR<R,P1>::operator=(rhs);
      return *this;
    }
    
    static R call(const Functor &cb, P1 p1) {
      return FPTR_CAST(Func, cb.ptr.func)(p1);
    } 
};

template <typename R, class Callee, typename P1, typename Method>
class MethodTranslator1wR : public Functor1wR<R,P1> {
  
  public:
    
    MethodTranslator1wR(Callee *c, Method &m)
      :Functor1wR<R,P1>(call, c, &m, sizeof(Method)) {
    }

    MethodTranslator1wR(const MethodTranslator1wR<R,Callee,P1,Method> &rhs)
      : Functor1wR<R,P1>(rhs) {
    }
    
    MethodTranslator1wR<R,Callee,P1,Method>& operator=(const MethodTranslator1wR<R,Callee,P1,Method> &rhs) {
      Functor1wR<R,P1>::operator=(rhs);
      return *this;
    }
    
    static R call(const Functor &cb, P1 p1) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      return (callee->*method)(p1);
    } 
};

// Two parameters callback

template <typename P1, typename P2>
class Functor2 : public Functor {
  
  public:
    
    Functor2() : Functor(), translator(0) {}
    
    Functor2(const Functor2<P1,P2> &rhs)
      : Functor(rhs), translator(rhs.translator) {
    }
    
    Functor2<P1,P2>& operator=(const Functor2<P1,P2> &rhs) {
      Functor::operator=(rhs);
      translator = rhs.translator;
      return *this;
    }

    void operator()(P1 p1, P2 p2) {
      translator(*this, p1, p2);
    }
    
  protected:
    
    typedef void (*Translator)(const Functor &, P1, P2);
    
    Functor2(Translator t, const void *c, const void *f, size_t sz)
      : Functor(c, f, sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename P1, typename P2, typename Func>
class FunctionTranslator2 : public Functor2<P1,P2> {
  
  public:
    
    FunctionTranslator2(Func f)
      :Functor2<P1,P2>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    FunctionTranslator2(const FunctionTranslator2<P1,P2,Func> &rhs)
      : Functor2<P1,P2>(rhs) {
    }
    
    FunctionTranslator2<P1,P2,Func>& operator=(const FunctionTranslator2<P1,P2,Func> &rhs) {
      Functor2<P1,P2>::operator=(rhs);
      return *this;
    }

    static void call(const Functor &cb, P1 p1, P2 p2) {
      FPTR_CAST(Func, cb.ptr.func)(p1, p2);
    }
};

template <class Callee, typename P1, typename P2, typename Method>
class MethodTranslator2 : public Functor2<P1,P2> {
  
  public:
    
    MethodTranslator2(Callee *c, Method &m)
      : Functor2<P1,P2>(call, c, &m, sizeof(Method)) {
    }

    MethodTranslator2(const MethodTranslator2<Callee,P1,P2,Method> &rhs)
      : Functor2<P1,P2>(rhs) {
    }
    
    MethodTranslator2<Callee,P1,P2,Method>& operator=(const MethodTranslator2<Callee,P1,P2,Method> &rhs) {
      Functor2<P1,P2>::operator=(rhs);
      return *this;
    }
    
    static void call(const Functor &cb, P1 p1, P2 p2) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      (callee->*method)(p1,p2);
    }
    
};

template <typename R, typename P1, typename P2>
class Functor2wR : public Functor {
  
  public:
    
    Functor2wR() : Functor(), translator(0) {}
    
    Functor2wR(const Functor2wR<R,P1,P2> &rhs)
      : Functor(rhs), translator(rhs.translator) {
    }
    
    Functor2wR<R,P1,P2>& operator=(const Functor2wR<R,P1,P2> &rhs) {
      Functor::operator=(rhs);
      translator = rhs.translator;
      return *this;
    }

    R operator()(P1 p1, P2 p2) {
      return translator(*this, p1, p2);
    }
    
  protected:
    
    typedef R (*Translator)(const Functor &, P1, P2);
    
    Functor2wR(Translator t, const void *c, const void *f, size_t sz)
      : Functor(c, f, sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename R, typename P1, typename P2, typename Func>
class FunctionTranslator2wR : public Functor2wR<R,P1,P2> {
  
  public:
    
    FunctionTranslator2wR(Func f)
      : Functor2wR<R,P1,P2>(call, 0, FPTR_CAST(const void*, f), 0) {
    }

    FunctionTranslator2wR(const FunctionTranslator2wR<R,P1,P2,Func> &rhs)
      : Functor2wR<R,P1,P2>(rhs) {
    }
    
    FunctionTranslator2wR<R,P1,P2,Func>& operator=(const FunctionTranslator2wR<R,P1,P2,Func> &rhs) {
      Functor2wR<R,P1,P2>::operator=(rhs);
      return *this;
    }
    
    static R call(const Functor &cb, P1 p1, P2 p2) {
      return FPTR_CAST(Func, cb.ptr.func)(p1, p2);
    }
};

template <typename R, class Callee, typename P1, typename P2, typename Method>
class MethodTranslator2wR : public Functor2wR<R,P1,P2> {
  
  public:
    
    MethodTranslator2wR(Callee *c, Method &m)
      :Functor2wR<R,P1,P2>(call, c, &m, sizeof(Method)) {
    }

    MethodTranslator2wR(const MethodTranslator2wR<R,Callee,P1,P2,Method> &rhs)
      : Functor2wR<R,P1,P2>(rhs) {
    }
    
    MethodTranslator2wR<R,Callee,P1,P2,Method>& operator=(const MethodTranslator2wR<R,Callee,P1,P2,Method> &rhs) {
      Functor2wR<R,P1,P2>::operator=(rhs);
      return *this;
    }
    
    static R call(const Functor &cb, P1 p1, P2 p2) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      return (callee->*method)(p1,p2);
    }
    
};

// Three parameters callback

template <typename P1, typename P2, typename P3>
class Functor3 : public Functor {
  public:
    
    Functor3() : Functor(), translator(0) {}
    
    Functor3(const Functor3<P1,P2,P3> &rhs)
      : Functor(rhs), translator(rhs.translator) {
    }
    
    Functor3<P1,P2,P3>& operator=(const Functor3<P1,P2,P3> &rhs) {
      Functor::operator=(rhs);
      translator = rhs.translator;
      return *this;
    }

    void operator()(P1 p1, P2 p2, P3 p3) {
      translator(*this, p1, p2, p3);
    }
    
  protected:
    
    typedef void (*Translator)(const Functor &, P1, P2, P3);
    
    Functor3(Translator t, const void *c, const void *f, size_t sz)
      :Functor(c, f, sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename P1, typename P2, typename P3, typename Func>
class FunctionTranslator3 : public Functor3<P1,P2,P3> {
  
  public:
    
    FunctionTranslator3(Func f)
      : Functor3<P1,P2,P3>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    FunctionTranslator3(const FunctionTranslator3<P1,P2,P3,Func> &rhs)
      : Functor3<P1,P2,P3>(rhs) {
    }
    
    FunctionTranslator3<P1,P2,P3,Func>& operator=(const FunctionTranslator3<P1,P2,P3,Func> &rhs) {
      Functor3<P1,P2,P3>::operator=(rhs);
      return *this;
    }

    static void call(const Functor &cb, P1 p1, P2 p2, P3 p3) {
      FPTR_CAST(Func, cb.ptr.func)(p1, p2, p3);
    }
};

template <class Callee, typename P1, typename P2, typename P3, typename Method>
class MethodTranslator3 : public Functor3<P1,P2,P3> {
  
  public:
    
    MethodTranslator3(Callee *c, Method &m)
      : Functor3<P1,P2,P3>(call, c, &m, sizeof(Method)) {
    }

    MethodTranslator3(const MethodTranslator3<Callee,P1,P2,P3,Method> &rhs)
      : Functor3<P1,P2,P3>(rhs) {
    }
    
    MethodTranslator3<Callee,P1,P2,P3,Method>& operator=(const MethodTranslator3<Callee,P1,P2,P3,Method> &rhs) {
      Functor3<P1,P2,P3>::operator=(rhs);
      return *this;
    }
    
    static void call(const Functor &cb, P1 p1, P2 p2, P3 p3) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      (callee->*method)(p1,p2,p3);
    }
    
};

template <typename R, typename P1, typename P2, typename P3>
class Functor3wR : public Functor {
  
  public:
    
    Functor3wR() : Functor(), translator(0) {}
    
    Functor3wR(const Functor3wR<R,P1,P2,P3> &rhs)
      : Functor(rhs), translator(rhs.translator) {
    }
    
    Functor3wR<R,P1,P2,P3>& operator=(const Functor3wR<R,P1,P2,P3> &rhs) {
      Functor::operator=(rhs);
      translator = rhs.translator;
      return *this;
    }

    R operator()(P1 p1, P2 p2, P3 p3) {
      return translator(*this, p1, p2, p3);
    }
    
  protected:
    
    typedef R (*Translator)(const Functor &, P1, P2, P3);
    
    Functor3wR(Translator t, const void *c, const void *f, size_t sz)
      : Functor(c, f, sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename R, typename P1, typename P2, typename P3, typename Func>
class FunctionTranslator3wR : public Functor3wR<R,P1,P2,P3> {
  
  public:
    
    FunctionTranslator3wR(Func f)
      : Functor3wR<R,P1,P2,P3>(call, 0, FPTR_CAST(const void*, f), 0) {
    }

    FunctionTranslator3wR(const FunctionTranslator3wR<R,P1,P2,P3,Func> &rhs)
      : Functor3wR<R,P1,P2,P3>(rhs) {
    }
    
    FunctionTranslator3wR<R,P1,P2,P3,Func>& operator=(const FunctionTranslator3wR<R,P1,P2,P3,Func> &rhs) {
      Functor3wR<R,P1,P2,P3>::operator=(rhs);
      return *this;
    }
    
    static R call(const Functor &cb, P1 p1, P2 p2, P3 p3) {
      return FPTR_CAST(Func, cb.ptr.func)(p1, p2, p3);
    }
};

template <typename R, class Callee, typename P1, typename P2, typename P3, typename Method>
class MethodTranslator3wR : public Functor3wR<R,P1,P2,P3> {
  
  public:
    
    MethodTranslator3wR(Callee *c, Method &m)
      : Functor3wR<R,P1,P2,P3>(call, c, &m, sizeof(Method)) {
    }
    
    MethodTranslator3wR(const MethodTranslator3wR<R,Callee,P1,P2,P3,Method> &rhs)
      : Functor3wR<R,P1,P2,P3>(rhs) {
    }
    
    MethodTranslator3wR<R,Callee,P1,P2,P3,Method>& operator=(const MethodTranslator3wR<R,Callee,P1,P2,P3,Method> &rhs) {
      Functor3wR<R,P1,P2,P3>::operator=(rhs);
      return *this;
    }

    static R call(const Functor &cb, P1 p1, P2 p2, P3 p3) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      return (callee->*method)(p1,p2,p3);
    }
    
};

// Four parameters callback

template <typename P1, typename P2, typename P3, typename P4>
class Functor4 : public Functor {
  
  public:
    
    Functor4() : Functor(), translator(0) {}
    
    Functor4(const Functor4<P1,P2,P3,P4> &rhs)
      : Functor(rhs), translator(rhs.translator) {
    }
    
    Functor4<P1,P2,P3,P4>& operator=(const Functor4<P1,P2,P3,P4> &rhs) {
      Functor::operator=(rhs);
      translator = rhs.translator;
      return *this;
    }

    void operator()(P1 p1, P2 p2, P3 p3, P4 p4) {
      translator(*this, p1, p2, p3, p4);
    }
    
  protected:
    
    typedef void (*Translator)(const Functor &, P1, P2, P3,P4);
    
    Functor4(Translator t, const void *c, const void *f, size_t sz)
      :Functor(c, f, sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename P1, typename P2, typename P3, typename P4, typename Func>
class FunctionTranslator4 : public Functor4<P1,P2,P3,P4> {
  
  public:
    
    FunctionTranslator4(Func f)
      : Functor4<P1,P2,P3,P4>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    FunctionTranslator4(const FunctionTranslator4<P1,P2,P3,P4,Func> &rhs)
      : Functor4<P1,P2,P3,P4>(rhs) {
    }
    
    FunctionTranslator4<P1,P2,P3,P4,Func>& operator=(const FunctionTranslator4<P1,P2,P3,P4,Func> &rhs) {
      Functor4<P1,P2,P3,P4>::operator=(rhs);
      return *this;
    }

    static void call(const Functor &cb, P1 p1, P2 p2, P3 p3, P4 p4) {
      FPTR_CAST(Func, cb.ptr.func)(p1, p2, p3, p4);
    }
};

template <class Callee, typename P1, typename P2, typename P3, typename P4, typename Method>
class MethodTranslator4 : public Functor4<P1,P2,P3,P4> {
  
  public:
    
    MethodTranslator4(Callee *c, Method &m)
      : Functor4<P1,P2,P3,P4>(call, c, &m, sizeof(Method)) {
    }
    
    MethodTranslator4(const MethodTranslator4<Callee,P1,P2,P3,P4,Method> &rhs)
      : Functor4<P1,P2,P3,P4>(rhs) {
    }
    
    MethodTranslator4<Callee,P1,P2,P3,P4,Method>& operator=(const MethodTranslator4<Callee,P1,P2,P3,P4,Method> &rhs) {
      Functor4<P1,P2,P3,P4>::operator=(rhs);
      return *this;
    }

    static void call(const Functor &cb, P1 p1, P2 p2, P3 p3, P4 p4) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      (callee->*method)(p1,p2,p3,p4);
    }
    
};

template <typename R, typename P1, typename P2, typename P3, typename P4>
class Functor4wR : public Functor {
  
  public:
    
    Functor4wR() : Functor(), translator(0) {}
    
    Functor4wR(const Functor4wR<R,P1,P2,P3,P4> &rhs)
      : Functor(rhs), translator(rhs.translator) {
    }
    
    Functor4wR<R,P1,P2,P3,P4>& operator=(const Functor4wR<R,P1,P2,P3,P4> &rhs) {
      Functor::operator=(rhs);
      translator = rhs.translator;
      return *this;
    }

    R operator()(P1 p1, P2 p2, P3 p3, P4 p4) {
      return translator(*this, p1, p2, p3, p4);
    }
    
  protected:
    
    typedef R (*Translator)(const Functor &, P1, P2, P3,P4);
    
    Functor4wR(Translator t, const void *c, const void *f, size_t sz)
      :Functor(c, f, sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename Func>
class FunctionTranslator4wR : public Functor4wR<R,P1,P2,P3,P4> {
  
  public:
    
    FunctionTranslator4wR(Func f)
      : Functor4wR<R,P1,P2,P3,P4>(call, 0, FPTR_CAST(const void*, f), 0) {
    }

    FunctionTranslator4wR(const FunctionTranslator4wR<R,P1,P2,P3,P4,Func> &rhs)
      : Functor4wR<R,P1,P2,P3,P4>(rhs) {
    }
    
    FunctionTranslator4wR<R,P1,P2,P3,P4,Func>& operator=(const FunctionTranslator4wR<R,P1,P2,P3,P4,Func> &rhs) {
      Functor4wR<R,P1,P2,P3,P4>::operator=(rhs);
      return *this;
    }
    
    static R call(const Functor &cb, P1 p1, P2 p2, P3 p3, P4 p4) {
      return FPTR_CAST(Func, cb.ptr.func)(p1, p2, p3, p4);
    }
};

template <typename R, class Callee, typename P1, typename P2, typename P3, typename P4, typename Method>
class MethodTranslator4wR : public Functor4wR<R,P1,P2,P3,P4> {
  
  public:
    
    MethodTranslator4wR(Callee *c, Method &m)
      : Functor4wR<R,P1,P2,P3,P4>(call, c, &m, sizeof(Method)) {
    }

    MethodTranslator4wR(const MethodTranslator4wR<R,Callee,P1,P2,P3,P4,Method> &rhs)
      : Functor4wR<R,P1,P2,P3,P4>(rhs) {
    }
    
    MethodTranslator4wR<R,Callee,P1,P2,P3,P4,Method>& operator=(const MethodTranslator4wR<R,Callee,P1,P2,P3,P4,Method> &rhs) {
      Functor4wR<R,P1,P2,P3,P4>::operator=(rhs);
      return *this;
    }
    
    static R call(const Functor &cb, P1 p1, P2 p2, P3 p3, P4 p4) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      return (callee->*method)(p1,p2,p3,p4);
    }
    
};

// For zero parameters callback

GCORE_API void Bind(void (*f)(void), Functor0 &cb);

template <class Callee, class Calltype>
void Bind(Callee *c, void (Calltype::*m)(void), Functor0 &cb)
{
  cb = MethodTranslator0<Callee, void (Calltype::*)(void)>(c, m);
}

template <class Callee, class Calltype>
void Bind(const Callee *c, void (Calltype::*m)(void) const, Functor0 &cb)
{
  cb = MethodTranslator0<const Callee, void (Calltype::*)(void) const>(c, m);
}

template <typename CR, typename R>
void Bind(R (*f)(void), Functor0wR<CR> &cb)
{
  cb = FunctionTranslator0wR<CR, R (*)(void)>(f);
}

template <typename CR, class Callee, typename R, class Calltype>
void Bind(Callee *c, R (Calltype::*m)(void), Functor0wR<CR> &cb)
{
  cb = MethodTranslator0wR<CR, Callee, R (Calltype::*)(void)>(c, m);
}

template <typename CR, class Callee, typename R, class Calltype>
void Bind(const Callee *c, R (Calltype::*m)(void) const, Functor0wR<CR> &cb)
{
  cb = MethodTranslator0wR<CR, const Callee, R (Calltype::*)(void) const>(c, m);
}

// For 1 parameter callback

template <typename CP1, typename P1>
void Bind(void (*f)(P1), Functor1<CP1> &cb)
{
  cb = FunctionTranslator1<CP1, void (*)(P1)>(f);
}

template <class Callee, typename CP1, class Calltype, typename P1>
void Bind(Callee *c, void (Calltype::*m)(P1), Functor1<CP1> &cb)
{
  cb = MethodTranslator1<Callee, CP1, void (Calltype::*)(P1)>(c,m);
}

template <class Callee, typename CP1, class Calltype, typename P1>
void Bind(const Callee *c, void (Calltype::*m)(P1) const, Functor1<CP1> &cb)
{
  cb = MethodTranslator1<const Callee, CP1, void (Calltype::*)(P1) const>(c,m);
}

template <typename CR, typename CP1, typename R, typename P1>
void Bind(R (*f)(P1), Functor1wR<CR,CP1> &cb)
{
  cb = FunctionTranslator1wR<CR, CP1, R (*)(P1)>(f);
}

template <typename CR, class Callee, typename CP1, typename R, class Calltype, typename P1>
void Bind(Callee *c, R (Calltype::*m)(P1), Functor1wR<CR,CP1> &cb)
{
  cb = MethodTranslator1wR<CR, Callee, CP1, R (Calltype::*)(P1)>(c,m);
}

template <typename CR, class Callee, typename CP1, typename R, class Calltype, typename P1>
void Bind(const Callee *c, R (Calltype::*m)(P1) const, Functor1wR<CR,CP1> &cb)
{
  cb = MethodTranslator1wR<CR, const Callee, CP1, R (Calltype::*)(P1) const>(c,m);
}

// For 2 parameters callback

template <typename CP1, typename CP2, typename P1, typename P2>
void Bind(void (*f)(P1,P2), Functor2<CP1,CP2> &cb)
{
  cb = FunctionTranslator2<CP1, CP2, void (*)(P1,P2)>(f);
}

template <class Callee, typename CP1, typename CP2, class Calltype, typename P1, typename P2>
void Bind(Callee *c, void (Calltype::*m)(P1,P2), Functor2<CP1,CP2> &cb)
{
  cb = MethodTranslator2<Callee, CP1, CP2, void (Calltype::*)(P1,P2)>(c,m);
}

template <class Callee, typename CP1, typename CP2, class Calltype, typename P1, typename P2>
void Bind(const Callee *c, void (Calltype::*m)(P1,P2) const, Functor2<CP1,CP2> &cb)
{
  cb = MethodTranslator2<const Callee, CP1, CP2, void (Calltype::*)(P1,P2) const>(c,m);
}

template <typename CR, typename CP1, typename CP2, typename R, typename P1, typename P2>
void Bind(R (*f)(P1,P2), Functor2wR<CR,CP1,CP2> &cb)
{
  cb = FunctionTranslator2wR<CR, CP1, CP2, R (*)(P1,P2)>(f);
}

template <typename CR, class Callee, typename CP1, typename CP2, typename R, class Calltype, typename P1, typename P2>
void Bind(Callee *c, R (Calltype::*m)(P1,P2), Functor2wR<CR,CP1,CP2> &cb)
{
  cb = MethodTranslator2wR<CR, Callee, CP1, CP2, R (Calltype::*)(P1,P2)>(c,m);
}

template <typename CR, class Callee, typename CP1, typename CP2, typename R, class Calltype, typename P1, typename P2>
void Bind(const Callee *c, R (Calltype::*m)(P1,P2) const, Functor2wR<CR,CP1,CP2> &cb)
{
  cb = MethodTranslator2wR<CR, const Callee, CP1, CP2, R (Calltype::*)(P1,P2) const>(c,m);
}

// For 3 parameters callback

template <typename CP1, typename CP2, typename CP3, typename P1, typename P2, typename P3>
void Bind(void (*f)(P1,P2,P3), Functor3<CP1,CP2,CP3> &cb)
{
  cb = FunctionTranslator3<CP1, CP2, CP3, void (*)(P1,P2,P3)>(f);
}

template <class Callee, typename CP1, typename CP2, typename CP3, class Calltype, typename P1, typename P2, typename P3>
void Bind(Callee *c, void (Calltype::*m)(P1,P2,P3), Functor3<CP1,CP2,CP3> &cb)
{
  cb = MethodTranslator3<Callee, CP1, CP2, CP3, void (Calltype::*)(P1,P2,P3)>(c,m);
}

template <class Callee, typename CP1, typename CP2, typename CP3, class Calltype, typename P1, typename P2, typename P3>
void Bind(const Callee *c, void (Calltype::*m)(P1,P2,P3) const, Functor3<CP1,CP2,CP3> &cb)
{
  cb = MethodTranslator3<const Callee, CP1, CP2, CP3, void (Calltype::*)(P1,P2,P3) const>(c,m);
}

template <typename CR, typename CP1, typename CP2, typename CP3, typename R, typename P1, typename P2, typename P3>
void Bind(R (*f)(P1,P2,P3), Functor3wR<CR,CP1,CP2,CP3> &cb)
{
  cb = FunctionTranslator3wR<CR, CP1, CP2, CP3, R (*)(P1,P2,P3)>(f);
}

template <typename CR, class Callee, typename CP1, typename CP2, typename CP3, typename R, class Calltype, typename P1, typename P2, typename P3>
void Bind(Callee *c, R (Calltype::*m)(P1,P2,P3), Functor3wR<CR,CP1,CP2,CP3> &cb)
{
  cb = MethodTranslator3wR<CR, Callee, CP1, CP2, CP3, R (Calltype::*)(P1,P2,P3)>(c,m);
}

template <typename CR, class Callee, typename CP1, typename CP2, typename CP3, typename R, class Calltype, typename P1, typename P2, typename P3>
void Bind(const Callee *c, R (Calltype::*m)(P1,P2,P3) const, Functor3wR<CR,CP1,CP2,CP3> &cb)
{
  cb = MethodTranslator3wR<CR, const Callee, CP1, CP2, CP3, R (Calltype::*)(P1,P2,P3) const>(c,m);
}

// For 4 parameters callback

template <typename CP1, typename CP2, typename CP3, typename CP4, typename P1, typename P2, typename P3, typename P4>
void Bind(void (*f)(P1,P2,P3,P4), Functor4<CP1,CP2,CP3,CP4> &cb)
{
  cb = FunctionTranslator4<CP1, CP2, CP3, CP4, void (*)(P1,P2,P3,P4)>(f);
}

template <class Callee, typename CP1, typename CP2, typename CP3, typename CP4, class Calltype, typename P1, typename P2, typename P3, typename P4>
void Bind(Callee *c, void (Calltype::*m)(P1,P2,P3,P4), Functor4<CP1,CP2,CP3,CP4> &cb)
{
  cb = MethodTranslator4<Callee,CP1,CP2,CP3,CP4, void (Calltype::*)(P1,P2,P3,P4)>(c,m);
}

template <class Callee, typename CP1, typename CP2, typename CP3, typename CP4, class Calltype, typename P1, typename P2, typename P3, typename P4>
void Bind(const Callee *c, void (Calltype::*m)(P1,P2,P3,P4) const, Functor4<CP1,CP2,CP3,CP4> &cb)
{
  cb = MethodTranslator4<const Callee,CP1,CP2,CP3,CP4,void (Calltype::*)(P1,P2,P3,P4) const>(c,m);
}

template <typename CR, typename CP1, typename CP2, typename CP3, typename CP4, typename R, typename P1, typename P2, typename P3, typename P4>
void Bind(R (*f)(P1,P2,P3,P4), Functor4wR<CR,CP1,CP2,CP3,CP4> &cb)
{
  cb = FunctionTranslator4wR<CR, CP1, CP2, CP3, CP4, R (*)(P1,P2,P3,P4)>(f);
}

template <typename CR, class Callee, typename CP1, typename CP2, typename CP3, typename CP4, typename R, class Calltype, typename P1, typename P2, typename P3, typename P4>
void Bind(Callee *c, R (Calltype::*m)(P1,P2,P3,P4), Functor4wR<CR,CP1,CP2,CP3,CP4> &cb)
{
  cb = MethodTranslator4wR<CR, Callee, CP1, CP2, CP3, CP4, R (Calltype::*)(P1,P2,P3,P4)>(c,m);
}

template <typename CR, class Callee, typename CP1, typename CP2, typename CP3, typename CP4, typename R, class Calltype, typename P1, typename P2, typename P3, typename P4>
void Bind(const Callee *c, R (Calltype::*m)(P1,P2,P3,P4) const, Functor4wR<CR,CP1,CP2,CP3,CP4> &cb)
{
  cb = MethodTranslator4wR<CR, const Callee, CP1, CP2, CP3, CP4, R (Calltype::*)(P1,P2,P3,P4) const>(c,m);
}


}

#endif

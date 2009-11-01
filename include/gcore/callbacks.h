/*

Copyright (C) 2009  Gaetan Guidet

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

#ifndef __gcore_callbacks_h_
#define __gcore_callbacks_h_

#include <gcore/config.h>

// List of the callback type
// Callback0
// Callback1<P1>
// Callback2<P1,P2>
// Callback3<P1,P2,P3>
// Callback4<P1,P2,P3,P4>
// Callback0wR<R>
// Callback1wR<R,P1>
// Callback2wR<R,P1,P2>
// Callback3wR<R,P1,P2,P3>
// Callback4wR<R,P1,P2,P3,P4>
//
// For any type, call either:
//   - MakeCallback(funcPtr, cb)
//   - MakeCallback(object, methodPtr, cb);
//     |-> use the METHOD macro
// Where cb is a callback of the wanted signature
// (doing so, if the function or method have a different signature but compatible
//  types, the callback can still be created)
// A callback object is typicaly: Callback2wR<bool,int,int>()
// A function with signature: int (*)(double,double) can still fit in the callback

#define METHOD(className,methodName) &className::methodName

namespace gcore {

class GCORE_API Callback {
  
  public:
    
    typedef void (Callback::*_Method)();
    typedef void (*Function)();
    
    inline Callback()
      :callee(0) {
      ptr.func = 0;
    }
    
    inline Callback(const void *c, const void *f, size_t sz) {
      if (c) {
        callee = (void*)c;
        memcpy(ptr.mem, f, sz);
      } else {
        ptr.func = f;
      }
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
      char mem[sizeof(_Method)];  // is that nig enought ?
      const void *func;
    } ptr;
};

// Zero parameter callback

class GCORE_API Callback0 : public Callback {
  
  public:
    
    inline Callback0() : Callback() {}
    
    inline void operator()() {
      translator(*this);
    }
    
  protected:
    
    typedef void (*Translator)(const Callback &cb);
    
    inline Callback0(Translator t, const void *c, const void *f, size_t sz)
      :Callback(c,f,sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename Func>
class FunctionTranslator0 : public Callback0 {
  
  public:
    
    FunctionTranslator0(Func f)
      :Callback0(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    static void call(const Callback &cb) {
      FPTR_CAST(Func, cb.ptr.func)();
    }
};

template <class Callee, typename Method>
class MethodTranslator0 : public Callback0 {
  
  public:
    
    MethodTranslator0(Callee *c, Method &m)
      :Callback0(call, c, &m, sizeof(Method)) {
    }
    
    static void call(const Callback &cb) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      (callee->*method)();
    }
};

template <typename R>
class Callback0wR : public Callback {
  
  public:
    
    Callback0wR() : Callback() {}
    
    R operator()() {
      return translator(*this);
    }
    
  protected:
    
    typedef R (*Translator)(const Callback &);
    
    Callback0wR(Translator t, const void *c, const void *f, size_t sz)
      :Callback(c,f,sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename R, typename Func>
class FunctionTranslator0wR : public Callback0wR<R> {
  
  public:
    
    FunctionTranslator0wR(Func f)
      :Callback0wR<R>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
      
    static R call(const Callback &cb) {
      return FPTR_CAST(Func, cb.ptr.func)();
    }
};

template <typename R, class Callee, typename Method>
class MethodTranslator0wR : public Callback0wR<R> {
  
  public:
    
    MethodTranslator0wR(Callee *c, Method &m)
      :Callback0wR<R>(call, c, &m, sizeof(Method)) {
    }
    
    static R call(const Callback &cb) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      return (callee->*method)();
    }
};

// One parameter callback

template <typename P1>
class Callback1 : public Callback {
  public:
    
    Callback1() : Callback() {}
    
    void operator()(P1 p1) {
      translator(*this, p1);
    }
    
  protected:
    
    typedef void (*Translator)(const Callback &, P1);
    
    Callback1(Translator t, const void *c,  const void *f, size_t sz)
      :Callback(c,f,sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename P1, typename Func>
class FunctionTranslator1 : public Callback1<P1> {
  
  public:
    
    FunctionTranslator1(Func f)
      :Callback1<P1>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    static void call(const Callback &cb, P1 p1) {
      FPTR_CAST(Func, cb.ptr.func)(p1);
    } 
};

template <class Callee, typename P1, typename Method>
class MethodTranslator1 : public Callback1<P1> {
  
  public:
    
    MethodTranslator1(Callee *c, Method &m)
      :Callback1<P1>(call, c, &m, sizeof(Method)) {
    }
    
    static void call(const Callback &cb, P1 p1) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      (callee->*method)(p1);
    } 
};



template <typename R, typename P1>
class Callback1wR : public Callback {
  
  public:
    
    Callback1wR() : Callback() {}
    
    R operator()(P1 p1) {
      return translator(*this, p1);
    }
    
  protected:
    
    typedef R (*Translator)(const Callback &, P1);
    
    Callback1wR(Translator t, const void *c, const void *f, size_t sz)
      :Callback(c,f,sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename R, typename P1, typename Func>
class FunctionTranslator1wR : public Callback1wR<R,P1> {
  
  public:
    
    FunctionTranslator1wR(Func f)
      :Callback1wR<R,P1>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    static R call(const Callback &cb, P1 p1) {
      return FPTR_CAST(Func, cb.ptr.func)(p1);
    } 
};

template <typename R, class Callee, typename P1, typename Method>
class MethodTranslator1wR : public Callback1wR<R,P1> {
  
  public:
    
    MethodTranslator1wR(Callee *c, Method &m)
      :Callback1wR<R,P1>(call, c, &m, sizeof(Method)) {
    }
    
    static R call(const Callback &cb, P1 p1) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      return (callee->*method)(p1);
    } 
};

// Two parameters callback

template <typename P1, typename P2>
class Callback2 : public Callback {
  
  public:
    
    Callback2() : Callback() {}
    
    void operator()(P1 p1, P2 p2) {
      translator(*this, p1, p2);
    }
    
  protected:
    
    typedef void (*Translator)(const Callback &, P1, P2);
    
    Callback2(Translator t, const void *c, const void *f, size_t sz)
      :Callback(c, f, sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename P1, typename P2, typename Func>
class FunctionTranslator2 : public Callback2<P1,P2> {
  
  public:
    
    FunctionTranslator2(Func f)
      :Callback2<P1,P2>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    static void call(const Callback &cb, P1 p1, P2 p2) {
      FPTR_CAST(Func, cb.ptr.func)(p1, p2);
    }
};

template <class Callee, typename P1, typename P2, typename Method>
class MethodTranslator2 : public Callback2<P1,P2> {
  
  public:
    
    MethodTranslator2(Callee *c, Method &m)
      :Callback2<P1,P2>(call, c, &m, sizeof(Method)) {
    }
    
    static void call(const Callback &cb, P1 p1, P2 p2) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      (callee->*method)(p1,p2);
    }
    
};

template <typename R, typename P1, typename P2>
class Callback2wR : public Callback {
  
  public:
    
    Callback2wR() : Callback() {}
    
    R operator()(P1 p1, P2 p2) {
      return translator(*this, p1, p2);
    }
    
  protected:
    
    typedef R (*Translator)(const Callback &, P1, P2);
    
    Callback2wR(Translator t, const void *c, const void *f, size_t sz)
      :Callback(c, f, sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename R, typename P1, typename P2, typename Func>
class FunctionTranslator2wR : public Callback2wR<R,P1,P2> {
  
  public:
    
    FunctionTranslator2wR(Func f)
      :Callback2wR<R,P1,P2>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    static R call(const Callback &cb, P1 p1, P2 p2) {
      return FPTR_CAST(Func, cb.ptr.func)(p1, p2);
    }
};

template <typename R, class Callee, typename P1, typename P2, typename Method>
class MethodTranslator2wR : public Callback2wR<R,P1,P2> {
  
  public:
    
    MethodTranslator2wR(Callee *c, Method &m)
      :Callback2wR<R,P1,P2>(call, c, &m, sizeof(Method)) {
    }
    
    static R call(const Callback &cb, P1 p1, P2 p2) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      return (callee->*method)(p1,p2);
    }
    
};

// Three parameters callback

template <typename P1, typename P2, typename P3>
class Callback3 : public Callback {
  public:
    
    Callback3() : Callback() {}
    
    void operator()(P1 p1, P2 p2, P3 p3) {
      translator(*this, p1, p2, p3);
    }
    
  protected:
    
    typedef void (*Translator)(const Callback &, P1, P2, P3);
    
    Callback3(Translator t, const void *c, const void *f, size_t sz)
      :Callback(c, f, sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename P1, typename P2, typename P3, typename Func>
class FunctionTranslator3 : public Callback3<P1,P2,P3> {
  
  public:
    
    FunctionTranslator3(Func f)
      :Callback3<P1,P2,P3>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    static void call(const Callback &cb, P1 p1, P2 p2, P3 p3) {
      FPTR_CAST(Func, cb.ptr.func)(p1, p2, p3);
    }
};

template <class Callee, typename P1, typename P2, typename P3, typename Method>
class MethodTranslator3 : public Callback3<P1,P2,P3> {
  
  public:
    
    MethodTranslator3(Callee *c, Method &m)
      :Callback3<P1,P2,P3>(call, c, &m, sizeof(Method)) {
    }
    
    static void call(const Callback &cb, P1 p1, P2 p2, P3 p3) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      (callee->*method)(p1,p2,p3);
    }
    
};

template <typename R, typename P1, typename P2, typename P3>
class Callback3wR : public Callback {
  
  public:
    
    Callback3wR() : Callback() {}
    
    R operator()(P1 p1, P2 p2, P3 p3) {
      return translator(*this, p1, p2, p3);
    }
    
  protected:
    
    typedef R (*Translator)(const Callback &, P1, P2, P3);
    
    Callback3wR(Translator t, const void *c, const void *f, size_t sz)
      :Callback(c, f, sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename R, typename P1, typename P2, typename P3, typename Func>
class FunctionTranslator3wR : public Callback3wR<R,P1,P2,P3> {
  
  public:
    
    FunctionTranslator3wR(Func f)
      :Callback3wR<R,P1,P2,P3>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    static R call(const Callback &cb, P1 p1, P2 p2, P3 p3) {
      return FPTR_CAST(Func, cb.ptr.func)(p1, p2, p3);
    }
};

template <typename R, class Callee, typename P1, typename P2, typename P3, typename Method>
class MethodTranslator3wR : public Callback3wR<R,P1,P2,P3> {
  
  public:
    
    MethodTranslator3wR(Callee *c, Method &m)
      :Callback3wR<R,P1,P2,P3>(call, c, &m, sizeof(Method)) {
    }
    
    static R call(const Callback &cb, P1 p1, P2 p2, P3 p3) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      return (callee->*method)(p1,p2,p3);
    }
    
};

// Four parameters callback

template <typename P1, typename P2, typename P3, typename P4>
class Callback4 : public Callback {
  
  public:
    
    Callback4() : Callback() {}
    
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4) {
      translator(*this, p1, p2, p3, p4);
    }
    
  protected:
    
    typedef void (*Translator)(const Callback &, P1, P2, P3,P4);
    
    Callback4(Translator t, const void *c, const void *f, size_t sz)
      :Callback(c, f, sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename P1, typename P2, typename P3, typename P4, typename Func>
class FunctionTranslator4 : public Callback4<P1,P2,P3,P4> {
  
  public:
    
    FunctionTranslator4(Func f)
      :Callback4<P1,P2,P3,P4>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    static void call(const Callback &cb, P1 p1, P2 p2, P3 p3, P4 p4) {
      FPTR_CAST(Func, cb.ptr.func)(p1, p2, p3, p4);
    }
};

template <class Callee, typename P1, typename P2, typename P3, typename P4, typename Method>
class MethodTranslator4 : public Callback4<P1,P2,P3,P4> {
  
  public:
    
    MethodTranslator4(Callee *c, Method &m)
      :Callback4<P1,P2,P3,P4>(call, c, &m, sizeof(Method)) {
    }
    
    static void call(const Callback &cb, P1 p1, P2 p2, P3 p3, P4 p4) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      (callee->*method)(p1,p2,p3,p4);
    }
    
};

template <typename R, typename P1, typename P2, typename P3, typename P4>
class Callback4wR : public Callback {
  
  public:
    
    Callback4wR() : Callback() {}
    
    R operator()(P1 p1, P2 p2, P3 p3, P4 p4) {
      return translator(*this, p1, p2, p3, p4);
    }
    
  protected:
    
    typedef R (*Translator)(const Callback &, P1, P2, P3,P4);
    
    Callback4wR(Translator t, const void *c, const void *f, size_t sz)
      :Callback(c, f, sz), translator(t) {
    }
    
  private:
    
    Translator translator;
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename Func>
class FunctionTranslator4wR : public Callback4wR<R,P1,P2,P3,P4> {
  
  public:
    
    FunctionTranslator4wR(Func f)
      :Callback4wR<R,P1,P2,P3,P4>(call, 0, FPTR_CAST(const void*, f), 0) {
    }
    
    static R call(const Callback &cb, P1 p1, P2 p2, P3 p3, P4 p4) {
      return FPTR_CAST(Func, cb.ptr.func)(p1, p2, p3, p4);
    }
};

template <typename R, class Callee, typename P1, typename P2, typename P3, typename P4, typename Method>
class MethodTranslator4wR : public Callback4wR<R,P1,P2,P3,P4> {
  
  public:
    
    MethodTranslator4wR(Callee *c, Method &m)
      :Callback4wR<R,P1,P2,P3,P4>(call, c, &m, sizeof(Method)) {
    }
    
    static R call(const Callback &cb, P1 p1, P2 p2, P3 p3, P4 p4) {
      Callee *callee = (Callee*)(cb.callee);
      Method &method(*(Method*)(void*)(cb.ptr.mem));
      return (callee->*method)(p1,p2,p3,p4);
    }
    
};

// For zero parameters callback

GCORE_API void MakeCallback(void (*f)(void), Callback0 &cb);

template <class Callee, class Calltype>
void MakeCallback(Callee *c, void (Calltype::*m)(void), Callback0 &cb)
{
  cb = MethodTranslator0<Callee, void (Calltype::*)(void)>(c, m);
}

template <class Callee, class Calltype>
void MakeCallback(const Callee *c, void (Calltype::*m)(void) const, Callback0 &cb)
{
  cb = MethodTranslator0<const Callee, void (Calltype::*)(void) const>(c, m);
}

template <typename CR, typename R>
void MakeCallback(R (*f)(void), Callback0wR<CR> &cb)
{
  cb = FunctionTranslator0wR<CR, R (*)(void)>(f);
}

template <typename CR, class Callee, typename R, class Calltype>
void MakeCallback(Callee *c, R (Calltype::*m)(void), Callback0wR<CR> &cb)
{
  cb = MethodTranslator0wR<CR, Callee, R (Calltype::*)(void)>(c, m);
}

template <typename CR, class Callee, typename R, class Calltype>
void MakeCallback(const Callee *c, R (Calltype::*m)(void) const, Callback0wR<CR> &cb)
{
  cb = MethodTranslator0wR<CR, const Callee, R (Calltype::*)(void) const>(c, m);
}

// For 1 parameter callback

template <typename CP1, typename P1>
void MakeCallback(void (*f)(P1), Callback1<CP1> &cb)
{
  cb = FunctionTranslator1<CP1, void (*)(P1)>(f);
}

template <class Callee, typename CP1, class Calltype, typename P1>
void MakeCallback(Callee *c, void (Calltype::*m)(P1), Callback1<CP1> &cb)
{
  cb = MethodTranslator1<Callee, CP1, void (Calltype::*)(P1)>(c,m);
}

template <class Callee, typename CP1, class Calltype, typename P1>
void MakeCallback(const Callee *c, void (Calltype::*m)(P1) const, Callback1<CP1> &cb)
{
  cb = MethodTranslator1<const Callee, CP1, void (Calltype::*)(P1) const>(c,m);
}

template <typename CR, typename CP1, typename R, typename P1>
void MakeCallback(R (*f)(P1), Callback1wR<CR,CP1> &cb)
{
  cb = FunctionTranslator1wR<CR, CP1, R (*)(P1)>(f);
}

template <typename CR, class Callee, typename CP1, typename R, class Calltype, typename P1>
void MakeCallback(Callee *c, R (Calltype::*m)(P1), Callback1wR<CR,CP1> &cb)
{
  cb = MethodTranslator1wR<CR, Callee, CP1, R (Calltype::*)(P1)>(c,m);
}

template <typename CR, class Callee, typename CP1, typename R, class Calltype, typename P1>
void MakeCallback(const Callee *c, R (Calltype::*m)(P1) const, Callback1wR<CR,CP1> &cb)
{
  cb = MethodTranslator1wR<CR, const Callee, CP1, R (Calltype::*)(P1) const>(c,m);
}

// For 2 parameters callback

template <typename CP1, typename CP2, typename P1, typename P2>
void MakeCallback(void (*f)(P1,P2), Callback2<CP1,CP2> &cb)
{
  cb = FunctionTranslator2<CP1, CP2, void (*)(P1,P2)>(f);
}

template <class Callee, typename CP1, typename CP2, class Calltype, typename P1, typename P2>
void MakeCallback(Callee *c, void (Calltype::*m)(P1,P2), Callback2<CP1,CP2> &cb)
{
  cb = MethodTranslator2<Callee, CP1, CP2, void (Calltype::*)(P1,P2)>(c,m);
}

template <class Callee, typename CP1, typename CP2, class Calltype, typename P1, typename P2>
void MakeCallback(const Callee *c, void (Calltype::*m)(P1,P2) const, Callback2<CP1,CP2> &cb)
{
  cb = MethodTranslator2<const Callee, CP1, CP2, void (Calltype::*)(P1,P2) const>(c,m);
}

template <typename CR, typename CP1, typename CP2, typename R, typename P1, typename P2>
void MakeCallback(R (*f)(P1,P2), Callback2wR<CR,CP1,CP2> &cb)
{
  cb = FunctionTranslator2wR<CR, CP1, CP2, R (*)(P1,P2)>(f);
}

template <typename CR, class Callee, typename CP1, typename CP2, typename R, class Calltype, typename P1, typename P2>
void MakeCallback(Callee *c, R (Calltype::*m)(P1,P2), Callback2wR<CR,CP1,CP2> &cb)
{
  cb = MethodTranslator2wR<CR, Callee, CP1, CP2, R (Calltype::*)(P1,P2)>(c,m);
}

template <typename CR, class Callee, typename CP1, typename CP2, typename R, class Calltype, typename P1, typename P2>
void MakeCallback(const Callee *c, R (Calltype::*m)(P1,P2) const, Callback2wR<CR,CP1,CP2> &cb)
{
  cb = MethodTranslator2wR<CR, const Callee, CP1, CP2, R (Calltype::*)(P1,P2) const>(c,m);
}

// For 3 parameters callback

template <typename CP1, typename CP2, typename CP3, typename P1, typename P2, typename P3>
void MakeCallback(void (*f)(P1,P2,P3), Callback3<CP1,CP2,CP3> &cb)
{
  cb = FunctionTranslator3<CP1, CP2, CP3, void (*)(P1,P2,P3)>(f);
}

template <class Callee, typename CP1, typename CP2, typename CP3, class Calltype, typename P1, typename P2, typename P3>
void MakeCallback(Callee *c, void (Calltype::*m)(P1,P2,P3), Callback3<CP1,CP2,CP3> &cb)
{
  cb = MethodTranslator3<Callee, CP1, CP2, CP3, void (Calltype::*)(P1,P2,P3)>(c,m);
}

template <class Callee, typename CP1, typename CP2, typename CP3, class Calltype, typename P1, typename P2, typename P3>
void MakeCallback(const Callee *c, void (Calltype::*m)(P1,P2,P3) const, Callback3<CP1,CP2,CP3> &cb)
{
  cb = MethodTranslator3<const Callee, CP1, CP2, CP3, void (Calltype::*)(P1,P2,P3) const>(c,m);
}

template <typename CR, typename CP1, typename CP2, typename CP3, typename R, typename P1, typename P2, typename P3>
void MakeCallback(R (*f)(P1,P2,P3), Callback3wR<CR,CP1,CP2,CP3> &cb)
{
  cb = FunctionTranslator3wR<CR, CP1, CP2, CP3, R (*)(P1,P2,P3)>(f);
}

template <typename CR, class Callee, typename CP1, typename CP2, typename CP3, typename R, class Calltype, typename P1, typename P2, typename P3>
void MakeCallback(Callee *c, R (Calltype::*m)(P1,P2,P3), Callback3wR<CR,CP1,CP2,CP3> &cb)
{
  cb = MethodTranslator3wR<CR, Callee, CP1, CP2, CP3, R (Calltype::*)(P1,P2,P3)>(c,m);
}

template <typename CR, class Callee, typename CP1, typename CP2, typename CP3, typename R, class Calltype, typename P1, typename P2, typename P3>
void MakeCallback(const Callee *c, R (Calltype::*m)(P1,P2,P3) const, Callback3wR<CR,CP1,CP2,CP3> &cb)
{
  cb = MethodTranslator3wR<CR, const Callee, CP1, CP2, CP3, R (Calltype::*)(P1,P2,P3) const>(c,m);
}

// For 4 parameters callback

template <typename CP1, typename CP2, typename CP3, typename CP4, typename P1, typename P2, typename P3, typename P4>
void MakeCallback(void (*f)(P1,P2,P3,P4), Callback4<CP1,CP2,CP3,CP4> &cb)
{
  cb = FunctionTranslator4<CP1, CP2, CP3, CP4, void (*)(P1,P2,P3,P4)>(f);
}

template <class Callee, typename CP1, typename CP2, typename CP3, typename CP4, class Calltype, typename P1, typename P2, typename P3, typename P4>
void MakeCallback(Callee *c, void (Calltype::*m)(P1,P2,P3,P4), Callback4<CP1,CP2,CP3,CP4> &cb)
{
  cb = MethodTranslator4<Callee,CP1,CP2,CP3,CP4, void (Calltype::*)(P1,P2,P3,P4)>(c,m);
}

template <class Callee, typename CP1, typename CP2, typename CP3, typename CP4, class Calltype, typename P1, typename P2, typename P3, typename P4>
void MakeCallback(const Callee *c, void (Calltype::*m)(P1,P2,P3,P4) const, Callback4<CP1,CP2,CP3,CP4> &cb)
{
  cb = MethodTranslator4<const Callee,CP1,CP2,CP3,CP4,void (Calltype::*)(P1,P2,P3,P4) const>(c,m);
}

template <typename CR, typename CP1, typename CP2, typename CP3, typename CP4, typename R, typename P1, typename P2, typename P3, typename P4>
void MakeCallback(R (*f)(P1,P2,P3,P4), Callback4wR<CR,CP1,CP2,CP3,CP4> &cb)
{
  cb = FunctionTranslator4wR<CR, CP1, CP2, CP3, CP4, R (*)(P1,P2,P3,P4)>(f);
}

template <typename CR, class Callee, typename CP1, typename CP2, typename CP3, typename CP4, typename R, class Calltype, typename P1, typename P2, typename P3, typename P4>
void MakeCallback(Callee *c, R (Calltype::*m)(P1,P2,P3,P4), Callback4wR<CR,CP1,CP2,CP3,CP4> &cb)
{
  cb = MethodTranslator4wR<CR, Callee, CP1, CP2, CP3, CP4, R (Calltype::*)(P1,P2,P3,P4)>(c,m);
}

template <typename CR, class Callee, typename CP1, typename CP2, typename CP3, typename CP4, typename R, class Calltype, typename P1, typename P2, typename P3, typename P4>
void MakeCallback(const Callee *c, R (Calltype::*m)(P1,P2,P3,P4) const, Callback4wR<CR,CP1,CP2,CP3,CP4> &cb)
{
  cb = MethodTranslator4wR<CR, const Callee, CP1, CP2, CP3, CP4, R (Calltype::*)(P1,P2,P3,P4) const>(c,m);
}


}

#endif

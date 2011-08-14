/*

Copyright (C) 2009, 2010, 2011  Gaetan Guidet

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

#include <gcore/plist.h>
#include <gcore/rex.h>
#include <cstdarg>

namespace gcore {

plist::Exception::Exception(const gcore::String &prop)
  : std::exception(), mStr("*** PropertyList Error: [" + prop + "] Unknown") {
}

plist::Exception::Exception(const gcore::String &prop, const gcore::String &str)
  : std::exception(), mStr("*** PropertyList Error: [" + prop + "]" + str) {
}

plist::Exception::Exception(const gcore::String &prop, const char *fmt, ...) {
  char buffer[1024];
  va_list vl;
  va_start(vl, fmt);  
  vsprintf(buffer, fmt, vl);
  va_end(vl);
  mStr = gcore::String("*** PropertyList Error: [") + prop + "] " + buffer;
}

plist::Exception::~Exception() throw() {
}

const char* plist::Exception::what() const throw() {
  return mStr.c_str();
}

// ---

plist::Value::Value()
  : mType(-2), mNull(true) {
}

plist::Value::~Value() {
}

// ---

plist::InvalidValue::InvalidValue() {
}

plist::InvalidValue::~InvalidValue() {
}

bool plist::InvalidValue::fromXML(const gcore::XMLElement *)  {
  return false;
}

gcore::XMLElement* plist::InvalidValue::toXML(gcore::XMLElement *) const {
  return NULL;
}

// ---


plist::String::String()
  : mValue("") {
  mNull = false;
  mType = PropertyList::ValueTypeID("string");
}

plist::String::String(const gcore::String &str)
  : mValue(str) {
  mNull = false;
  mType = PropertyList::ValueTypeID("string");
}

plist::String::~String() {
}

bool plist::String::fromXML(const gcore::XMLElement *elt)  {
  if (!elt) return false;
  if (elt->getTag() != "string") {
    return false;
  } else {
    mValue = elt->getText();
    mValue.strip();
    return true;
  }
}

gcore::XMLElement* plist::String::toXML(gcore::XMLElement *elt) const {
  gcore::XMLElement *s = new gcore::XMLElement("string");
  s->setText(mValue);
  if (elt) elt->addChild(s);
  return s;
}

plist::Value* plist::String::New() {
  return new String();
}

const char* plist::String::TypeName() {
  return "string";
} 

// ---

plist::Integer::Integer()
  : mValue(0) {
  mNull = false;
  mType = PropertyList::ValueTypeID("integer");
}

plist::Integer::Integer(long value)
  : mValue(value) {
  mNull = false;
  mType = PropertyList::ValueTypeID("integer");
}

plist::Integer::~Integer() {
}

bool plist::Integer::fromXML(const gcore::XMLElement *elt)  {
  if (!elt) return false;
  if (elt->getTag() != "integer") {
    return false;
  } else {
    gcore::String txt = elt->getText();
    return txt.strip().toLong(mValue);
  }
}

gcore::XMLElement* plist::Integer::toXML(gcore::XMLElement *elt) const {
  gcore::XMLElement *s = new gcore::XMLElement("integer");
  s->setText(gcore::String(mValue));
  if (elt) elt->addChild(s);
  return s;
}

plist::Value* plist::Integer::New() {
  return new Integer();
}

const char* plist::Integer::TypeName() {
  return "integer";
}

// ---

plist::Real::Real()
  : mValue(0.0) {
  mNull = false;
  mType = PropertyList::ValueTypeID("real");
}

plist::Real::Real(double value)
  : mValue(value) {
  mNull = false;
  mType = PropertyList::ValueTypeID("real");
}

plist::Real::~Real() {
}

bool plist::Real::fromXML(const gcore::XMLElement *elt)  {
  if (!elt) return false;
  if (elt->getTag() != "real") {
    return false;
  } else {
    gcore::String txt = elt->getText();
    return txt.strip().toDouble(mValue);
  }
}

gcore::XMLElement* plist::Real::toXML(gcore::XMLElement *elt) const {
  gcore::XMLElement *s = new gcore::XMLElement("real");
  s->setText(gcore::String(mValue));
  if (elt) elt->addChild(s);
  return s;
}

plist::Value* plist::Real::New() {
  return new Real();
}

const char* plist::Real::TypeName() {
  return "real";
} 

// ---

plist::Boolean::Boolean()
  : mValue(false) {
  mNull = false;
  mType = PropertyList::ValueTypeID("boolean");
}

plist::Boolean::Boolean(bool value)
  : mValue(value) {
  mNull = false;
  mType = PropertyList::ValueTypeID("boolean");
}

plist::Boolean::~Boolean() {
}

bool plist::Boolean::fromXML(const gcore::XMLElement *elt)  {
  if (!elt) return false;
  if (elt->getTag() != "boolean") {
    return false;
  } else {
    gcore::String txt = elt->getText();
    return txt.strip().toBool(mValue);
  }
}

gcore::XMLElement* plist::Boolean::toXML(gcore::XMLElement *elt) const {
  gcore::XMLElement *s = new gcore::XMLElement("boolean");
  s->setText(gcore::String(mValue));
  if (elt) elt->addChild(s);
  return s;
}

plist::Value* plist::Boolean::New() {
  return new Boolean();
}

const char* plist::Boolean::TypeName() {
  return "boolean";
} 

// ---

plist::Array::Array() {
  mNull = false;
  mType = PropertyList::ValueTypeID("array");
}

plist::Array::Array(const List<Value*> &val)
  : mValues(val) {
  mNull = false;
  mType = PropertyList::ValueTypeID("array");
}

plist::Array::~Array() {
  clear();
}
    
void plist::Array::clear() {
  for (size_t i=0; i<mValues.size(); ++i) {
    if (mValues[i]) {
      delete mValues[i];
    }
  }
}
    
plist::Value* plist::Array::at(size_t idx) {
  static plist::InvalidValue dummy = plist::InvalidValue();
  if (idx >= mValues.size()) {
    return &dummy;
  }
  if (!mValues[idx]) {
    return &dummy;
  }
  return mValues[idx];
}

const plist::Value* plist::Array::at(size_t idx) const {
  static plist::InvalidValue dummy = plist::InvalidValue();
  if (idx >= mValues.size()) {
    return &dummy;
  }
  if (!mValues[idx]) {
    return &dummy;
  }
  return mValues[idx];
}

void plist::Array::set(size_t idx, Value *v, bool replace) {
  if (idx < mValues.size()) {
    if (mValues[idx]) {
      if (!replace) {
        return;
      }
      delete mValues[idx];
    }
  }
  while (idx >= mValues.size()) {
    mValues.push(0);
  }
  mValues[idx] = v;
}

bool plist::Array::fromXML(const gcore::XMLElement *elt)  {
  clear();
  if (!elt) return false;
  if (elt->getTag() != "array") {
    return false;
  } else {
    for (size_t i=0; i<elt->numChildren(); ++i) {
      const gcore::XMLElement *c = elt->getChild(i);
      Value *v = PropertyList::NewValue(c->getTag());
      if (v == 0) {
        return false;
      }
      if (!v->fromXML(c)) {
        delete v;
        return false;
      }
      mValues.push(v);
    }
    return true;
  }
}

gcore::XMLElement* plist::Array::toXML(gcore::XMLElement *elt) const {
  gcore::XMLElement *s = new gcore::XMLElement("array");
  for (size_t i=0; i<mValues.size(); ++i) {
    if (mValues[i]) {
      mValues[i]->toXML(s);
    }
  }
  if (elt) elt->addChild(s);
  return s;
}

plist::Value* plist::Array::New() {
  return new Array();
}

const char* plist::Array::TypeName() {
  return "array";
}

// ---

plist::Dictionary::Dictionary() {
  mNull = false;
  mType = PropertyList::ValueTypeID("dict");
}

plist::Dictionary::Dictionary(const std::map<gcore::String, Value*> &val)
  : mPairs(val) {
  mNull = false;
  mType = PropertyList::ValueTypeID("dict");
}

plist::Dictionary::~Dictionary() {
  clear();
}
  	
void plist::Dictionary::clear() {
  std::map<gcore::String, Value*>::iterator it = mPairs.begin();
  while (it != mPairs.end()) {
    if (it->second) {
      delete it->second;
    }
    ++it;
  }
  mPairs.clear();
}

bool plist::Dictionary::has(const gcore::String &key) const {
  return (mPairs.find(key) != mPairs.end());
}

size_t plist::Dictionary::keys(gcore::StringList &keys) const
{
   keys.clear();
   std::map<gcore::String, Value*>::const_iterator it = mPairs.begin();
   while (it != mPairs.end())
   {
      keys.push_back(it->first);
      ++it;
   }
   return keys.size();
}

plist::Value* plist::Dictionary::value(const gcore::String &key) {
  static plist::InvalidValue dummy = plist::InvalidValue();
  std::map<gcore::String, Value*>::iterator it = mPairs.find(key);
  if (it == mPairs.end()) {
    return &dummy;
  }
  if (!it->second) {
    return &dummy;
  }
  return it->second;
}

const plist::Value* plist::Dictionary::value(const gcore::String &key) const {
  static plist::InvalidValue dummy = plist::InvalidValue();
  std::map<gcore::String, Value*>::const_iterator it = mPairs.find(key);
  if (it == mPairs.end()) {
    return &dummy;
  }
  if (!it->second) {
    return &dummy;
  }
  return it->second;
}

void plist::Dictionary::set(const gcore::String &key, Value *v, bool replace) {
  if (has(key)) {
    if (!replace) {
      return;
    }
    delete mPairs[key];
  }
  mPairs[key] = v;
}

bool plist::Dictionary::fromXML(const gcore::XMLElement *elt)  {
  clear();
  if (!elt) return false;
  if (elt->getTag() != "dict") {
    return false;
  } else {
    size_t i = 0;
    while (i < elt->numChildren()) {
      const gcore::XMLElement *k = elt->getChild(i);
      if (k->getTag() != "key") {
        return false;
      }
      gcore::String key = k->getText();
      key.strip();
      if (mPairs.find(key) != mPairs.end()) {
        return false;
      }
      const gcore::XMLElement *v = elt->getChild(i+1);
      Value *val = PropertyList::NewValue(v->getTag());
      if (val == 0) {
        return false;
      }
      if (!val->fromXML(v)) {
        delete val;
        return false;
      }
      mPairs[key] = val;
      i += 2;
    }
    return true;
  }
}

gcore::XMLElement* plist::Dictionary::toXML(gcore::XMLElement *elt) const {
  gcore::XMLElement *s = new gcore::XMLElement("dict");
  std::map<gcore::String, Value*>::const_iterator it = mPairs.begin();
  while (it != mPairs.end()) {
    if (it->second) {
      gcore::XMLElement *k = new gcore::XMLElement("key");
      k->setText(it->first);
      s->addChild(k);
      it->second->toXML(s);
    }
    ++it;
  }
  if (elt) elt->addChild(s);
  return s;
}

plist::Value* plist::Dictionary::New() {
  return new Dictionary();
}

const char* plist::Dictionary::TypeName() {
  return "dict";
}

// ---

PropertyList::ValueDescDict PropertyList::msValueDesc;

void PropertyList::RegisterBasicTypes() {
  RegisterType<plist::String>();
  RegisterType<plist::Boolean>();
  RegisterType<plist::Integer>();
  RegisterType<plist::Real>();
  RegisterType<plist::Array>();
  RegisterType<plist::Dictionary>();
}

void PropertyList::ClearTypes() {
  msValueDesc.clear();
}

plist::Value* PropertyList::NewValue(const String &type) {
  ValueDescIterator it = msValueDesc.find(type);
  if (it != msValueDesc.end()) {
    plist::Value *v = it->second.ctor();
    return v;
  } else {
    return 0;
  }
}

long PropertyList::ValueTypeID(const String &type) {
  ValueDescIterator it = msValueDesc.find(type);
  if (it != msValueDesc.end()) {
    return it->second.id;
  } else {
    return -1;
  }
}

const String& PropertyList::ValueTypeName(long id) {
  static const String null = "Null";
  ValueDescIterator it = msValueDesc.begin();
  while (it != msValueDesc.end()) {
    if (it->second.id == id) {
      return it->first;
    }
    ++it;
  }
  return null;
}

PropertyList::PropertyList()
  : mTop(0) {
  RegisterBasicTypes();
}

PropertyList::~PropertyList() {
  if (mTop) {
    delete mTop;
  }
}

void PropertyList::create() {
  if (mTop) {
    delete mTop;
  }
  mTop = new plist::Dictionary();
}

XMLElement* PropertyList::write(XMLElement *elt) const {
  if (mTop) {
    return mTop->toXML(elt);
  } else {
    return NULL;
  }
}

bool PropertyList::read(const XMLElement *elt) {
  bool newed = true;
  if (!mTop) {
    mTop = new plist::Dictionary();
  } else {
    mTop->clear();
    newed = false;
  }
  bool rv = mTop->fromXML(elt);
  if (!rv && newed) {
    delete mTop;
    mTop = 0;
  }
  return rv;
}

void PropertyList::write(const String &filename) const {
  XMLDoc *doc = new XMLDoc();
  XMLElement *root = write(NULL);
  if (root) {
    doc->setRoot(root);
    doc->write(filename);
  }
  delete doc;
}

bool PropertyList::read(const String &filename) {
  bool rv = false;
  XMLDoc *doc = new XMLDoc();
  if (doc->read(filename)) {
    rv = read(doc->getRoot());
  }
  delete doc;
  return rv;
}

// might have a way to merge GetPropertyMember and SetPropertyMember
// common parts in one function

static plist::Value* GetPropertyMember(plist::Dictionary* &cdict,
                                       const String &pre,
                                       const String &current,
                                       bool final=false) throw(plist::Exception)
{
  String cprop = pre;

  const char *c = current.c_str();
  const char *b = strchr(c, '[');
  
  if (cprop.length() > 0) {
    cprop += ".";
  }
  
  if (b) {
    
    plist::Array *ary = 0;
    
    while (b) {
      
      const char *e = strchr(b, ']');
      
      if (e) {
        
        size_t from = c - current.c_str();
        size_t to = b - current.c_str();
        long idx;
        
        String member = current.substr(from, to-from);
        cprop += member; // might be 0 for multiple dim array... that's ok
        
        from = to + 1;
        to = e - current.c_str();
        
        String number = current.substr(from, to-from);
        
        if (number.find_first_not_of("0123456789") != String::npos) {
          throw plist::Exception(cprop, "Invalid subscript (%s)", number.c_str());
        }
        
        if (sscanf(number.c_str(), "%ld", &idx) != 1) {
          throw plist::Exception(cprop, "Invalid subscript (%s)", number.c_str());
        }
        
        cprop += "[" + number + "]";
                
        if (!ary) {
          if (member.length() == 0) {
            throw plist::Exception(cprop, "Missing member name");
          }
          
          if (!cdict->value(member)->checkType(ary)) {
            throw plist::Exception(cprop, "Incompatible types (expected \"array\", got \"%s\")",
                                   PropertyList::ValueTypeName(cdict->value(member)->getType()).c_str());
          }
        }
        
        c = e + 1;
        b = strchr(c, '[');
        
        if (idx >= long(ary->size())) {
          throw plist::Exception(cprop, "Invalid index %ld", idx);
        }
        
        if (!b) {
          
          if (to+1 != current.length()) {
            throw plist::Exception(cprop, "Characters after subscript");
          }
          
          if (final) {
            
            return ary->at(idx);
            
          } else {
            plist::Dictionary *subDict = 0;
            
            if (!ary->at(idx)->checkType(subDict)) {
              throw plist::Exception(cprop, "Incompatible types (expected \"dict\", got \"%s\")",
                                     PropertyList::ValueTypeName(ary->at(idx)->getType()).c_str());
              
            }
          
            cdict = subDict;
          }
          
        } else {
          
          // it necessarily an array, as we have more subscripts
          plist::Array *subAry = 0;
          
          if (!ary->at(idx)->checkType(subAry)) {
            throw plist::Exception(cprop, "Incompatible types (expected \"dict\", got \"%s\")",
                                   PropertyList::ValueTypeName(ary->at(idx)->getType()).c_str());
          }
          
          ary = subAry;
          
        }
        
      } else {
        throw plist::Exception(cprop, "Missing close bracket");
      }
    }
    
  } else {
    
    cprop += current;
  
    if (final) {
      
      return cdict->value(current);
      
    } else {
      
      plist::Dictionary *d = 0;
    
      if (!cdict->value(current)->checkType(d)) {
        throw plist::Exception(cprop, "Incompatible types (expected \"dict\", got \"%s\")",
                               PropertyList::ValueTypeName(cdict->value(current)->getType()).c_str());
      }
      
      cdict = d;
    }
  }
  
  return 0;
}

static void SetPropertyMember(plist::Dictionary* &cdict,
                              const String &pre,
                              const String &current,
                              plist::Value *value=0) throw(plist::Exception)
{
  String cprop = pre;

  const char *c = current.c_str();
  const char *b = strchr(c, '[');
  
  if (cprop.length() > 0) {
    cprop += ".";
  }
  
  if (b) {
    
    plist::Array *ary = 0;
    
    while (b) {
      
      const char *e = strchr(b, ']');
      
      if (e) {
        
        size_t from = c - current.c_str();
        size_t to = b - current.c_str();
        long idx;
        
        String member = current.substr(from, to-from);
        cprop += member; // might be 0 for multiple dim array... that's ok
        
        from = to + 1;
        to = e - current.c_str();
        
        String number = current.substr(from, to-from);
        
        if (number.find_first_not_of("0123456789") != String::npos) {
          throw plist::Exception(cprop, "Invalid subscript (%s)", number.c_str());
        }
        
        if (sscanf(number.c_str(), "%ld", &idx) != 1) {
          throw plist::Exception(cprop, "Invalid subscript (%s)", number.c_str());
        }
        
        cprop += "[" + number + "]";
                
        // Get or create array
        if (!ary) {
          if (member.length() == 0) {
            throw plist::Exception(cprop, "Missing member name");
          }
          
          if (cdict->value(member)->isNull()) {
            ary = new plist::Array();
            cdict->set(member, ary);
            
          } else if (!cdict->value(member)->checkType(ary)) {
            throw plist::Exception(cprop, "Incompatible types (expected \"array\", got \"%s\")",
                                   PropertyList::ValueTypeName(cdict->value(member)->getType()).c_str());
          
          }
        }
        
        c = e + 1;
        b = strchr(c, '[');
        
        while (idx >= long(ary->size())) {
          ary->append(0);
        }
        
        if (!b) {
          
          if (to+1 != current.length()) {
            throw plist::Exception(cprop, "Characters after subscript");
          }
          
          if (value) {
            
            ary->set(idx, value);
            
          } else {
            plist::Dictionary *subDict = 0;
            
            if (ary->at(idx)->isNull()) {
              subDict = new plist::Dictionary();
              ary->set(idx, subDict);
              
            } else if (!ary->at(idx)->checkType(subDict)) {
              throw plist::Exception(cprop, "Incompatible types (expected \"dict\", got \"%s\")",
                                     PropertyList::ValueTypeName(ary->at(idx)->getType()).c_str());
              
            }
          
            cdict = subDict;
          }
          
        } else {
          
          // it necessarily an array, as we have more subscripts
          plist::Array *subAry = 0;
          
          if (ary->at(idx)->isNull()) {
            subAry = new plist::Array();
            ary->set(idx, subAry);
          
          } else if (!ary->at(idx)->checkType(subAry)) {
            throw plist::Exception(cprop, "Incompatible types (expected \"dict\", got \"%s\")",
                                   PropertyList::ValueTypeName(ary->at(idx)->getType()).c_str());
          
          }
          
          ary = subAry;
          
        }
        
      } else {
        throw plist::Exception(cprop, "Missing close bracket");
      }
    }
    
  } else {
    
    cprop += current;
  
    if (value) {
      
      cdict->set(current, value);
      
    } else {
      
      plist::Dictionary *d = 0;
    
      if (cdict->value(current)->isNull()) {
        d = new plist::Dictionary();
        cdict->set(current, d);
      }
      
      if (!cdict->value(current)->checkType(d)) {
        throw plist::Exception(cprop, "Incompatible types (expected \"dict\", got \"%s\")",
                               PropertyList::ValueTypeName(cdict->value(current)->getType()).c_str());
      }
      
      cdict = d;
    }
  }
}

static plist::Value* GetProperty(plist::Dictionary *dict,
                                 const String &prop) throw(plist::Exception)
{
  if (!dict) {
    throw plist::Exception("", "Passed null dictionary pointer");
  }
  
  plist::Dictionary *cdict = dict;
  
  String current, pre = "", remain = prop;
  
  size_t pos = remain.find('.');
  
  while (pos != String::npos) {
    
    current = remain.substr(0, pos);
    
    remain = remain.substr(pos+1);
    
    GetPropertyMember(cdict, pre, current);
    
    pos = remain.find('.');
    
    if (pre.length() > 0) {
      pre += ".";
    }
    pre += current;
  }
  
  return GetPropertyMember(cdict, pre, remain, true);
}

static void SetProperty(plist::Dictionary *dict,
                        const String &prop,
                        plist::Value *value) throw(plist::Exception)
{
  if (!dict) {
    throw plist::Exception("", "Passed null dictionary pointer");
  }
                          
  plist::Dictionary *cdict = dict;
  
  String current, pre = "", remain = prop;
  
  size_t pos = remain.find('.');
  
  while (pos != String::npos) {
    
    current = remain.substr(0, pos);
    
    remain = remain.substr(pos+1);
  
    SetPropertyMember(cdict, pre, current);
  
    pos = remain.find('.');
    
    if (pre.length() > 0) {
      pre += ".";
    }
    pre += current;
  }
  
  SetPropertyMember(cdict, pre, remain, value);
}

template <typename T>
static typename T::ReturnType GetTypedProperty(plist::Dictionary *dict,
                                               const String &prop) throw(plist::Exception)
{
  const plist::Value *val = GetProperty(dict, prop);
  const T *rv=0;
  if (!val->checkType(rv)) {
    throw plist::Exception(prop, "Invalid type (expected \"%s\", got \"%s\")",
                            T::TypeName(),
                            PropertyList::ValueTypeName(val->getType()).c_str());
  }
  return rv->get();
}

template <typename T>
static void SetTypedProperty(plist::Dictionary *dict,
                             const String &prop,
                             typename T::InputType value) throw(plist::Exception)
{
  T *v = new T(value);
  try {
    SetProperty(dict, prop, v);
  } catch (plist::Exception &e) {
    delete v;
    throw e;
  }
}

size_t PropertyList::getArraySize(const String &p) const throw(plist::Exception) {
  plist::Array::ReturnType ary = GetTypedProperty<plist::Array>(mTop, p);
  return ary.size();
}

size_t PropertyList::getDictKeys(const String &p, StringList &kl) const throw(plist::Exception) {
  const plist::Value *val = GetProperty(mTop, p);
  const plist::Dictionary *dict=0;
  if (!val->checkType(dict)) {
    throw plist::Exception(p, "Invalid type (expected \"%s\", got \"%s\")",
                           plist::Dictionary::TypeName(),
                           PropertyList::ValueTypeName(val->getType()).c_str());
  }
  return dict->keys(kl);
}

bool PropertyList::has(const String &prop) const {
  try {
    GetProperty(mTop, prop);
    return true;
  } catch (...) {
    return false;
  }
}

const String& PropertyList::getString(const String &p) const throw(plist::Exception) {
  return GetTypedProperty<plist::String>(mTop, p);
}

long PropertyList::getInteger(const String &p) const throw(plist::Exception) {
  return GetTypedProperty<plist::Integer>(mTop, p);
}

double PropertyList::getReal(const String &p) const throw(plist::Exception) {
  return GetTypedProperty<plist::Real>(mTop, p);
}

bool PropertyList::getBoolean(const String &p) const throw(plist::Exception) {
  return GetTypedProperty<plist::Boolean>(mTop, p);
}

void PropertyList::setString(const String &prop, const String &str) throw(plist::Exception) {
  SetTypedProperty<plist::String>(mTop, prop, str);
}

void PropertyList::setReal(const String &prop, double val) throw(plist::Exception) {
  SetTypedProperty<plist::Real>(mTop, prop, val);
}

void PropertyList::setInteger(const String &prop, long val) throw(plist::Exception) {
  SetTypedProperty<plist::Integer>(mTop, prop, val);
}

void PropertyList::setBoolean(const String &prop, bool val) throw(plist::Exception) {
  SetTypedProperty<plist::Boolean>(mTop, prop, val);
}

}


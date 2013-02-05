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

plist::Value* plist::InvalidValue::clone() const {
  return new plist::InvalidValue();
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
  if (!elt) {
    return false;
  }
  mValue = elt->getText();
  mValue.strip();
  return true;
}

gcore::XMLElement* plist::String::toXML(gcore::XMLElement *elt) const {
  if (!elt) {
    elt = new gcore::XMLElement("string");
  }
  elt->setText(mValue);
  return elt;
}

plist::Value* plist::String::clone() const {
  return new plist::String(mValue);
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
  if (!elt) {
    elt = new gcore::XMLElement("integer");
  }
  elt->setText(gcore::String(mValue));
  return elt;
}

plist::Value* plist::Integer::clone() const {
  return new plist::Integer(mValue);
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
  if (!elt) {
    return false;
  }
  gcore::String txt = elt->getText();
  return txt.strip().toDouble(mValue);
}

plist::Value* plist::Real::clone() const {
  return new plist::Real(mValue);
}

gcore::XMLElement* plist::Real::toXML(gcore::XMLElement *elt) const {
  if (!elt) {
    elt = new gcore::XMLElement("real");
  }
  elt->setText(gcore::String(mValue));
  return elt;
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
  if (!elt) {
    return false;
  }
  if (elt->getTag() == "false") {
    mValue = false;
    return true;
  } else if (elt->getTag() == "true") {
    mValue = true;
    return true;
  } else {
    gcore::String txt = elt->getText();
    return txt.strip().toBool(mValue);
  }
}

gcore::XMLElement* plist::Boolean::toXML(gcore::XMLElement *elt) const {
  if (!elt) {
    //elt = new gcore::XMLElement("boolean");
    if (mValue) {
      elt = new gcore::XMLElement("true");
    } else {
      elt = new gcore::XMLElement("false");
    }
  } else {
    elt->setText(gcore::String(mValue));
  }
  return elt;
}

plist::Value* plist::Boolean::clone() const {
  return new plist::Boolean(mValue);
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
    // we already have a value at given index
    if (!replace) {
      return;
    }
    if (mValues[idx] == v) {
      // same object
      return;
    }
    if (mValues[idx]) {
      // delete old value
      delete mValues[idx];
      mValues[idx] = 0;
    }
  }
  if (v != 0) {
    // sizing up, filling with zeros
    if (idx >= mValues.size()) {
      mValues.resize(idx+1, 0);
    }
    mValues[idx] = v;
    
  } else {
    if (idx < mValues.size()) {
      mValues[idx] = 0;
      for (size_t i=idx+1; i<mValues.size(); ++i) {
        std::swap(mValues[i-1], mValues[i]);
      }
    }
    long lastIdx = long(mValues.size()) - 1;
    while (lastIdx >= 0 && mValues[lastIdx] == 0) {
      --lastIdx;
    }
    size_t newSize = size_t(lastIdx + 1);
    if (newSize != mValues.size()) {
      mValues.resize(newSize);
    }
  }
}

plist::Value* plist::Array::clone() const {
  plist::Array *a = new plist::Array();
  a->mValues.resize(mValues.size());
  for (size_t i=0; i<mValues.size(); ++i) {
    if (mValues[i]) {
      a->mValues[i] = mValues[i]->clone();
    } else {
      a->mValues[i] = 0;
    }
  }
  return a;
}

bool plist::Array::fromXML(const gcore::XMLElement *elt)  {
  clear();
  if (!elt) {
    return false;
  }
  for (size_t i=0; i<elt->numChildren(); ++i) {
    const gcore::XMLElement *c = elt->getChild(i);
    Value *v = PropertyList::NewValue(c->getTag());
    if (v == 0) {
      if (c->getTag() == "true" || c->getTag() == "false") {
        v = new plist::Boolean();
      } else {
        return false;
      }
    }
    if (!v->fromXML(c)) {
      delete v;
      return false;
    }
    mValues.push(v);
  }
  return true;
}

gcore::XMLElement* plist::Array::toXML(gcore::XMLElement *elt) const {
  if (!elt) {
    elt = new gcore::XMLElement("array");
  }
  for (size_t i=0; i<mValues.size(); ++i) {
    if (mValues[i]) {
      elt->addChild(mValues[i]->toXML());
    }
  }
  return elt;
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

size_t plist::Dictionary::size() const {
  return mPairs.size();
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
  std::map<gcore::String, Value*>::iterator it = mPairs.find(key);
  if (it != mPairs.end()) {
    if (!replace) {
      return;
    }
    if (it->second == v) {
      return;
    }
    delete it->second;
    if (v) {
      it->second = v;
    } else {
      mPairs.erase(it);
    }
  } else {
    if (v) {
      mPairs[key] = v;
    }
  }
}

plist::Value* plist::Dictionary::clone() const {
  plist::Dictionary *d = new plist::Dictionary();
  std::map<gcore::String, Value*>::const_iterator it = mPairs.begin();
  while (it != mPairs.end()) {
    d->mPairs[it->first] = (it->second ? it->second->clone() : 0);
    ++it;
  }
  return d;
}

bool plist::Dictionary::fromXML(const gcore::XMLElement *elt)  {
  clear();
  if (!elt) {
    return false;
  }
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
      if (v->getTag() == "true" || v->getTag() == "false") {
        val = new plist::Boolean();
      } else {
        return false;
      }
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

gcore::XMLElement* plist::Dictionary::toXML(gcore::XMLElement *elt) const {
  if (elt == NULL) {
    elt = new gcore::XMLElement("dict");
  }
  std::map<gcore::String, Value*>::const_iterator it = mPairs.begin();
  while (it != mPairs.end()) {
    if (it->second) {
      gcore::XMLElement *k = new gcore::XMLElement("key");
      k->setText(it->first);
      elt->addChild(k);
      elt->addChild(it->second->toXML());
    }
    ++it;
  }
  return elt;
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

PropertyList::PropertyList(const PropertyList &rhs)
  : mTop(0) {
  if (rhs.mTop != 0) {
    mTop = (plist::Dictionary*) rhs.mTop->clone();
  }
}   

PropertyList::~PropertyList() {
  if (mTop) {
    delete mTop;
  }
}

PropertyList& PropertyList::operator=(const PropertyList &rhs) {
  if (this != &rhs) {
    if (mTop) {
      delete mTop;
      mTop = 0;
    }
    if (rhs.mTop) {
      mTop = (plist::Dictionary*) rhs.mTop->clone();
    }
  }
  return *this;
}

void PropertyList::create() {
  if (mTop) {
    delete mTop;
  }
  mTop = new plist::Dictionary();
}

XMLElement* PropertyList::write(XMLElement *elt) const {
  if (mTop) {
    if (elt == NULL) {
      elt = new XMLElement("dict");
    }
    mTop->toXML(elt);
    return elt;
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
  XMLElement *root = write();
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
          throw plist::Exception(cprop, "Invalid index %ld (array size is %lu)", idx, ary->size());
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

static bool IsValidPropertyName(const String &s)
{
  static Rex nameExp(RAW("[a-zA-Z][a-zA-Z0-9_-]*(\[\d+\])?"));
  return nameExp.match(s);
}

static bool IsValidDottedPropertyName(const String &s)
{
  static Rex nameExp(RAW("[a-zA-Z][.a-zA-Z0-9_-]*(\[\d+\])?"));
  return nameExp.match(s);
}

static bool IsValidDottedPropertyNamePart(const String &s)
{
  static Rex partExp(RAW("[a-zA-Z0-9_-]+"));
  return partExp.match(s);
}

static bool GetPropertyNameParts(const String &prop, StringList &parts)
{
  prop.split('.', parts);

  if (!IsValidPropertyName(parts[0])) {
    return false;
  }

  size_t j = 0;

  for (size_t i=1; i<parts.size(); ++i) {

    if (!IsValidPropertyName(parts[i])) {
      if (parts[i].length() == 0) {
        // found .. pattern or . at end of prop
        return false;

      } else if (!IsValidDottedPropertyNamePart(parts[i])) {
        // invalid part
        return false;

      }
      parts[j] += "." + parts[i];
      if (!IsValidDottedPropertyName(parts[j])) {
        // merged property name is still invalid
        return false;
      }

    } else {
      ++j;
      parts[j] = parts[i];
    }
  }

  parts.resize(j + 1);

  return true;
}

static plist::Value* GetProperty(plist::Dictionary *dict,
                                 const String &prop) throw(plist::Exception)
{
  if (!dict) {
    throw plist::Exception("", "Passed null dictionary pointer");
  }
  
  StringList parts;

  if (!GetPropertyNameParts(prop, parts)) {
    throw plist::Exception("", "Invalid property \"%s\"", prop.c_str());
  }

  String pre = "";
  plist::Dictionary *cdict = dict;

  for (size_t i=0; i+1<parts.size(); ++i) {
    GetPropertyMember(cdict, pre, parts[i]);
    if (pre.length() > 0) {
      pre += ".";
    }
    pre += parts[i];
  }

  return GetPropertyMember(cdict, pre, parts.back(), true);
}

static bool RemoveProperty(plist::Dictionary *dict, const String &prop)
{
  if (!dict || prop.length() == 0) {
    return false;
  }
  
  StringList parts;

  if (!GetPropertyNameParts(prop, parts)) {
    return false;
  }

  String pre = "";
  plist::Dictionary *cdict = dict;

  try {
    for (size_t i=0; i+1<parts.size(); ++i) {
      GetPropertyMember(cdict, pre, parts[i]);
      if (pre.length() > 0) {
        pre += ".";
      }
      pre += parts[i];
    }
  } catch (std::exception &) {
    // invalid property
    return false;
  }

  String &leaf = parts.back();
  size_t len = leaf.length();

  if (leaf[len-1] == ']') {
    // remove array element
    size_t pos = leaf.rfind('[');
    if (pos == String::npos) {
      return false;
    }
    String arrayAttr = leaf.substr(0, pos);
    plist::Array *array = 0;
    if (!cdict->value(arrayAttr)->checkType(array)) {
      // property is not an array
      return false;
    }
    unsigned long index = 0;
    String sindex = leaf.substr(pos + 1, len - pos - 2);
    if (sscanf(sindex.c_str(), "%lu", &index) != 1) {
      // invalid property index
      return false;
    }
    if (index >= array->size()) {
      return false;
    }
    array->set(index, 0);
    if (array->size() == 0) {
      cdict->set(arrayAttr, 0);
    }
    return true;

  } else {
    // remove simple element
    if (cdict->has(leaf)) {
      cdict->set(leaf, 0);
      return true;
    } else {
      return false;
    }
  }
}

static void SetProperty(plist::Dictionary *dict,
                        const String &prop,
                        plist::Value *value) throw(plist::Exception)
{
  if (!dict) {
    throw plist::Exception("", "Passed null dictionary pointer");
  }
  
  StringList parts;

  if (!GetPropertyNameParts(prop, parts)) {
    throw plist::Exception("", "Invalid property name \"%s\"", prop.c_str());
  }

  String pre = "";
  plist::Dictionary *cdict = dict;

  for (size_t i=0; i+1<parts.size(); ++i) {
    SetPropertyMember(cdict, pre, parts[i]);
    if (pre.length() > 0) {
      pre += ".";
    }
    pre += parts[i];
  }

  SetPropertyMember(cdict, pre, parts.back(), value);
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

size_t PropertyList::getSize(const String &p) const throw(plist::Exception) {
  const plist::Value *val = GetProperty(mTop, p);
  const plist::Dictionary *dict = 0;
  const plist::Array *array = 0;
  if (val->checkType(dict)) {
    return dict->size();
  } else if (val->checkType(array)) {
    return array->size();
  } else {
    throw plist::Exception(p, "Invalid type (expected \"%s\" or \"%s\", got \"%s\")",
                           plist::Array::TypeName(),
                           plist::Dictionary::TypeName(),
                           PropertyList::ValueTypeName(val->getType()).c_str());
    return 0;
  }
}

size_t PropertyList::getKeys(const String &p, StringList &kl) const throw(plist::Exception) {
  const plist::Value *val = GetProperty(mTop, p);
  const plist::Dictionary *dict=0;
  if (!val->checkType(dict)) {
    throw plist::Exception(p, "Invalid type (expected \"%s\", got \"%s\")",
                           plist::Dictionary::TypeName(),
                           PropertyList::ValueTypeName(val->getType()).c_str());
  }
  return dict->keys(kl);
}

void PropertyList::clear(const String &p) throw(plist::Exception) {
  plist::Value *val = GetProperty(mTop, p);
  plist::Dictionary *dict = 0;
  plist::Array *array = 0;
  if (val->checkType(dict)) {
    dict->clear();
  } else if (val->checkType(array)) {
    array->clear();
  } else {
    throw plist::Exception(p, "Invalid type (expected \"%s\" or \"%s\", got \"%s\")",
                           plist::Array::TypeName(),
                           plist::Dictionary::TypeName(),
                           PropertyList::ValueTypeName(val->getType()).c_str());
  }
}

bool PropertyList::remove(const String &p) {
  return RemoveProperty(mTop, p);
}

bool PropertyList::has(const String &prop) const {
  try {
    plist::Value *v = GetProperty(mTop, prop);
    if (v != 0) {
      plist::InvalidValue *iv = dynamic_cast<plist::InvalidValue*>(v);
      return (iv == 0);
    } else {
      return false;
    }
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


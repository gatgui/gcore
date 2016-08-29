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
#include <gcore/json.h>
#include <gcore/typetraits.h>
#include <cstdarg>

namespace gcore {

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

Status plist::InvalidValue::fromXML(const gcore::XMLElement *)  {
  return Status(false, "Invalid value.");
}

gcore::XMLElement* plist::InvalidValue::toXML(gcore::XMLElement *) const {
  return NULL;
}

plist::Value* plist::InvalidValue::clone() const {
  return new plist::InvalidValue();
}

// ---

const gcore::String plist::String::DefaultValue("");

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

Status plist::String::fromXML(const gcore::XMLElement *elt)  {
  if (!elt) {
    return Status(false, "NULL element.");
  }
  mValue = elt->getText();
  mValue.strip();
  return Status(true);
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

const long plist::Integer::DefaultValue = 0;

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

Status plist::Integer::fromXML(const gcore::XMLElement *elt)  {
  if (!elt) {
    return Status(false, "NULL element.");
  }
  if (elt->getTag() != "integer") {
    return Status(false, "Invalid tag for integer (%s)", elt->getTag().c_str());
  }
  gcore::String txt = elt->getText();
  if (!txt.strip().toLong(mValue)) {
    return Status(false, "Not a valid integer value (%s)", txt.c_str());
  }
  return Status(true);
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

const double plist::Real::DefaultValue = 0.0;

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

Status plist::Real::fromXML(const gcore::XMLElement *elt)  {
  if (!elt) {
    return Status(false, "NULL element.");
  }
  gcore::String txt = elt->getText();
  if (!txt.strip().toDouble(mValue)) {
    return Status(false, "Invalid real value (%s)", txt.c_str());
  }
  return Status(true);
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

const bool plist::Boolean::DefaultValue = false;

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

Status plist::Boolean::fromXML(const gcore::XMLElement *elt)  {
  if (!elt) {
    return Status(false, "NULL element.");
  }
  if (elt->getTag() == "false") {
    mValue = false;
    return Status(true);
  } else if (elt->getTag() == "true") {
    mValue = true;
    return Status(true);
  } else {
    gcore::String txt = elt->getText();
    if (!txt.strip().toBool(mValue)) {
      return Status(false, "Invalid boolean value (%s)", txt.c_str());
    }
    return Status(true);
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

const List<plist::Value*> plist::Array::DefaultValue;

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

Status plist::Array::fromXML(const gcore::XMLElement *elt)  {
  clear();
  if (!elt) {
    return Status(false, "NULL element.");
  }
  for (size_t i=0; i<elt->numChildren(); ++i) {
    const gcore::XMLElement *c = elt->getChild(i);
    Value *v = PropertyList::NewValue(c->getTag());
    if (v == 0) {
      if (c->getTag() == "true" || c->getTag() == "false") {
        v = new plist::Boolean();
      } else {
        return Status(false, "Empty value for tag '%s'", c->getTag().c_str());
      }
    }
    Status stat = v->fromXML(c);
    if (!stat) {
      delete v;
      return stat;
    }
    mValues.push(v);
  }
  return Status(true);
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

const std::map<gcore::String, plist::Value*> plist::Dictionary::DefaultValue;

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

Status plist::Dictionary::fromXML(const gcore::XMLElement *elt)  {
  clear();
  if (!elt) {
    return Status(false, "NULL element.");
  }
  size_t i = 0;
  while (i < elt->numChildren()) {
    const gcore::XMLElement *k = elt->getChild(i);
    if (k->getTag() != "key") {
      return Status(false, "Missing <key> child tag");
    }
    gcore::String key = k->getText();
    key.strip();
    if (mPairs.find(key) != mPairs.end()) {
      return Status(false, "Duplicate key '%s'", key.c_str());
    }
    const gcore::XMLElement *v = elt->getChild(i+1);
    Value *val = PropertyList::NewValue(v->getTag());
    if (val == 0) {
      if (v->getTag() == "true" || v->getTag() == "false") {
        val = new plist::Boolean();
      } else {
        return Status(false, "Empty value for tag '%s'", v->getTag().c_str());
      }
    }
    Status stat = val->fromXML(v);
    if (!stat) {
      delete val;
      return stat;
    }
    mPairs[key] = val;
    i += 2;
  }
  return Status(true);
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

Status PropertyList::read(const XMLElement *elt) {
  bool newed = true;
  if (!mTop) {
    mTop = new plist::Dictionary();
  } else {
    mTop->clear();
    newed = false;
  }
  Status rv = mTop->fromXML(elt);
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

Status PropertyList::read(const String &filename) {
  Status rv;
  XMLDoc *doc = new XMLDoc();
  if (doc->read(filename)) {
    rv = read(doc->getRoot());
  } else {
    rv = Status(false, "Invalid XML file '%s'", filename.c_str());
  }
  delete doc;
  return rv;
}

// might have a way to merge GetPropertyMember and SetPropertyMember
// common parts in one function

static plist::Value* GetPropertyMember(plist::Dictionary* &cdict,
                                       const String &pre,
                                       const String &current,
                                       bool final,
                                       Status *status=NULL)
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
          if (status) {
            status->set(false, "Property '%s': Invalid index (%s)", cprop.c_str(), number.c_str());
          }
          return NULL;
        }
        
        if (sscanf(number.c_str(), "%ld", &idx) != 1) {
          if (status) {
            status->set(false, "Property '%s': Invalid index (%s)", cprop.c_str(), number.c_str());
          }
          return NULL;
        }
        
        cprop += "[" + number + "]";
                
        if (!ary) {
          if (member.length() == 0) {
            if (status) {
              status->set(false, "Property '%s': Missing member name", cprop.c_str());
            }
            return NULL;
          }
          
          if (!cdict->value(member)->checkType(ary)) {
            if (status) {
              status->set(false, "Property '%s': Incompatible types (expected \"array\" got \"%s\")",
                          cprop.c_str(),
                          PropertyList::ValueTypeName(cdict->value(member)->getType()).c_str());
            }
            return NULL;
          }
        }
        
        c = e + 1;
        b = strchr(c, '[');
        
        if (idx >= long(ary->size())) {
          if (status) {
            status->set(false, "Property '%s': Index out of range (%d for an array of size %u)",
                        cprop.c_str(), idx, ary->size());
          }
          return NULL;
        }
        
        if (!b) {
          
          if (to+1 != current.length()) {
            if (status) {
              status->set(false, "Property '%s': Unexpected characters after index",
                          cprop.c_str());
            }
            return NULL;
          }
          
          if (final) {
            
            if (status) {
              status->set(true);
            }
            return ary->at(idx);
            
          } else {
            plist::Dictionary *subDict = 0;
            
            if (!ary->at(idx)->checkType(subDict)) {
              if (status) {
                status->set(false, "Property '%s': Incompatible types (expected \"dict\" got \"%s\")",
                            cprop.c_str(),
                            PropertyList::ValueTypeName(ary->at(idx)->getType()).c_str());
              }
              return NULL;
            }
          
            cdict = subDict;
          }
          
        } else {
          
          // it necessarily is an array, as we have more subscripts
          plist::Array *subAry = 0;
          
          if (!ary->at(idx)->checkType(subAry)) {
            if (status) {
              status->set(false, "Property '%s': Incompatible types (expected \"array\" got \"%s\")",
                          cprop.c_str(),
                          PropertyList::ValueTypeName(ary->at(idx)->getType()).c_str());
            }
            return NULL;
          }
          
          ary = subAry;
          
        }
        
      } else {
        if (status) {
          status->set(false, "Property '%s': Unclosed bracket.",
                      cprop.c_str());
        }
        return NULL;
      }
    }
    
  } else {
    
    cprop += current;
  
    if (final) {
      
      if (status) {
        status->set(true);
      }
      return cdict->value(current);
      
    } else {
      
      plist::Dictionary *d = 0;
    
      if (!cdict->value(current)->checkType(d)) {
        if (status) {
          status->set(false, "Property '%s': Incompatible types (expected \"dict\" got \"%s\")",
                      cprop.c_str(),
                      PropertyList::ValueTypeName(cdict->value(current)->getType()).c_str());
        }
        return NULL;
      }
      
      cdict = d;
    }
  }
  
  if (status) {
    status->set(true);
  }
  return NULL;
}

static Status SetPropertyMember(plist::Dictionary* &cdict,
                                const String &pre,
                                const String &current,
                                plist::Value *value=0)
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
          return Status(false, "Property '%s': Invalid index (%s)",
                        cprop.c_str(), number.c_str());
        }
        
        if (sscanf(number.c_str(), "%ld", &idx) != 1) {
          return Status(false, "Property '%s': Invalid index (%s)",
                        cprop.c_str(), number.c_str());
        }
        
        cprop += "[" + number + "]";
                
        // Get or create array
        if (!ary) {
          if (member.length() == 0) {
            return Status(false, "Property '%s': Missing member name", cprop.c_str());
          }
          
          if (cdict->value(member)->isNull()) {
            ary = new plist::Array();
            cdict->set(member, ary);
            
          } else if (!cdict->value(member)->checkType(ary)) {
            return Status(false, "Property '%s': Incompatible types (expected \"array\" got \"%s\")",
                          cprop.c_str(),
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
            return Status(false, "Property '%s': Unexpected characters after index", cprop.c_str());
          }
          
          if (value) {
            
            ary->set(idx, value);
            
          } else {
            plist::Dictionary *subDict = 0;
            
            if (ary->at(idx)->isNull()) {
              subDict = new plist::Dictionary();
              ary->set(idx, subDict);
              
            } else if (!ary->at(idx)->checkType(subDict)) {
              return Status(false, "Property '%s': Incompatible types (expected \"dict\" got \"%s\")",
                            cprop.c_str(),
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
            return Status(false, "Property '%s': Incompatible types (expected \"array\" got \"%s\")",
                          cprop.c_str(),
                          PropertyList::ValueTypeName(ary->at(idx)->getType()).c_str());
          }
          
          ary = subAry;
          
        }
        
      } else {
        return Status(false, "Property '%s': Unclosed bracket", cprop.c_str());
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
        return Status(false, "Property '%s': Incompatible types (expected \"dict\" got \"%s\")",
                      cprop.c_str(),
                      PropertyList::ValueTypeName(cdict->value(current)->getType()).c_str());
      }
      
      cdict = d;
    }
  }
  
  return Status(true);
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
                                 const String &prop,
                                 Status *status=NULL)
{
  if (!dict) {
    if (status) {
      status->set(false, "Invalid dictionnary");
    }
    return NULL;
  }
  
  StringList parts;

  if (!GetPropertyNameParts(prop, parts)) {
    if (status) {
      std::string msg = "Invalid property \"" + prop + "\"";
      status->set(false, msg.c_str());
    }
    return NULL;
  }

  Status stat;
  String pre = "";
  plist::Dictionary *cdict = dict;

  for (size_t i=0; i+1<parts.size(); ++i) {
    GetPropertyMember(cdict, pre, parts[i], false, &stat);
    if (!stat) {
      if (status) {
        *status = stat;
      }
      return NULL;
    }
    if (pre.length() > 0) {
      pre += ".";
    }
    pre += parts[i];
  }

  return GetPropertyMember(cdict, pre, parts.back(), true, status);
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

  Status stat;
  String pre = "";
  plist::Dictionary *cdict = dict;

  for (size_t i=0; i+1<parts.size(); ++i) {
    GetPropertyMember(cdict, pre, parts[i], false, &stat);
    if (!stat) {
      return false;
    }
    if (pre.length() > 0) {
      pre += ".";
    }
    pre += parts[i];
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

static Status SetProperty(plist::Dictionary *dict,
                          const String &prop,
                          plist::Value *value)
{
  if (!dict) {
    return Status(false, "Invalid dictionary");
  }
  
  StringList parts;

  if (!GetPropertyNameParts(prop, parts)) {
    std::string msg = "Invalid property name \"" + prop + "\"";
    return Status(false, msg.c_str());
  }

  Status stat;
  String pre = "";
  plist::Dictionary *cdict = dict;

  for (size_t i=0; i+1<parts.size(); ++i) {
    stat = SetPropertyMember(cdict, pre, parts[i]);
    if (!stat) {
      return stat;
    }
    if (pre.length() > 0) {
      pre += ".";
    }
    pre += parts[i];
  }

  return SetPropertyMember(cdict, pre, parts.back(), value);
}

template <typename T>
static typename T::ReturnType GetTypedProperty(plist::Dictionary *dict,
                                               const String &prop,
                                               typename NoRef<typename T::ReturnType>::Type *failValue=NULL,
                                               Status *status=NULL)
{
  Status stat;
  const plist::Value *val = GetProperty(dict, prop, &stat);
  if (!stat) {
    if (status) {
      *status = stat;
    }
    return (failValue ? *failValue : T::DefaultValue);
  }
  const T *rv=0;
  if (!val->checkType(rv)) {
    if (status) {
      status->set(false, "Property '%s': Invalid type (expected \"%s\", got \"%s\")",
                  prop.c_str(), T::TypeName(),
                  PropertyList::ValueTypeName(val->getType()).c_str());
    }
    return (failValue ? *failValue : T::DefaultValue);
  }
  return rv->get();
}

template <typename T>
static Status SetTypedProperty(plist::Dictionary *dict,
                               const String &prop,
                               typename T::InputType value)
{
  T *v = new T(value);
  Status stat = SetProperty(dict, prop, v);
  if (!stat) {
    delete v;
  }
  return stat;
}

size_t PropertyList::getSize(const String &p, Status *status) const {
  const plist::Value *val = GetProperty(mTop, p);
  const plist::Dictionary *dict = 0;
  const plist::Array *array = 0;
  
  if (val->checkType(dict)) {
    if (status) {
      status->set(true);
    }
    return dict->size();
  
  } else if (val->checkType(array)) {
    if (status) {
      status->set(true);
    }
    return array->size();
  
  } else {
    if (status) {
      status->set(false, "Property '%s': Invalid type (expected \"%s\" or \"%s\", got \"%s\")",
                  p.c_str(), plist::Array::TypeName(), plist::Dictionary::TypeName(),
                  PropertyList::ValueTypeName(val->getType()).c_str());
    }
    return 0;
  }
}

size_t PropertyList::getKeys(const String &p, StringList &kl, Status *status) const {
  const plist::Value *val = GetProperty(mTop, p);
  const plist::Dictionary *dict=0;
  
  if (!val->checkType(dict)) {
    if (status) {
      status->set(false, "Property '%s': Invalid type (expected \"%s\", got \"%s\")",
                  p.c_str(), plist::Dictionary::TypeName(),
                  PropertyList::ValueTypeName(val->getType()).c_str());
    }
    kl.clear();
    return 0;
    
  } else {
    if (status) {
      status->set(true);
    }
    return dict->keys(kl);
  }
}

Status PropertyList::clear(const String &p) {
  plist::Value *val = GetProperty(mTop, p);
  plist::Dictionary *dict = 0;
  plist::Array *array = 0;
  
  if (val->checkType(dict)) {
    dict->clear();
    return Status(true);
  
  } else if (val->checkType(array)) {
    array->clear();
    return Status(true);
  
  } else {
    return Status(false, "Property '%s': Invalid type (expected \"%s\" or \"%s\", got \"%s\")",
                  p.c_str(), plist::Array::TypeName(), plist::Dictionary::TypeName(),
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

const String& PropertyList::getString(const String &p, Status *status) const {
  return GetTypedProperty<plist::String>(mTop, p, NULL, status);
}

long PropertyList::getInteger(const String &p, Status *status) const {
  return GetTypedProperty<plist::Integer>(mTop, p, NULL, status);
}

double PropertyList::getReal(const String &p, Status *status) const {
  return GetTypedProperty<plist::Real>(mTop, p, NULL, status);
}

bool PropertyList::getBoolean(const String &p, Status *status) const {
  return GetTypedProperty<plist::Boolean>(mTop, p, NULL, status);
}

const String& PropertyList::getString(const String &p, const String &defaultValue) const {
  return GetTypedProperty<plist::String>(mTop, p, &defaultValue);
}

long PropertyList::getInteger(const String &p, long defaultValue) const {
  return GetTypedProperty<plist::Integer>(mTop, p, &defaultValue);
}

double PropertyList::getReal(const String &p, double defaultValue) const {
  return GetTypedProperty<plist::Real>(mTop, p, &defaultValue);
}

bool PropertyList::getBoolean(const String &p, bool defaultValue) const {
  return GetTypedProperty<plist::Boolean>(mTop, p, &defaultValue);
}

Status PropertyList::setString(const String &prop, const String &str) {
  return SetTypedProperty<plist::String>(mTop, prop, str);
}

Status PropertyList::setReal(const String &prop, double val) {
  return SetTypedProperty<plist::Real>(mTop, prop, val);
}

Status PropertyList::setInteger(const String &prop, long val) {
  return SetTypedProperty<plist::Integer>(mTop, prop, val);
}

Status PropertyList::setBoolean(const String &prop, bool val) {
  return SetTypedProperty<plist::Boolean>(mTop, prop, val);
}

bool PropertyList::toJSON(json::Value &v) const {
  if (!mTop) {
    return false;
  }
  
  v.reset();
  
  if (!toJSON(mTop, v)) {
    v.reset();
    return false;
    
  } else {
    return true;
  }
}

bool PropertyList::toJSON(plist::Value *in, json::Value &out) const {
  plist::Dictionary *dval = 0;
  plist::Array *aval = 0;
  plist::Boolean *bval = 0;
  plist::Integer *ival = 0;
  plist::Real *rval = 0;
  plist::String *sval = 0;
  
  if (in->checkType(dval)) {
    out = new json::Object();
    
    std::map<gcore::String, plist::Value*>::const_iterator it = dval->get().begin();
    std::map<gcore::String, plist::Value*>::const_iterator itend = dval->get().end();
    
    while (it != itend) {
      json::Value &member = out[it->first];
      if (!toJSON(it->second, member)) {
        return false;
      }
      ++it;
    }
  } else if (in->checkType(aval)) {
    out = new json::Array();
    
    List<plist::Value*>::const_iterator it = aval->get().begin();
    List<plist::Value*>::const_iterator itend = aval->get().end();
    size_t i = 0;
    
    while (it != itend) {
      out.insert(i, json::Value());
      json::Value &member = out[i];
      if (!toJSON(*it, member)) {
        return false;
      }
      ++it;
      ++i;
    }
    
  } else if (in->checkType(bval)) {
    out = bval->get();
    
  } else if (in->checkType(ival)) {
    out = double(ival->get());
    
  } else if (in->checkType(rval)) {
    out = rval->get();
    
  } else if (in->checkType(sval)) {
    out = sval->get();
    
  } else {
    return false;
  }
  
  return true;
}

}


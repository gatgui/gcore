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

#include <gcore/plist.h>
#include <gcore/regexp.h>
#include <cstdarg>

// --- Utils

static std::string ReadLine(std::istream &is) {
  std::string line;
  std::getline(is, line);
  if (line.length() > 0) {
    size_t p;
    if (line[line.length()-1] == '\r') {
      line.erase(line.length()-1, 1);
    }
    p = line.find_first_not_of(" \t\v");
    if (p != std::string::npos) {
      line.erase(0, p);
    }
    p = line.find_last_not_of(" \t\v");
    if (p != std::string::npos) {
      line.erase(p+1);
    }
  }
#ifdef _DEBUG
  std::cout << ">> \"" << line << "\"" << std::endl;
#endif
  return line;
}

static std::string ReadOpenTag(const std::string &line, std::string &b, std::string &a) {
  static const gcore::Regexp tagopen(IEC("<([-\w]+)(\s+[^>]*)?>"), gcore::REX_CAPTURE);
  gcore::RegexpMatch md;
  if (tagopen.match(line, md)) {
    b = md.pre();
    a = md.post();
    return md.group(1);
  } else {
    b = line;
    a = "";
    return "";
  }
}

static std::string ReadOpenTag(std::istream &is, std::string &b, std::string &a) {
  std::string line = ReadLine(is);
  return ReadOpenTag(line, b, a);
}

static bool ReadCloseTag(const std::string &name, const std::string &line, std::string &b, std::string &a) {
  gcore::Regexp tagclose("</" + name + ">");
  gcore::RegexpMatch md;
  if (tagclose.match(line, md)) {
    b = md.pre();
    a = md.post();
    return true;
  } else {
    a = b = "";
    return false;
  }
}

static void AppendXMLString(std::string &dst, const std::string &str) {
  if (str.length() > 0) {
    if (dst.length() > 0) {
      dst += " ";
    }
    dst += str;
  }
}

static bool ContainsTag(const std::string &line) {
  static const gcore::Regexp tag(IEC("</?[-\w]+>"));
  gcore::RegexpMatch md;
  if (tag.match(line, md)) {
    return true;
  } else {
    return false;
  }
}

static bool ReadKey(std::istream &xml, std::string &remain, std::string &key) {
  
  std::string before, after, tag;
  bool failed = false;
  
  do {
  
    tag = ReadOpenTag(remain, before, after);
  
    if (tag.length() > 0) {
    
      if (tag != "key") {
#ifdef _DEBUG
        std::cerr << "Expected <key> tag, got <" << tag << ">" << std::endl;
#endif
        failed = true;
       
      } else {
        
        key = "";
        
        remain = after;
        
        while (!ContainsTag(remain)) {
          AppendXMLString(key, remain);
          remain = ReadLine(xml);
        }
        
        // we have a tag in the line, check if it is a </key> and no other tag before
        if (!ReadCloseTag("key", remain, before, after)) {
#ifdef _DEBUG
          std::cerr << "Expected </key> tag" << std::endl;
#endif
          failed = true;
          
        } else {
        
          if (ContainsTag(before)) {
#ifdef _DEBUG
            std::cerr << "Found invalid tag before </key>" << std::endl;
#endif
            failed = true;
          
          } else {
          
            AppendXMLString(key, before);
            remain = after;
#ifdef _DEBUG
            std::cout << "Read key: \"" << key << "\"" << std::endl;
#endif
            return true;
          }
        }
      }
      
    } else {
      // no tag, check if remain is empty
      if (remain.length() > 0) {
#ifdef _DEBUG
        std::cerr << "Invalid content (ReadKey): \"" << remain << "\"" << std::endl;
#endif
        failed = true;
      
      } else {
        remain = ReadLine(xml);
      }
    }
  } while (!failed);
  
  return false;
}

static gcore::plist::Value* ReadValue(std::istream &xml, std::string &remain) {
  
  std::string before, after, tag;
  bool failed = false;
  
  do {
    
    tag = ReadOpenTag(remain, before, after);
    
    if (tag.length() > 0) {
      
      if (before.length() > 0) {
#ifdef _DEBUG
        std::cerr << "Invalid content (ReadValue): \"" << before << "\"" << std::endl;
#endif
        failed = true;
      
      } else {
      
        gcore::plist::Value *v = gcore::PropertyList::NewValue(tag);
        
        if (!v) {
#ifdef _DEBUG
          std::cerr << "Could not create value for tag <" << tag << ">" << std::endl;
#endif
          failed = true;
          
        } else {
        
          remain = after;
        
          if (!v->fromXML(xml, remain)) {
            delete v;
            failed = true;
          
          } else {
            return v;
          }
        }
      }
    
    } else {
      
      if (remain.length() > 0) {
#ifdef _DEBUG
        std::cerr << "Invalid content (ReadValue): \"" << remain << "\"" << std::endl;
#endif
        failed = true;
      
      } else {
        
        remain = ReadLine(xml);
      }
    }
    
  } while (!failed);
  
  return 0;
}

// ---

namespace gcore {

plist::Exception::Exception(const std::string &prop)
  : std::exception(), mStr("*** PropertyList Error: [" + prop + "] Unknown") {
}

plist::Exception::Exception(const std::string &prop, const std::string &str)
  : std::exception(), mStr("*** PropertyList Error: [" + prop + "]" + str) {
}

plist::Exception::Exception(const std::string &prop, const char *fmt, ...) {
  char buffer[1024];
  va_list vl;
  va_start(vl, fmt);  
  vsprintf(buffer, fmt, vl);
  va_end(vl);
  mStr = std::string("*** PropertyList Error: [") + prop + "] " + buffer;
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

bool plist::InvalidValue::fromXML(std::istream&, std::string&)  {
  return false;
}

void plist::InvalidValue::toXML(std::ostream &, const std::string &) const {
  return;
}

// ---


plist::String::String()
  : mValue("") {
  mNull = false;
  mType = PropertyList::ValueTypeID("string");
}

plist::String::String(const std::string &str)
  : mValue(str) {
  mNull = false;
  mType = PropertyList::ValueTypeID("string");
}

plist::String::~String() {
}

bool plist::String::fromXML(std::istream &xml, std::string &remain) {
  std::string before, after;
  
  if (remain.length() == 0) {
    remain = ReadLine(xml);
  }
  
  mValue = "";
  
  while (!ReadCloseTag("string", remain, before, after)) {
    if (ContainsTag(remain)) {
#ifdef _DEBUG
      std::cerr << "Invalid <string> content: \"" << remain << "\"" << std::endl;
#endif
      return false;
    }
    AppendXMLString(mValue, remain);
    remain = ReadLine(xml);
  }
  
  if (ContainsTag(before)) {
#ifdef _DEBUG
    std::cerr << "Invalid <string> content: \"" << before << "\"" << std::endl;
#endif
    return false;
  }
  
  AppendXMLString(mValue, before);
  
  remain = after;
  
#ifdef _DEBUG
  std::cout << "Read String: \"" << mValue << "\"" << std::endl;
#endif
  return true;
}

void plist::String::toXML(std::ostream &xml, const std::string &indent="") const {
  xml << indent << "<string>" << mValue << "</string>" << std::endl;
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

bool plist::Integer::fromXML(std::istream &xml, std::string &remain) {
  std::string before, after;
  
  if (remain.length() == 0) {
    remain = ReadLine(xml);
  }
  
  std::string content = "";
  
  while (!ReadCloseTag("integer", remain, before, after)) {
    if (ContainsTag(remain)) {
#ifdef _DEBUG
      std::cerr << "Invalid <integer> content: \"" << remain << "\"" << std::endl;
#endif
      return false;
    }
    AppendXMLString(content, remain);
    remain = ReadLine(xml);
  }
  
  if (ContainsTag(before)) {
#ifdef _DEBUG
    std::cerr << "Invalid <integer> content: \"" << before << "\"" << std::endl;
#endif
    return false;
  }
  
  AppendXMLString(content, before);
  
  if (sscanf(content.c_str(), "%ld", &mValue) != 1) {
#ifdef _DEBUG
    std::cerr << "Invalid <integer> content: \"" << content << "\"" << std::endl;
#endif
    return false;
  }
  
#ifdef _DEBUG
  std::cout << "Read Integer: " << mValue << std::endl;
#endif
  remain = after;
  
  return true;
}

void plist::Integer::toXML(std::ostream &xml, const std::string &indent="") const {
  xml << indent << "<integer>" << mValue << "</integer>" << std::endl;
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

bool plist::Real::fromXML(std::istream &xml, std::string &remain) {
  std::string before, after;
  
  if (remain.length() == 0) {
    remain = ReadLine(xml);
  }
  
  std::string content = "";
  
  while (!ReadCloseTag("real", remain, before, after)) {
    if (ContainsTag(remain)) {
#ifdef _DEBUG
      std::cerr << "Invalid <real> content: \"" << remain << "\"" << std::endl;
#endif
      return false;
    }
    AppendXMLString(content, remain);
    remain = ReadLine(xml);
  }
  
  if (ContainsTag(before)) {
#ifdef _DEBUG
    std::cerr << "Invalid <real> content: \"" << before << "\"" << std::endl;
#endif
    return false;
  }
  
  AppendXMLString(content, before);
  
  if (sscanf(content.c_str(), "%lf", &mValue) != 1) {
#ifdef _DEBUG
    std::cerr << "Invalid <real> content: \"" << content << "\"" << std::endl;
#endif
    return false;
  }
  
#ifdef _DEBUG
  std::cout << "Read Real: " << mValue << std::endl;
#endif
  remain = after;
  
  return true;
}

void plist::Real::toXML(std::ostream &xml, const std::string &indent="") const {
  xml << indent << "<real>" << mValue << "</real>" << std::endl;
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

bool plist::Boolean::fromXML(std::istream &xml, std::string &remain) {
  static const Regexp trueexp(IEC("^\s*true\s*$"), REX_ICASE);
  static const Regexp falseexp(IEC("^\s*false\s*$"), REX_ICASE);
  
  std::string before, after;
  
  if (remain.length() == 0) {
    remain = ReadLine(xml);
  }
  
  std::string content = "";
  
  while (!ReadCloseTag("boolean", remain, before, after)) {
    if (ContainsTag(remain)) {
#ifdef _DEBUG
      std::cerr << "Invalid <boolean> content: \"" << remain << "\"" << std::endl;
#endif
      return false;
    }
    AppendXMLString(content, remain);
    remain = ReadLine(xml);
  }
  
  if (ContainsTag(before)) {
#ifdef _DEBUG
    std::cerr << "Invalid <boolean> content: \"" << before << "\"" << std::endl;
#endif
    return false;
  }
  
  AppendXMLString(content, before);
  gcore::RegexpMatch md;
  
  if (!trueexp.match(content, md)) {
    if (!falseexp.match(content, md)) {
#ifdef _DEBUG
      std::cerr << "Invalid <boolean> content: \"" << content << "\"" << std::endl;
#endif
      return false;
    } else {
      mValue = false;
    }
  } else {
    mValue = true;
  }
  
#ifdef _DEBUG
  std::cout << "Read Boolean: " << (mValue ? "true" : "false") << std::endl;
#endif
  remain = after;
  
  return true;
}

void plist::Boolean::toXML(std::ostream &xml, const std::string &indent="") const {
  xml << indent << "<boolean>" << (mValue ? "true" : "false") << "</boolean>" << std::endl;
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

plist::Array::Array(const std::vector<Value*> &val)
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
    mValues.push_back(0);
  }
  mValues[idx] = v;
}

bool plist::Array::fromXML(std::istream &xml, std::string &remain) {
  
  clear();
  
  std::string before, after;
  
  if (remain.length() == 0) {
    remain = ReadLine(xml);
  }
  
  while (!ReadCloseTag("array", remain, before, after)) {
    
    std::string tag = ReadOpenTag(remain, before, after);
    
    if (tag.length() > 0) {
      // Found an opening tag in remaining of line
    
      if (before.length() > 0) {
#ifdef _DEBUG
        std::cerr << "Found invalid content in <array>: \"" << before << "\"" << std::endl;
#endif
        return false;
      }
      
      Value *v = PropertyList::NewValue(tag);
      
      if (!v) {
#ifdef _DEBUG
        std::cerr << "Could not create value of type \"" << tag << "\"" << std::endl;
#endif
        return false;
      }
      
      if (!v->fromXML(xml, after)) {
        delete v;
        return false;
      }

#ifdef _DEBUG
      std::cout << "Append value to array" << std::endl;
#endif
      mValues.push_back(v);
      
      remain = after;
      if (remain.length() == 0) {
        remain = ReadLine(xml);
      }
      
    } else {
    
      if (remain.length() > 0) {
#ifdef _DEBUG
        std::cerr << "Found invalid content in <array>: \"" << remain << "\"" << std::endl;
#endif
        return false;
      }
      
      remain = ReadLine(xml);
    }
  }
  
  // found closing tag, before should be empty or blank
  if (before.length() > 0) {
#ifdef _DEBUG
    std::cerr << "Found invalid content in <array>: \"" << before << "\"" << std::endl;
#endif
    return false;
  }
  
  remain = after;
  
  return true;
}

void plist::Array::toXML(std::ostream &xml, const std::string &indent="") const {
  xml << indent << "<array>" << std::endl;
  for (size_t i=0; i<mValues.size(); ++i) {
    if (mValues[i]) {
      mValues[i]->toXML(xml, indent+"  ");
    }
  }
  xml << indent << "</array>" << std::endl;
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

plist::Dictionary::Dictionary(const std::map<std::string, Value*> &val)
  : mPairs(val) {
  mNull = false;
  mType = PropertyList::ValueTypeID("dict");
}

plist::Dictionary::~Dictionary() {
  clear();
}
  	
void plist::Dictionary::clear() {
  std::map<std::string, Value*>::iterator it = mPairs.begin();
  while (it != mPairs.end()) {
    if (it->second) {
      delete it->second;
    }
    ++it;
  }
  mPairs.clear();
}

bool plist::Dictionary::has(const std::string &key) const {
  return (mPairs.find(key) != mPairs.end());
}

plist::Value* plist::Dictionary::value(const std::string &key) {
  static plist::InvalidValue dummy = plist::InvalidValue();
  std::map<std::string, Value*>::iterator it = mPairs.find(key);
  if (it == mPairs.end()) {
    return &dummy;
  }
  if (!it->second) {
    return &dummy;
  }
  return it->second;
}

const plist::Value* plist::Dictionary::value(const std::string &key) const {
  static plist::InvalidValue dummy = plist::InvalidValue();
  std::map<std::string, Value*>::const_iterator it = mPairs.find(key);
  if (it == mPairs.end()) {
    return &dummy;
  }
  if (!it->second) {
    return &dummy;
  }
  return it->second;
}

void plist::Dictionary::set(const std::string &key, Value *v, bool replace) {
  if (has(key)) {
    if (!replace) {
      return;
    }
    delete mPairs[key];
  }
  mPairs[key] = v;
}

bool plist::Dictionary::fromXML(std::istream &xml, std::string &remain) {
  
  clear();
  
  std::string before, after, key, tag;
  
  if (remain.length() == 0) {
    remain = ReadLine(xml);
  }
  
  while (!ReadCloseTag("dict", remain, before, after)) {
  
    if (!ReadKey(xml, remain, key)) {
      return false;
    }
    
    if (mPairs.find(key) != mPairs.end()) {
#ifdef _DEBUG
      std::cerr << "Key already exists: \"" << key << "\"" << std::endl;
#endif
      return false;
    }
    
    Value *v = ReadValue(xml, remain); 
    if (!v) {
      return false;
    }
    
#ifdef _DEBUG
    std::cout << "Add key to dict: \"" << key << "\"" << std::endl;
#endif
    mPairs[key] = v;
    
    if (remain.length() == 0) {
      remain = ReadLine(xml);
    }
  }
  
  if (before.length() > 0) {
#ifdef _DEBUG
    std::cerr << "Found invalid content in <dict>: \"" << before << "\"" << std::endl;
#endif
    return false;
  }
  
  remain = after;
  
  return true;
}

void plist::Dictionary::toXML(std::ostream &xml, const std::string &indent="") const {
  xml << indent << "<dict>" << std::endl;
  std::map<std::string, Value*>::const_iterator it = mPairs.begin();
  std::string sindent = indent + "  ";
  
  while (it != mPairs.end()) {
    if (it->second) {
      xml << sindent << "<key>" << it->first << "</key>" << std::endl;
      it->second->toXML(xml, sindent);
    }
    ++it;
  }
  
  xml << indent << "</dict>" << std::endl;
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

plist::Value* PropertyList::NewValue(const std::string &type) {
  ValueDescIterator it = msValueDesc.find(type);
  if (it != msValueDesc.end()) {
    plist::Value *v = it->second.ctor();
    return v;
  } else {
    return 0;
  }
}

long PropertyList::ValueTypeID(const std::string &type) {
  ValueDescIterator it = msValueDesc.find(type);
  if (it != msValueDesc.end()) {
    return it->second.id;
  } else {
    return -1;
  }
}

const std::string& PropertyList::ValueTypeName(long id) {
  static const std::string null = "Null";
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
  
void PropertyList::write(const std::string &filename) const {
  std::ofstream xml(filename.c_str());
  xml << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" standalone=\"yes\" ?>"
      << std::endl;
  if (mTop) {
    mTop->toXML(xml, "");
  }
  xml << std::endl;
}

bool PropertyList::read(const std::string &filename) {
  static const gcore::Regexp header(IEC("<\?xml\s.*\?>"));
  
  if (mTop) {
    delete mTop;
    mTop = 0;
  }
  
  std::ifstream xml(filename.c_str());
  
  if (!xml.is_open()) {
#ifdef _DEBUG
    std::cerr << "Could not read file: \"" << filename << "\"" << std::endl;
#endif
    return false;
  }
  
  std::string line = ReadLine(xml);
  
  gcore::RegexpMatch md;
  
  if (!header.match(line, md)) {
#ifdef _DEBUG
    std::cerr << "Missing XML header" << std::endl;
#endif
    return false;
  }
  
  std::string before, after;
  std::string tag = ReadOpenTag(xml, before, after);

  if (tag != "dict") {
#ifdef _DEBUG
    std::cerr << "First TAG must be \"dict\"" << std::endl;
#endif
    return false;
  }

  mTop = new plist::Dictionary();

  if (!mTop) {
    return false;
  }
  
  if (!mTop->fromXML(xml, after)) {
    delete mTop;
    mTop = 0;
    return false;
  }
  
  return true;
}

// might have a way to merge GetPropertyMember and SetPropertyMember
// common parts in one function

static plist::Value* GetPropertyMember(plist::Dictionary* &cdict,
                                        const std::string &pre,
                                        const std::string &current,
                                        bool final=false) throw(plist::Exception) {

  std::string cprop = pre;

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
        
        std::string member = current.substr(from, to-from);
        cprop += member; // might be 0 for multiple dim array... that's ok
        
        from = to + 1;
        to = e - current.c_str();
        
        std::string number = current.substr(from, to-from);
        
        if (number.find_first_not_of("0123456789") != std::string::npos) {
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
                              const std::string &pre,
                              const std::string &current,
                              plist::Value *value=0) throw(plist::Exception) {

  std::string cprop = pre;

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
        
        std::string member = current.substr(from, to-from);
        cprop += member; // might be 0 for multiple dim array... that's ok
        
        from = to + 1;
        to = e - current.c_str();
        
        std::string number = current.substr(from, to-from);
        
        if (number.find_first_not_of("0123456789") != std::string::npos) {
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
                                  const std::string &prop) throw(plist::Exception) {
  if (!dict) {
    throw plist::Exception("", "Passed null dictionary pointer");
  }
  
  plist::Dictionary *cdict = dict;
  
  std::string current, pre = "", remain = prop;
  
  size_t pos = remain.find('.');
  
  while (pos != std::string::npos) {
    
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
                         const std::string &prop,
                         plist::Value *value) throw(plist::Exception) {
  
  if (!dict) {
    throw plist::Exception("", "Passed null dictionary pointer");
  }
                          
  plist::Dictionary *cdict = dict;
  
  std::string current, pre = "", remain = prop;
  
  size_t pos = remain.find('.');
  
  while (pos != std::string::npos) {
    
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
                                                 const std::string &prop) throw(plist::Exception) {
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
                              const std::string &prop,
                              typename T::InputType value) throw(plist::Exception) {
  T *v = new T(value);
  try {
    SetProperty(dict, prop, v);
  } catch (plist::Exception &e) {
    delete v;
    throw e;
  }
}

unsigned long PropertyList::getArraySize(const std::string &p) const throw(plist::Exception) {
  plist::Array::ReturnType ary = GetTypedProperty<plist::Array>(mTop, p);
  return (unsigned long)(ary.size());
}

const std::string& PropertyList::getString(const std::string &p) const throw(plist::Exception) {
  return GetTypedProperty<plist::String>(mTop, p);
}

long PropertyList::getInteger(const std::string &p) const throw(plist::Exception) {
  return GetTypedProperty<plist::Integer>(mTop, p);
}

double PropertyList::getReal(const std::string &p) const throw(plist::Exception) {
  return GetTypedProperty<plist::Real>(mTop, p);
}

bool PropertyList::getBoolean(const std::string &p) const throw(plist::Exception) {
  return GetTypedProperty<plist::Boolean>(mTop, p);
}

void PropertyList::setString(const std::string &prop, const std::string &str) throw(plist::Exception) {
  SetTypedProperty<plist::String>(mTop, prop, str);
}

void PropertyList::setReal(const std::string &prop, double val) throw(plist::Exception) {
  SetTypedProperty<plist::Real>(mTop, prop, val);
}

void PropertyList::setInteger(const std::string &prop, long val) throw(plist::Exception) {
  SetTypedProperty<plist::Integer>(mTop, prop, val);
}

void PropertyList::setBoolean(const std::string &prop, bool val) throw(plist::Exception) {
  SetTypedProperty<plist::Boolean>(mTop, prop, val);
}

}


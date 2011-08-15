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

#include <gcore/xml.h>

namespace gcore {

static bool IsValidAttribute(const String &str) {
  const char *cp = str.c_str();
  while (*cp != '\0') {
    if ((*cp >= 'a' && *cp <= 'z') ||
        (*cp >= 'A' && *cp <= 'Z') ||
        (*cp >= '0' && *cp <= '9') ||
        *cp == '-' ||
        *cp == '_' ||
        *cp == ':') {
      ++cp;
    } else {
      return false;
    }
  }
  return true;
}

static bool IsValidValue(const String &str) {
  return (str.find_first_of("\"<>") == String::npos);
}

static bool IsValidText(const String &txt) {
  return (txt.find_first_of("\"<>") == String::npos);
}

static bool IsWS(char c) {
  if (c == ' ' || c == '\t' || c == '\v' || c == '\n' || c == '\r') {
    return true;
  } else {
    return false;
  }
}

/*
static bool IsValidXMLChar(char c) {
  if ((c >= '0' && c <= '9') ||
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c == '-') ||
      (c == '_') ||
      (c == ':')) {
    return true;
  } else {
    return false;
  }
}
*/

static char* SkipWS(char *p, char *upTo) {
  while (p != upTo) {
    if (IsWS(*p)) {
      ++p;
    } else {
      break;
    }
  }
  return p;
}

static char* SkipNonWS(char *p, char *upTo) {
  while (p != upTo) {
    if (!IsWS(*p)) {
      ++p;
    } else {
      break;
    }
  }
  return p;
}

// ---

const String XMLElement::Empty = "";

XMLElement::XMLElement()
  : mTag(""), mParent(0), mTextIsCDATA(false) {
}

XMLElement::XMLElement(const String &tag)
  : mTag(tag), mParent(0), mTextIsCDATA(false) {
}

XMLElement::~XMLElement() {
  mAttrs.clear();
  for (size_t i=0; i<mChildren.size(); ++i) {
    delete mChildren[i];
  }
  mChildren.clear();
}

bool XMLElement::addChild(XMLElement *elt) {
  if (elt->mParent) {
    if (elt->mParent != this) {
      std::cerr << "Element already has differennt parent" << std::endl;
      return false;
    }
    return true;
  } else {
    elt->mParent = this;
    mChildren.push(elt);
    return true;
  }
}

void XMLElement::write(std::ostream &os, const String &indent) const {
  os << indent << "<" << mTag;
  StringDict::const_iterator it = mAttrs.begin();
  while (it != mAttrs.end()) {
    os << " " << it->first << "=\"" << it->second << "\"";
    ++it;
  }
  if (mChildren.size() > 0 || mText.length() > 0) {
    //os << ">" << std::endl;
    os << ">";
    
    if (mChildren.size() > 0) {
      os << std::endl;
      String childIndent = indent + "  ";
      for (size_t i=0; i<mChildren.size(); ++i) {
        mChildren[i]->write(os, childIndent);
      }
      os << indent;
    }
    
    if (mText.length() > 0) {
      if (mTextIsCDATA) {
        os << "<![CDATA[";
        os << mText;
        os << "]]>";
      } else {
        os << mText;
      }
      if (mChildren.size() > 0) {
        os << std::endl << indent;
      }
    }
    
    os << "</" << mTag << ">" << std::endl;
    
  } else {
    os << " />" << std::endl;
  }
}

bool XMLElement::setAttribute(const String &name, const String &value) {
  if (!IsValidAttribute(name)) {
    std::cerr << "Invalid attribute name: \"" << name << "\"" << std::endl;
    return false;
  }
  if (!IsValidValue(value)) {
    std::cerr << "Invalid characters in attribute \"" << name << "\" value: \"" << value << "\"" << std::endl;
    return false;
  }
  mAttrs[name] = value;
  return true;
}

bool XMLElement::setText(const String &str, bool asCDATA) {
  mTextIsCDATA = asCDATA;
  if (!asCDATA && !IsValidText(str)) {
    std::cerr << "Invalid characters in text" << std::endl;
    return false;
  }
  mText = str;
  return true;
}

bool XMLElement::addText(const String &str) {
  if (mTextIsCDATA) {
    std::cerr << "Element cannot have both text and CDATA" << std::endl;
    return false;
  }
  if (!IsValidText(str)) {
    std::cerr << "Invalid characters in text" << std::endl;
    return false;
  }
  mText += str;
  return true;
}

const XMLElement* XMLElement::getParent() const {
  return mParent;
}

XMLElement* XMLElement::getParent() {
  return mParent;
}

const XMLElement* XMLElement::getChild(size_t idx) const {
  if (idx < mChildren.size()) {
    return mChildren[idx];
  }
  return 0;
}

XMLElement* XMLElement::getChild(size_t idx) {
  if (idx < mChildren.size()) {
    return mChildren[idx];
  }
  return 0;
}

size_t XMLElement::numChildren() const {
  return mChildren.size();
}

bool XMLElement::hasAttribute(const String &name) const {
  return (mAttrs.find(name) != mAttrs.end());
}

const String& XMLElement::getAttribute(const String &name) const {
  StringDict::const_iterator it = mAttrs.find(name);
  if (it != mAttrs.end()) {
    return it->second;
  } else {
    return Empty;
  }
}  

const String& XMLElement::getText() const {
  return mText;
}

void XMLElement::setTag(const String &tag) {
  mTag = tag;
}

const String& XMLElement::getTag() const {
  return mTag;
}

bool XMLElement::hasChildWithTag(const String &tag) const {
  for (size_t i=0; i<mChildren.size(); ++i) {
    if (mChildren[i]->getTag() == tag) {
      return true;
    }
  }
  return false;
}

size_t XMLElement::numChildrenWithTag(const String &tag) const {
  size_t cnt = 0;
  for (size_t i=0; i<mChildren.size(); ++i) {
    if (mChildren[i]->getTag() == tag) {
      ++cnt;
    }
  }
  return cnt;
}

XMLElement* XMLElement::getChildWithTag(const String &tag, size_t n) {
  size_t cur = 0;
  for (size_t i=0; i<mChildren.size(); ++i) {
    if (mChildren[i]->getTag() == tag) {
      if (cur == n) {
        return mChildren[i];
      }
      ++cur;
    }
  }
  return NULL;
}

const XMLElement* XMLElement::getChildWithTag(const String &tag, size_t n) const {
  size_t cur = 0;
  for (size_t i=0; i<mChildren.size(); ++i) {
    if (mChildren[i]->getTag() == tag) {
      if (cur == n) {
        return mChildren[i];
      }
      ++cur;
    }
  }
  return NULL;
}

// ---

XMLDoc::XMLDoc()
  : mRoot(0) {
}

XMLDoc::~XMLDoc() {
  if (mRoot) {
    delete mRoot;
  }
}

void XMLDoc::setRoot(XMLElement *elt) {
  if (mRoot) {
    delete mRoot;
  }
  mRoot = elt;
}

XMLElement* XMLDoc::getRoot() const {
  return mRoot;
}

void XMLDoc::write(const String &fileName) const {
  if (mRoot) {
    std::ofstream ofile(fileName.c_str());
    if (ofile.is_open()) {
      ofile << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
      mRoot->write(ofile, "");
      ofile << std::endl;
    }
  }
}

void XMLDoc::write(std::ostream &os) const {
  if (mRoot) {
    os << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
    mRoot->write(os, "");
    os << std::endl;
  }
}

bool XMLDoc::read(const String &fileName) {
  
  FILE *file = fopen(fileName.c_str(), "rb");
  
  if (!file) {
    return false;
  }
  
  enum {
    READ_OPEN = 0,
    READ_COMMENT,
    READ_CDATA,
    READ_ELEMENT,
    READ_CLOSING_ELEMENT,
    READ_HEADER,
  } state = READ_OPEN;
  
  String pending;
  
  char readBuffer[1024];
  char pendingBuffer[1024];
  size_t nLastChar = 0;
  char lastChars[16];
  size_t nread = 0;
  
  XMLElement *root = 0; // only one allowed, if set all other elements are child
  XMLElement *cur = 0;
  
  
  // read 7 chars less you'll see after
  while ((nread = fread(readBuffer, 1, 1024-7, file)) != 0) {
    
    char *eob = readBuffer + nread;
    
    char *p0 = readBuffer;
    
    while (p0 < eob) {
      
      if (state == READ_OPEN) {
        
        char *p1 = p0;
        
        // advance position until we reach the end of the buffer or the '<' char
        while (p1 != eob) {
          if (*p1 == '<') {
            break;
          }
          ++p1;
        }
        
        size_t plen = p1 - p0;
        memcpy(pendingBuffer, p0, plen);
        pendingBuffer[plen] = '\0';
        //std::cout << "READ_OPEN | Add: \"" << pendingBuffer << "\" to pending buffer" << std::endl;
        pending += pendingBuffer;
        
        if (p1 != eob) {
        
          // add the pending buffer as text to the current element
          if (cur) {
            // if only spaces do not add
            char *ps = (char*) pending.c_str();
            char *pe = ps + pending.length();
            if (SkipWS(ps, pe) != pe) {
              cur->addText(pending);
            }
          }
          pending = "";
          
          // we found the '<' char check following char
          if ((p1+1) == eob) {
            
            // need to read more to decide what we want to read
            // std::cout << "Read a new full line" << std::endl;
            nLastChar = eob - readBuffer;
            if (nLastChar > 16) {
              nLastChar = 16;
            }
            memcpy(lastChars, eob-nLastChar, nLastChar);
            
            nread = fread(readBuffer, 1, 1024-7, file);
            if (nread == 0) {
              std::cerr << "Invalid XML file: non-closed <" << std::endl;
              goto failed;
            }
            
            eob = readBuffer + nread;
            p0 = readBuffer;
            p1 = p0;
          
          } else {
            ++p1;
          }
            
          if (*p1 == '?') {
            ++p1;
            state = READ_HEADER;
            
          } else if (*p1 == '!') {
            // could be "<!-- ... -->" or "<![CDATA[ ... ]]>"
            // need at most 7 more characters to decide ( == len("[CDATA[") )
            
            size_t remain = eob - p1 - 1;
            
            if (remain < 7) {
              // read required number of characters, at most 7 (we always leave place for 7)
              size_t nr = fread(p1+2, 7-remain, 1, file);
              
              nread += nr;
              eob += nr;
              
              remain = eob - p1 - 1;
            }
            
            if (remain < 2) {
              std::cerr << "Invalid XML file: invalid <! construct" << std::endl;
              goto failed;
            
            } else if (*(p1+1) == '-' && *(p1+2) == '-') {
              p1 += 3;
              state = READ_COMMENT;
              
            } else if (remain >= 7) {
              
              if (!strncmp(p1, "![CDATA[", 8)) {
                p1 += 8;
                state = READ_CDATA;
              
              } else {
                std::cerr << "Invalid XML file: invalid <! construct" << std::endl;
                goto failed;
              }
              
            } else {
            
              std::cerr << "Invalid XML file: invalid <! construct" << std::endl;
            }
          
          } else if (*p1 == '/') {
            p1 += 1;
            state = READ_CLOSING_ELEMENT;
          
          } else {
            state = READ_ELEMENT;
            
          }
        }
        
        p0 = p1;
        
      } else if (state == READ_HEADER) {
        
        char *p1 = p0;
        
        // advance position until we reach the end of the buffer or the '<' char
        while (p1 != eob) {
          if (*p1 == '>') {
            
            if (p1 - 1 >= readBuffer) {
              if (*(p1 - 1) != '?') {
                std::cerr << "Invalid XML file: expected '?>' (1)" << std::endl;
                goto failed;
              }
            } else {
              if (nLastChar < 1 || lastChars[15] != '?') {
                std::cerr << "Invalid XML file: expected '?>' (2)" << std::endl;
                goto failed;
              }
            }
            
            break;
          }
          ++p1;
        }
        
        size_t plen = p1 - p0;
        memcpy(pendingBuffer, p0, plen);
        pendingBuffer[plen] = '\0';
        pending += pendingBuffer;
        
        if (p1 != eob) {
          ++p1;
          // remove last ? characters
          if (pending.length() < 1) {
            std::cerr << "Invalid XML file: missing ? closing character" << std::endl;
            goto failed;
          }
          pending.erase(pending.length()-1, 1);
          pending = "";
          state = READ_OPEN;
        }
        
        p0 = p1;
        
      } else if (state == READ_CDATA) {
        
        char *p1 = p0;
        
        // advance position until we reach the end of the buffer or the '<' char
        while (p1 != eob) {
          if (*p1 == '>') {
          
            if (p1 - 1 >= readBuffer) {
              if (*(p1 - 1) != ']') {
                std::cerr << "Invalid XML file: expected ']]>' (1)" << std::endl;
                goto failed;
              }
            } else {
              if (nLastChar < 1 || lastChars[15] != ']') {
                std::cerr << "Invalid XML file: expected ']]>' (2)" << std::endl;
                goto failed;
              }
            }
            
            if (p1 - 2 >= readBuffer) {
              if (*(p1 - 2) != ']') {
                std::cerr << "Invalid XML file: expected ']]>' (3)" << std::endl;
                goto failed;
              }
            } else {
              if (nLastChar < 2 || lastChars[14] != ']') {
                std::cerr << "Invalid XML file: expected ']]>' (4)" << std::endl;
                goto failed;
              }
            }
            
            break;
          }
          ++p1;
        }
        
        size_t plen = p1 - p0;
        memcpy(pendingBuffer, p0, plen);
        pendingBuffer[plen] = '\0';
        pending += pendingBuffer;
        
        if (p1 != eob) {
          ++p1;
          // remove the last ]] characters
          if (pending.length() < 2) {
            std::cerr << "Invalid XML file: missing CDATA ]] closing characters" << std::endl;
            goto failed;
          }
          pending.erase(pending.length()-2, 2);
          //std::cout << "CDATA: \"" << pending << "\"" << std::endl;
          cur->setText(pending, true);
          pending = "";
          state = READ_OPEN;
        }
        
        p0 = p1;
        
      } else if (state == READ_COMMENT) {
        
        char *p1 = p0;
        
        // advance position until we reach the end of the buffer or the '<' char
        while (p1 != eob) {
          if (*p1 == '>') {
            
            if (p1 - 1 >= readBuffer) {
              if (*(p1 - 1) != '-') {
                std::cerr << "Invalid XML file: expected '-->' (1)" << std::endl;
                goto failed;
              }
            } else {
              if (nLastChar < 1 || lastChars[15] != '-') {
                std::cerr << "Invalid XML file: expected '-->' (2)" << std::endl;
                goto failed;
              }
            }
            
            if (p1 - 2 >= readBuffer) {
              if (*(p1 - 2) != '-') {
                std::cerr << "Invalid XML file: expected '-->' (3)" << std::endl;
                goto failed;
              }
            } else {
              if (nLastChar < 2 || lastChars[14] != '-') {
                std::cerr << "Invalid XML file: expected '-->' (4)" << std::endl;
                goto failed;
              }
            }
            
            break;
          }
          ++p1;
        }
        
        size_t plen = p1 - p0;
        memcpy(pendingBuffer, p0, plen);
        pendingBuffer[plen] = '\0';
        pending += pendingBuffer;
        
        if (p1 != eob) {
          ++p1;
          // remove the last 2 characters '--'
          if (pending.length() < 2) {
            std::cerr << "Invalid XML file: missing comment -- closing characters" << std::endl;
            goto failed;
          }
          pending.erase(pending.length()-2, 2);
          std::cout << "Comment: \"" << pending << "\"" << std::endl;
          pending = "";
          state = READ_OPEN;
        }
        
        // previous chars must be --
        p0 = p1;
        
      } else if (state == READ_ELEMENT) {
        
        char *p1 = p0;
        
        // advance position until we reach the end of the buffer or the '<' char
        while (p1 != eob) {
          if (*p1 == '>') {
            break;
          }
          ++p1;
        }
        
        size_t plen = p1 - p0;
        memcpy(pendingBuffer, p0, plen);
        pendingBuffer[plen] = '\0';
        pending += pendingBuffer;
        
        if (p1 != eob) {
          ++p1;
          
          if (pending.length() == 0) {
            std::cerr << "Invalid XML file: empty element" << std::endl;
            goto failed;
          }
          
          bool close = (pending[pending.length()-1] == '/');
          if (close) {
            // remove last / if any
            pending.erase(pending.length()-1, 1);
          }
          
          char *ts = (char*) pending.c_str();
          char *te = ts + pending.length();
          char *tc = SkipNonWS(ts, te);
          size_t len = tc - ts;
          
          if (len == 0) {
            std::cerr << "Invalid XML file: invalid element name" << std::endl;
            goto failed;
          } 
          
          String tag = pending.substr(0, len);
          
          XMLElement *elem = new XMLElement(tag);
          
          while (tc < te) {
            
            tc = SkipWS(tc, te);
            if (tc >= te) {
              break;
            }
            
            size_t o = tc - ts;
            size_t p = pending.find('=', o);
            
            if (p == String::npos) {
              std::cout << "Invalid XML file: missing = for attribute" << std::endl;
              delete elem;
              goto failed;
            }
            
            String attr = pending.substr(o, p-o);
            if (!IsValidAttribute(attr)) {
              std::cout << "Invalid XML file: invalid attribute name \"" << attr << "\"" << std::endl;
              delete elem;
              goto failed;
            }
            
            ++p;
            
            if (p >= pending.length()) {
              std::cout << "Invalid XML file: no value for attribute" << std::endl;
              delete elem;
              goto failed;
            }
            
            if (pending[p] != '"') {
              std::cout << "Invalid XML file: missing opening \" for attribute" << std::endl;
              delete elem;
              goto failed;
            }
            
            ++p;
            
            size_t e = pending.find('"', p);
            while (e != String::npos) {
              if (pending[e-1] == '\\') {
                e = pending.find('"', e+1);
              } else {
                break;
              }
            }
            
            if (e == String::npos) {
              std::cout << "Invalid XML file: missing closing \" for attribute" << std::endl;
              delete elem;
              goto failed;
            }
            
            String val = pending.substr(p, e-p);
            
            elem->setAttribute(attr, val);
            
            tc = ts + e + 1;
          }
          
          if (close) {
            if (cur) {
              cur->addChild(elem);
            }
          } else {
            if (cur) {
              cur->addChild(elem);
            }
            cur = elem;
          }
          if (!root) {
            root = elem;
          }
          
          pending = "";
          state = READ_OPEN;
        }
        
        p0 = p1;
        
      } else if (state == READ_CLOSING_ELEMENT) {
        
        char *p1 = p0;
        
        // advance position until we reach the end of the buffer or the '<' char
        while (p1 != eob) {
          if (*p1 == '>') {
            break;
          }
          ++p1;
        }
        
        size_t plen = p1 - p0;
        memcpy(pendingBuffer, p0, plen);
        pendingBuffer[plen] = '\0';
        pending += pendingBuffer;
        
        if (p1 != eob) {
          ++p1;
          
          if (!cur) {
            std::cerr << "Invalid XML file: Closing tag \"" << pending
                      << "\" has no counter-part opening" << std::endl;
            goto failed; 
          }
          
          if (cur->getTag() != pending) {
            std::cerr << "Invalid XML file: Colsing tag \"" << pending
                      << "\" mismatches opening \"" << cur->getTag() << "\"" << std::endl;
            goto failed;
          }
          
          //std::cout << "Closing element: \"" << pending << "\"" << std::endl;
          
          cur = cur->getParent();
          
          pending = "";
          state = READ_OPEN;
        }
        
        p0 = p1;
        
      } else {
        
        std::cerr << "Invalid parser state" << std::endl;
        goto failed;
      }
    }
    
    // do we have 16 characters
    nLastChar = eob - readBuffer;
    if (nLastChar > 16) {
      nLastChar = 16;
    }
    memcpy(lastChars, eob-nLastChar, nLastChar);  
  }
    
  fclose(file);
  
  setRoot(root);
  
  return true;

failed:
  if (root) {
    delete root;
  } else {
    if (cur) {
      delete cur;
    }
  }
  return false;
}

}


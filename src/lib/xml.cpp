/*

Copyright (C) 2010, 2011  Gaetan Guidet

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
#include <gcore/log.h>

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

static String RemoveEntities(const String &txt) {
  String ent, rv = txt;
  size_t p0 = 0;
  size_t p1 = rv.find('&', p0);
  size_t p2;
  while (p1 != String::npos) {
    p2 = rv.find(';', p1);
    if (p2 != String::npos) {
      ent = rv.substr(p1+1, p2-p1-1);
      if (ent == "gt") {
        rv.erase(p1, p2-p1+1);
        rv.insert(p1, ">");
      } else if (ent == "lt") {
        rv.erase(p1, p2-p1+1);
        rv.insert(p1, "<");
      } else if (ent == "quot") {
        rv.erase(p1, p2-p1+1);
        rv.insert(p1, "\"");
      } else if (ent == "apos") {
        rv.erase(p1, p2-p1+1);
        rv.insert(p1, "'");
      } else if (ent == "amp") {
        rv.erase(p1, p2-p1+1);
        rv.insert(p1, "&");
      }
    } else {
      break;
    }
    p0 = p1 + 1;
    p1 = rv.find('&', p0);
  }
  return rv;
}

static String ValidateString(const String &txt) {
  String rv = txt;
  size_t p0 = 0;
  size_t p1 = rv.find_first_of("<>&\"'");
  while (p1 != String::npos) {
    if (rv[p1] == '<') {
      rv.erase(p1, 1);
      rv.insert(p1, "&lt;");
    } else if (rv[p1] == '>') {
      rv.erase(p1, 1);
      rv.insert(p1, "&gt;");
    } else if (rv[p1] == '"') {
      rv.erase(p1, 1);
      rv.insert(p1, "&quot;");
    } else if (rv[p1] == '\'') {
      rv.erase(p1, 1);
      rv.insert(p1, "&apos;");
    } else if (rv[p1] == '&') {
      size_t p2 = rv.find(';', p1);
      if (p2 != String::npos && (p2-p1) > 2) {
        String ent = rv.substr(p1+1, p2-p1-1);
        if (ent != "gt" && ent != "lt" && ent != "quot" && ent != "apos" && ent != "amp") {
          rv.erase(p1, 1);
          rv.insert(p1, "&amp;");
        }
      } else {
        rv.erase(p1, 1);
        rv.insert(p1, "&amp;");
      }
    } else {
      break;
    }
    p0 = p1 + 1;
    p1 = rv.find_first_of("<>&\"'", p0);
  }
  return rv;
}

static bool IsWS(char c) {
  if (c == ' ' || c == '\t' || c == '\v' || c == '\n' || c == '\r') {
    return true;
  } else {
    return false;
  }
}

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
      Log::PrintError("[gcore] XMLElement::addChild: Element already has a different parent");
      return false;
    }
    return true;
  } else {
    elt->mParent = this;
    mChildren.push(elt);
    return true;
  }
}

void XMLElement::removeChild(XMLElement *elt) {
  List<XMLElement*>::iterator it = std::find(mChildren.begin(), mChildren.end(), elt);
  if (it != mChildren.end()) {
    delete *it;
    mChildren.erase(it);
  }
}

void XMLElement::removeChild(size_t idx) {
  if (idx >= mChildren.size()) {
    return;
  }
  List<XMLElement*>::iterator it = mChildren.begin() + idx;
  delete *it;
  mChildren.erase(it);
}

void XMLElement::write(std::ostream &os, const String &indent) const {
  os << indent << "<" << mTag;
  StringDict::const_iterator it = mAttrs.begin();
  while (it != mAttrs.end()) {
    os << " " << it->first << "=\"" << ValidateString(it->second) << "\"";
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
        os << ValidateString(mText);
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
    Log::PrintError("[gcore] XMLElement::setAttribute: Invalid attribute name \"%s\"", name.c_str());
    return false;
  }
  mAttrs[name] = value;
  return true;
}

void XMLElement::removeAttribute(const String &name) {
  StringDict::iterator it = mAttrs.find(name);
  if (it != mAttrs.end()) {
    mAttrs.erase(it);
  }
}

bool XMLElement::setText(const String &str, bool asCDATA) {
  mTextIsCDATA = asCDATA;
  mText = str;
  return true;
}

bool XMLElement::addText(const String &str) {
  if (mTextIsCDATA) {
    Log::PrintError("[gcore] XMLElement::addText: Element cannot have both text and CDATA");
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

size_t XMLElement::getChildrenWithTag(const String &tag, List<XMLElement*> &el) const {
  el.clear();
  for (size_t i=0; i<mChildren.size(); ++i) {
    if (mChildren[i]->getTag() == tag) {
      el.push_back(mChildren[i]);
    }
  }
  return el.size();
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

bool XMLDoc::read(std::istream &is) {
  
  if (!is.good()) {
    Log::PrintError("[gcore] XMLDoc::read: Bad stream");
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
  int taglevel = 0;

  XMLElement *root = 0; // only one allowed, if set all other elements are child
  XMLElement *cur = 0;

  // read 7 chars less you'll see after
  //while ((nread = fread(readBuffer, 1, 1024-7, file)) != 0) {
  while (is.good()) {
    
    is.read(readBuffer, 1024-7);
    if (is.bad()) {
      goto failed;
    }
    // fail and eof both set if we reached eof
    nread = is.gcount();
    
    if (nread == 0) {
      break;
    }

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
        pending += pendingBuffer;

        if (p1 != eob) {

          // add the pending buffer as text to the current element
          if (cur) {
            // if only spaces do not add
            char *ps = (char*) pending.c_str();
            char *pe = ps + pending.length();
            if (SkipWS(ps, pe) != pe) {
              cur->addText(RemoveEntities(pending));
            }
          }
          pending = "";

          // we found the '<' char check following char
          if ((p1+1) == eob) {

            // need to read more to decide what we want to read
            nLastChar = eob - readBuffer;
            if (nLastChar > 16) {
              nLastChar = 16;
            }
            memcpy(lastChars, eob-nLastChar, nLastChar);

            //nread = fread(readBuffer, 1, 1024-7, file);
            //if (nread == 0) {
            //  goto failed;
            //}
            is.read(readBuffer, 1024-7);
            nread = is.gcount();
            if (is.bad() || nread == 0) {
              Log::PrintError("[gcore] XMLDoc::read: Unclosed <");
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
              
              //size_t nr = fread(p1+2, 7-remain, 1, file);
              is.read(eob, 7-remain);
              if (is.bad()) {
                goto failed;
              }
              size_t nr = is.gcount();

              nread += nr;
              eob += nr;

              remain = eob - p1 - 1;
            }

            if (remain < 2) {
              Log::PrintError("[gcore] XMLDoc::read: Invalid <! construct");
              goto failed;

            } else if (*(p1+1) == '-' && *(p1+2) == '-') {
              p1 += 3;
              state = READ_COMMENT;

            } else if (remain >= 7) {

              if (!strncmp(p1, "![CDATA[", 8)) {
                p1 += 8;
                state = READ_CDATA;

              } else {
                Log::PrintError("[gcore] XMLDoc::read: Invalid <! construct");
                goto failed;
              }

            } else {
              Log::PrintError("[gcore] XMLDoc::read: Invalid <! construct");
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
                Log::PrintError("[gcore] XMLDoc::read: Expected '?>'");
                goto failed;
              }
            } else {
              if (nLastChar < 1 || lastChars[15] != '?') {
                Log::PrintError("[gcore] XMLDoc::read: Expected '?>'");
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
            Log::PrintError("[gcore] XMLDoc::read: Missing ? closing character");
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

            bool found = false;

            if (p1 - 1 >= readBuffer) {
              if (*(p1 - 1) == ']') {
                found = true;
              }
            } else {
              if (nLastChar >= 1 && lastChars[15] == ']') {
                found = true;
              }
            }

            if (found) {

              found = false;

              if (p1 - 2 >= readBuffer) {
                if (*(p1 - 2) == ']') {
                  found = true;
                }
              } else {
                if (nLastChar >= 2 && lastChars[14] == ']') {
                  found = true;
                }
              }
            }

            if (found) {
              break;
            }
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
            Log::PrintError("[gcore] XMLDoc::read: Missing CDATA ]] closing characters");
            goto failed;
          }
          pending.erase(pending.length()-2, 2);
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

            bool found = false;

            if (p1 - 1 >= readBuffer) {
              if (*(p1 - 1) == '-') {
                found = true;
              }
            } else {
              if (nLastChar >= 1 && lastChars[15] == '-') {
                found = true;
              }
            }

            if (found) {

              found = false;

              if (p1 - 2 >= readBuffer) {
                if (*(p1 - 2) == '-') {
                  found = true;
                }
              } else {
                if (nLastChar >= 2 && lastChars[14] == '-') {
                  found = true;
                }
              }
            }

            if (found) {
              break;
            }
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
            Log::PrintError("[gcore] XMLDoc::read: Missing comment -- closing characters");
            goto failed;
          }
          pending.erase(pending.length()-2, 2);
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
            Log::PrintError("[gcore] XMLDoc::read: Empty element");
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
            Log::PrintError("[gcore] XMLDoc::read: Invalid element name");
            goto failed;
          } 

          String tag = pending.substr(0, len);

          XMLElement *elem = new XMLElement(tag);
#ifdef _DEBUG
          std::cout << "Open tag \"" << tag << "\" [level = " << taglevel << "]" << std::endl;
#endif

          while (tc < te) {

            tc = SkipWS(tc, te);
            if (tc >= te) {
              break;
            }

            size_t o = tc - ts;
            size_t p = pending.find('=', o);

            if (p == String::npos) {
              Log::PrintError("[gcore] XMLDoc::read: Missing = for attribute");
              delete elem;
              goto failed;
            }

            String attr = pending.substr(o, p-o);
            if (!IsValidAttribute(attr)) {
              Log::PrintError("[gcore] XMLDoc::read: Invalid attribute name \"%s\"", attr.c_str());
              delete elem;
              goto failed;
            }

            ++p;

            if (p >= pending.length()) {
              Log::PrintError("[gcore] XMLDoc::read: No value for attribute");
              delete elem;
              goto failed;
            }

            if (pending[p] != '"' && pending[p] != '\'') {
              Log::PrintError("[gcore] XMLDoc::read: Missing opening \" or ' for attribute");
              delete elem;
              goto failed;
            }

            char quoteChar = pending[p];

            ++p;

            size_t e = pending.find(quoteChar, p);
            while (e != String::npos) {
              if (pending[e-1] == '\\') {
                e = pending.find(quoteChar, e+1);
              } else {
                break;
              }
            }

            if (e == String::npos) {
              std::string cc;
              cc.push_back(quoteChar);
              Log::PrintError("[gcore] XMLDoc::read: Missing closing %c for attribute", quoteChar);
              delete elem;
              goto failed;
            }

            String val = pending.substr(p, e-p);

            elem->setAttribute(attr, RemoveEntities(val));

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
            taglevel += 1;
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
            Log::PrintError("[gcore] XMLDoc::read: Closing tag \"%s\" has no opening counter-part", pending.c_str());
            goto failed; 
          }

          if (cur->getTag() != pending) {
            Log::PrintError("[gcore] XMLDoc::read: Closing tag \"%s\" mismatches opening \"%s\"", pending.c_str(), cur->getTag().c_str());
            goto failed;
          }

          taglevel -= 1;
#ifdef _DEBUG
          std::cout << "Close tag \"" << cur->getTag() << "\" [level = " << taglevel << "]" << std::endl;
#endif
          cur = cur->getParent();

          pending = "";
          state = READ_OPEN;
        }

        p0 = p1;

      } else {

        Log::PrintError("[gcore] XMLDoc::read: Invalid parser state");
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

  if (state != READ_OPEN) {
    Log::PrintError("[gcore] XMLDoc::read: Missing closing tag");
    goto failed;
  }
  
  if (taglevel != 0) {
    Log::PrintError("[gcore] XMLDoc::read: Unclosed tag <%s>", (cur ? cur->getTag().c_str() : "UNKNOWN"));
    goto failed;
  }
  
  if (root == NULL) {
    Log::PrintError("[gcore] XMLDoc::read: No root element found");
    return false;
  }

  setRoot(root);
  //fclose(file);
  return true;

failed:
  if (root) {
    delete root;
  } else {
    if (cur) {
      delete cur;
    }
  }
  //if (file) {
  //  fclose(file);
  //}
  return false;
}

bool XMLDoc::read(const String &fileName) {
  std::ifstream ifs(fileName.c_str(), std::ifstream::binary);
  return read(ifs);
}

}


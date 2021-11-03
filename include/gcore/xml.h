/*
MIT License

Copyright (c) 2010 Gaetan Guidet

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

#ifndef __gcore_xml_h_
#define __gcore_xml_h_

#include <gcore/string.h>
#include <gcore/list.h>

namespace gcore {
  
  class GCORE_API XMLElement {
    public:
      friend class XMLDoc;
      
      static const String Empty;
      
      XMLElement();
      XMLElement(const String &tag);
      ~XMLElement();
      
      bool addChild(XMLElement *elt);
      void removeChild(XMLElement *elt);
      void removeChild(size_t idx);
      
      const XMLElement* getParent() const;
      XMLElement* getParent();
      
      const XMLElement* getChild(size_t idx) const;
      XMLElement* getChild(size_t idx);
      size_t numChildren() const;
      
      bool setAttribute(const String &name, const String &value);
      void removeAttribute(const String &name);
      bool setText(const String &str, bool asCDATA=false);
      bool addText(const String &str);
      
      bool hasAttribute(const String &name) const;
      const String& getAttribute(const String &name) const;
      size_t getAttributes(StringDict &) const;
      
      const String& getText() const;
      
      void setTag(const String &tag);
      const String& getTag() const;
      
      bool hasChildWithTag(const String &tag) const;
      size_t numChildrenWithTag(const String &tag) const;
      XMLElement* getChildWithTag(const String &tag, size_t n=0);
      const XMLElement* getChildWithTag(const String &tag, size_t n=0) const;
      size_t getChildrenWithTag(const String &tag, List<XMLElement*> &el) const;
    
    protected:
    
      void write(std::ostream &os, const String &indent) const;
      
    private:
      
      String mTag;
      StringDict mAttrs;
      String mText;
      XMLElement *mParent;
      List<XMLElement*> mChildren;
      bool mTextIsCDATA;
  };
  
  class GCORE_API XMLDoc {
    public:
      XMLDoc();
      ~XMLDoc();
      
      void setRoot(XMLElement *elt);
      XMLElement* getRoot() const;

      size_t numRoots() const;
      XMLElement* getRoot(size_t i) const;
      void addRoot(XMLElement *elt);
      
      void write(const String &fileName) const;
      void write(std::ostream &os) const;
      bool read(const String &fileName);
      bool read(std::istream &is);
    
    protected:
      
      List<XMLElement*> mRoots;
  };
}

#endif


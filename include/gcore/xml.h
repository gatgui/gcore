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

#ifndef __gcore_xml_h_
#define __gcore_xml_h_

#include <gcore/config.h>

namespace gcore {
  
  class GCORE_API XMLElement {
    public:
      friend class XMLDoc;
      
      static const std::string Empty;
      
      XMLElement();
      XMLElement(const std::string &tag);
      ~XMLElement();
      
      bool addChild(XMLElement *elt);
      
      const XMLElement* getParent() const;
      XMLElement* getParent();
      
      const XMLElement* getChild(size_t idx) const;
      XMLElement* getChild(size_t idx);
      size_t numChildren() const;
      
      bool setAttribute(const std::string &name, const std::string &value);
      bool setText(const std::string &str);
      bool addText(const std::string &str);
      
      bool hasAttribute(const std::string &name) const;
      const std::string& getAttribute(const std::string &name) const;
      
      const std::string& getText() const;
      
      void setTag(const std::string &tag);
      const std::string& getTag() const;
      
      bool hasChildWithTag(const std::string &tag) const;
      size_t numChildrenWithTag(const std::string &tag) const;
      XMLElement* getChildWithTag(const std::string &tag, size_t n=0);
      const XMLElement* getChildWithTag(const std::string &tag, size_t n=0) const;
    
    protected:
    
      void write(std::ostream &os, const std::string &indent) const;
      
    private:
      
      std::string mTag;
      std::map<std::string, std::string> mAttrs;
      std::string mText;
      XMLElement *mParent;
      std::vector<XMLElement*> mChildren;
  };
  
  class GCORE_API XMLDoc {
    public:
      XMLDoc();
      ~XMLDoc();
      
      void setRoot(XMLElement *elt);
      XMLElement* getRoot() const;
      
      void write(const std::string &fileName) const;
      bool read(const std::string &fileName);
    
    protected:
      
      XMLElement *mRoot;
  };
}

#endif


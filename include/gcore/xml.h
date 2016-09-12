/*

Copyright (C) 2010~  Gaetan Guidet

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

#include <gcore/string.h>
#include <gcore/list.h>

namespace gcore
{
   class GCORE_API XMLElement
   {
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
   
   class GCORE_API XMLDoc
   {
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


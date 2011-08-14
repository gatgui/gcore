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

#ifndef __gcore_plist_h_
#define __gcore_plist_h_

#include <gcore/string.h>
#include <gcore/list.h>
#include <gcore/xml.h>

namespace gcore {
  
  namespace plist {
    class Value;
    class Dictionary;
    
    class GCORE_API Exception : public std::exception {
      public:
        explicit Exception(const gcore::String &prop);
        explicit Exception(const gcore::String &prop, const gcore::String &str);
        explicit Exception(const gcore::String &prop, const char *fmt, ...);
        virtual ~Exception() throw();
        
        virtual const char* what() const throw();
        
        inline const char* property() const {
          return mProp.c_str();
        }
        
      protected:
        gcore::String mStr;
        gcore::String mProp;
    };
  }
  
  class GCORE_API PropertyList {
    public:

      struct ValueDesc {
        plist::Value* (*ctor)();
        long id;
      };

      typedef std::map<String, ValueDesc> ValueDescDict;
      typedef ValueDescDict::iterator ValueDescIterator;
      typedef ValueDescDict::const_iterator ValueDescConstIterator;

    protected:

      static ValueDescDict msValueDesc;
    
    public:
      
      template <typename T>
      static void RegisterType(bool replace=false) {
        String name = T::TypeName();
        ValueDescIterator it = msValueDesc.find(name);
        if (it != msValueDesc.end()) {
          if (replace) {
#ifdef _DEBUG
            std::cout << "Replace registered type: \"" << name << "\"" << std::endl;
#endif
            it->second.ctor = &T::New;
          }
        } else {
#ifdef _DEBUG
          std::cout << "Register type: \"" << name << "\"" << std::endl;
#endif
          ValueDesc vd;
          vd.ctor = &T::New;
          vd.id = (long) msValueDesc.size();
          msValueDesc[name] = vd;
        }
      }

      static void RegisterBasicTypes();

      static void ClearTypes();

      static plist::Value* NewValue(const String &type);

      static long ValueTypeID(const String &type);
      static const String& ValueTypeName(long id);
    
    public:
  
      PropertyList();
      ~PropertyList();
      
      void create();
      
      bool read(const String &filename);
      void write(const String &filename) const;
      
      bool read(const XMLElement *elt);
      XMLElement* write(XMLElement *elt) const;
      
      const String& getString(const String &prop) const throw(plist::Exception);
      long getInteger(const String &prop) const throw(plist::Exception);
      double getReal(const String &prop) const throw(plist::Exception);
      bool getBoolean(const String &prop) const throw(plist::Exception);
      
      size_t getArraySize(const String &prop) const throw(plist::Exception);
      size_t getDictKeys(const String &prop, StringList &keys) const throw(plist::Exception);
      
      bool has(const String &prop) const;
      
      void setString(const String &prop, const String &str) throw(plist::Exception);
      void setReal(const String &prop, double val) throw(plist::Exception);
      void setInteger(const String &prop, long val) throw(plist::Exception);
      void setBoolean(const String &prop, bool val) throw(plist::Exception);
      
      inline class plist::Dictionary* top() {
        return mTop;
      }
  
    protected:
      
      class plist::Dictionary *mTop;
  };
  
  namespace plist {
    
    // add a way the browse property sequentially
    
    class GCORE_API Value {
      public:
        
        Value();
        virtual ~Value();

        virtual bool fromXML(const gcore::XMLElement *elt) = 0;
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt) const = 0;

        inline long getType() const {
          return mType;
        }
        
        inline bool isNull() const {
          return mNull;
        }

        template <typename T>
        bool checkType(T* &out) {
          if (PropertyList::ValueTypeID(T::TypeName()) == mType) {
            out = (T*)this;
            return true;
          } else {
            out = 0;
            return false;
          }
        }

        template <typename T>
        bool checkType(const T* &out) const {
          if (PropertyList::ValueTypeID(T::TypeName()) == mType) {
            out = (const T*)this;
            return true;
          } else {
            out = 0;
            return false;
          }
        }

      protected:

        long mType;
        bool mNull;
    };
    
    class GCORE_API InvalidValue : public Value {
      public:
    
        InvalidValue();
        virtual ~InvalidValue();
        
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt) const;
    };
    
    class GCORE_API String : public Value {
      public:
    
        typedef const gcore::String& ReturnType;
        typedef const gcore::String& InputType;
        typedef gcore::String& OutputType;
    
        String();
        String(const gcore::String &v);
        virtual ~String();
        
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt) const;
        
        inline const gcore::String& get() const {
          return mValue;
        }
        
        inline void set(const gcore::String &v) {
          mValue = v;
        }
        
      public:
        
        static Value* New();
        static const char* TypeName();
        
      protected:
        
        gcore::String mValue;
    };
    
    class GCORE_API Real : public Value {
      public:
    
        typedef double ReturnType;
        typedef double InputType;
        typedef double& OutputType;
    
        Real();
        Real(double v);
        virtual ~Real();
        
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt) const;
        
        inline double get() const {
          return mValue;
        }
        
        inline void set(double v) {
          mValue = v;
        }
        
      public:
        
        static Value* New();
        static const char* TypeName();
        
      protected:
        
        double mValue;
    };
    
    class GCORE_API Integer : public Value {
      public:
      
        typedef long ReturnType;
        typedef long InputType;
        typedef long& OutputType;
    
        Integer();
        Integer(long v);
        virtual ~Integer();
        
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt) const;
        
        inline long get() const {
          return mValue;
        }
        
        inline void set(long v) {
          mValue = v;
        }
        
      public:
        
        static Value* New();
        static const char* TypeName();
        
      protected:
        
        long mValue;
    };
    
    class GCORE_API Boolean : public Value {
      public:
      
        typedef bool ReturnType;
        typedef bool InputType;
        typedef bool& OutputType;
    
        Boolean();
        Boolean(bool v);
        virtual ~Boolean();
        
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt) const;
        
        inline bool get() const {
          return mValue;
        }
        
        inline void set(bool v) {
          mValue = v;
        }
        
      public:
        
        static Value* New();
        static const char* TypeName();
        
      protected:
        
        bool mValue;
    };
    
    class GCORE_API Array : public Value {
      public:
        
        typedef const List<Value*>& ReturnType;
        typedef const List<Value*>& InputType;
        typedef List<Value*>& OutputType;
    
        Array();
        Array(InputType val);
        virtual ~Array();
        
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt) const;
        
        inline size_t size() const {
          return mValues.size();
        }
        
        inline void append(Value *v) {
          mValues.push_back(v);
        }
        
        inline ReturnType get() const {
          return mValues;
        }
        
        void clear();
        
        Value* at(size_t idx);
        const Value* at(size_t idx) const;
        void set(size_t idx, Value *v, bool replace=true);
        
      public:
        
        static Value* New();
        static const char* TypeName();
        
      protected:
        
        List<Value*> mValues;
    };
    
    class GCORE_API Dictionary : public Value {
      public:
        
        typedef const std::map<gcore::String, Value*>& ReturnType;
        typedef const std::map<gcore::String, Value*>& InputType;
        typedef std::map<gcore::String, Value*>& OutputType;
    
        Dictionary();
        Dictionary(InputType val);
        virtual ~Dictionary();
        
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt) const;
        
        inline ReturnType get() const {
          return mPairs;
        }
        
        void clear();
        Value* value(const gcore::String &key);
        const Value* value(const gcore::String &key) const;
        bool has(const gcore::String &key) const;
        size_t keys(gcore::StringList &keys) const;
        void set(const gcore::String &key, Value *v, bool replace=true);
        
      public:
        
        static Value* New();
        static const char* TypeName();
        
      protected:
        
        std::map<gcore::String, Value*> mPairs;
    };
  }
  
}


#endif

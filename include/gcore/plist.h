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

#ifndef __gcore_plist_h_
#define __gcore_plist_h_

#include <gcore/string.h>
#include <gcore/list.h>
#include <gcore/xml.h>
#include <gcore/log.h>

namespace gcore {
  
  namespace json {
    class Value;
  }
  
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
            Log::PrintDebug("[gcore] PropertyList::RegisterType: Replaced registered type \"%s\"", name.c_str());
            it->second.ctor = &T::New;
          }
        } else {
          Log::PrintDebug("[gcore] PropertyList::RegisterType: Registered new type \"%s\"", name.c_str());
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
      PropertyList(const PropertyList &rhs);
      ~PropertyList();
      
      PropertyList& operator=(const PropertyList &rhs);
      
      void create();
      
      bool read(const String &filename);
      void write(const String &filename) const;
      
      bool read(const XMLElement *elt);
      XMLElement* write(XMLElement *elt=NULL) const;
      
      bool toJSON(json::Value &v) const;
      
      // The following 7 methods may throw plist::Exception
      const String& getString(const String &prop) const;
      long getInteger(const String &prop) const;
      double getReal(const String &prop) const;
      bool getBoolean(const String &prop) const;
      // For arrays and dictionaries
      size_t getSize(const String &prop) const;
      // For dictionaries
      size_t getKeys(const String &prop, StringList &keys) const;
      // For arrays and dictionaries
      void clear(const String &prop);
      
      bool remove(const String &prop);
      bool has(const String &prop) const;
      
      // The following 4 methods may throw plist::Exception
      void setString(const String &prop, const String &str);
      void setReal(const String &prop, double val);
      void setInteger(const String &prop, long val);
      void setBoolean(const String &prop, bool val);
      
      inline class plist::Dictionary* top() {
        return mTop;
      }
  
    protected:
      
      bool toJSON(plist::Value *in, json::Value &out) const;
      
    protected:
      
      class plist::Dictionary *mTop;
  };
  
  namespace plist {
    
    // add a way the browse property sequentially
    
    class GCORE_API Value {
      public:
        
        Value();
        virtual ~Value();

        virtual Value* clone() const = 0;
        virtual bool fromXML(const gcore::XMLElement *elt) = 0;
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt=NULL) const = 0;

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
        
        virtual Value* clone() const;
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt=NULL) const;
    };
    
    class GCORE_API String : public Value {
      public:
    
        typedef const gcore::String& ReturnType;
        typedef const gcore::String& InputType;
        typedef gcore::String& OutputType;
    
        String();
        String(const gcore::String &v);
        virtual ~String();
        
        virtual Value* clone() const;
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt=NULL) const;
        
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
        
        virtual Value* clone() const;
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt=NULL) const;
        
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
        
        virtual Value* clone() const;
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt=NULL) const;
        
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
        
        virtual Value* clone() const;
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt=NULL) const;
        
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
        
        virtual Value* clone() const;
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt=NULL) const;
        
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
        
        virtual Value* clone() const;
        virtual bool fromXML(const gcore::XMLElement *elt);
        virtual gcore::XMLElement* toXML(gcore::XMLElement *elt=NULL) const;
        
        inline ReturnType get() const {
          return mPairs;
        }
        
        size_t size() const;
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

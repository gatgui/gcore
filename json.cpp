#include <gcore/gcore>

namespace gcore
{
   namespace json
   {
      class GCORE_API Value
      {
      public:
         enum Type
         {
            NullType = 0,
            BooleanType,
            NumberType,
            StringType,
            ArrayType,
            ObjectType
         };
         
         typedef std::map<gcore::String, Value> Object;
         typedef std::deque<Value> Array;
         
         // Cannot use Object::iterator, Object::const_iterator, Array::iterator,
         //   Array::const_iterator directly in Value class methods as Value class
         //   is yet begin defined
         // Work around by declaring a template class for iterators so that type
         //   instancing is postponed
         
         template <class T>
         class iterator : public T::iterator
         {
         public:
            iterator(const typename T::iterator &rhs)
               : T::iterator(rhs)
            {
            }
            iterator& operator=(const typename T::iterator &rhs)
            {
               T::iterator::operator=(rhs);
               return *this;
            }
         };
         
         template <class T>
         class const_iterator : public T::const_iterator
         {
         public:
            const_iterator(const typename T::iterator &rhs)
               : T::const_iterator(rhs)
            {
            }
            const_iterator(const typename T::const_iterator &rhs)
               : T::const_iterator(rhs)
            {
            }
            const_iterator& operator=(const typename T::const_iterator &rhs)
            {
               T::const_iterator::operator=(rhs);
               return *this;
            }
         };
         
      public:
         
         Value();
         Value(bool b);
         Value(int num);
         Value(float num);
         Value(double num);
         Value(const char *str);
         Value(const gcore::String &str);
         Value(Object *obj); // steals ownership
         Value(const Object &obj);
         Value(Array *arr); // steals ownership
         Value(const Array &arr);
         Value(const Value &rhs);
         ~Value();
         
         Value& operator=(const Value &rhs);
         Value& operator=(bool b);
         Value& operator=(int num);
         Value& operator=(float num);
         Value& operator=(double num);
         Value& operator=(const char *str);
         Value& operator=(const gcore::String &str);
         Value& operator=(Object *obj); // steals ownership
         Value& operator=(const Object &obj);
         Value& operator=(Array *arr); // steals ownership
         Value& operator=(const Array &arr);
         
         Type type() const;
         
         operator bool () const throw(std::runtime_error);
         operator int () const throw(std::runtime_error);
         operator float () const throw(std::runtime_error);
         operator double () const throw(std::runtime_error);
         operator const gcore::String& () const throw(std::runtime_error);
         operator const char* () const throw(std::runtime_error);
         operator const Object& () const throw(std::runtime_error);
         operator const Array& () const throw(std::runtime_error);
         
         operator Object& () throw(std::runtime_error);
         operator Array& () throw(std::runtime_error);
         
         void reset();
         
         size_t size() const;
         void clear();
         
         iterator<Array> abegin() throw(std::runtime_error);
         const_iterator<Array> abegin() const throw(std::runtime_error);
         iterator<Array> aend() throw(std::runtime_error);
         const_iterator<Array> aend() const throw(std::runtime_error);
         const Value& operator[](size_t idx) const throw(std::runtime_error);
         Value& operator[](size_t idx) throw(std::runtime_error);
         
         iterator<Object> obegin() throw(std::runtime_error);
         const_iterator<Object> obegin() const throw(std::runtime_error);
         iterator<Object> oend() throw(std::runtime_error);
         const_iterator<Object> oend() const throw(std::runtime_error);
         iterator<Object> find(const gcore::String &name) throw(std::runtime_error);
         const_iterator<Object> find(const gcore::String &name) const throw(std::runtime_error);
         iterator<Object> find(const char *name) throw(std::runtime_error);
         const_iterator<Object> find(const char *name) const throw(std::runtime_error);
         const Value& operator[](const gcore::String &name) const throw(std::runtime_error);
         Value& operator[](const gcore::String &name) throw(std::runtime_error);
         const Value& operator[](const char *name) const throw(std::runtime_error);
         Value& operator[](const char *name) throw(std::runtime_error);
         
      private:
         Type mType;
         bool mBool;
         double mNum;
         gcore::String mStr;
         Object *mObj;
         Array *mArr;
      };
      
      typedef Value::Object Object;
      typedef Value::iterator<Value::Object> ObjectIterator;
      typedef Value::const_iterator<Value::Object> ObjectConstIterator;
      
      typedef Value::Array Array;
      typedef Value::iterator<Value::Array> ArrayIterator;
      typedef Value::const_iterator<Value::Array> ArrayConstIterator;
      
      class GCORE_API Parser
      {
      public:
         Parser();
         Parser(const char *path);
         ~Parser();
         
         bool read(const char *path);
         
         Value& top();
         const Value& top() const;
         
      private:
         Value mTop;
      };
      
      // Schema and Validator
   }
}

GCORE_API std::ostream& operator<<(std::ostream &os, const gcore::json::Value &value);
GCORE_API std::ostream& operator<<(std::ostream &os, const gcore::json::Object &object);
GCORE_API std::ostream& operator<<(std::ostream &os, const gcore::json::Array &array);

// --- implementation

gcore::json::Value::Value()
   : mType(NullType)
   , mBool(false)
   , mNum(0.0)
   , mStr("")
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(bool b)
   : mType(BooleanType)
   , mBool(b)
   , mNum(0.0)
   , mStr("")
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(int num)
   : mType(NumberType)
   , mBool(false)
   , mNum(num)
   , mStr("")
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(float num)
   : mType(NumberType)
   , mBool(false)
   , mNum(num)
   , mStr("")
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(double num)
   : mType(NumberType)
   , mBool(false)
   , mNum(num)
   , mStr("")
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(const char *str)
   : mType(str ? StringType : NullType)
   , mBool(false)
   , mNum(0.0)
   , mStr(str ? str : "")
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(const gcore::String &str)
   : mType(StringType)
   , mBool(false)
   , mNum(0.0)
   , mStr(str)
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(Object *obj)
   : mType(obj ? ObjectType : NullType)
   , mBool(false)
   , mNum(0.0)
   , mStr("")
   , mObj(obj)
   , mArr(0)
{
}

gcore::json::Value::Value(const Object &obj)
   : mType(ObjectType)
   , mBool(false)
   , mNum(0.0)
   , mStr("")
   , mObj(new gcore::json::Object(obj))
   , mArr(0)
{
}

gcore::json::Value::Value(Array *arr)
   : mType(arr ? ArrayType : NullType)
   , mBool(false)
   , mNum(0.0)
   , mStr("")
   , mObj(0)
   , mArr(arr)
{
}

gcore::json::Value::Value(const Array &arr)
   : mType(ArrayType)
   , mBool(false)
   , mNum(0.0)
   , mStr("")
   , mObj(0)
   , mArr(new gcore::json::Array(arr))
{
}

gcore::json::Value::Value(const gcore::json::Value &rhs)
   : mType(rhs.mType)
   , mBool(rhs.mBool)
   , mNum(rhs.mNum)
   , mStr(rhs.mStr)
   , mObj(0)
   , mArr(0)
{
   if (rhs.mObj)
   {
      mObj = new gcore::json::Object(*(rhs.mObj));
   }
   if (rhs.mArr)
   {
      mArr = new gcore::json::Array(*(rhs.mArr));
   }
}

gcore::json::Value::~Value()
{
   reset();
}

void gcore::json::Value::reset()
{
   if (mObj)
   {
      delete mObj;
      mObj = 0;
   }
   
   if (mArr)
   {
      delete mArr;
      mArr = 0;
   }
   
   mBool = false;
   mNum = 0.0;
   mStr = "";
   mType = NullType;
}

gcore::json::Value& gcore::json::Value::operator=(bool b)
{
   if (mType != BooleanType)
   {
      reset();
      mType = BooleanType;
   }
   
   mBool = b;
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(int num)
{
   if (mType != NumberType)
   {
      reset();
      mType = NumberType;
   }
   
   mNum = num;
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(float num)
{
   if (mType != NumberType)
   {
      reset();
      mType = NumberType;
   }
   
   mNum = num;
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(double num)
{
   if (mType != NumberType)
   {
      reset();
      mType = NumberType;
   }
   
   mNum = num;
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(const char *str)
{
   if (mType != StringType)
   {
      reset();
      mType = StringType;
   }
   
   if (str)
   {
      mStr = str;
   }
   else
   {
      mStr = "";
      mType = NullType;
   }
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(const gcore::String &str)
{
   if (mType != StringType)
   {
      reset();
      mType = StringType;
   }
   
   mStr = str;
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(Object *obj)
{
   if (mType != ObjectType)
   {
      reset();
      mType = ObjectType;
   }
   
   if (obj)
   {
      if (mObj && mObj != obj)
      {
         delete mObj;
      }
      mObj = obj;
   }
   else
   {
      if (mObj)
      {
         delete mObj;
         mObj = 0;
      }
      mType = NullType;
   }
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(const Object &obj)
{
   if (mType != ObjectType)
   {
      reset();
      mType = ObjectType;
   }
   
   if (mObj)
   {
      *mObj = obj;
   }
   else
   {
      mObj = new gcore::json::Object(obj);
   }
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(Array *arr)
{
   if (mType != ArrayType)
   {
      reset();
      mType = ArrayType;
   }
   
   if (arr)
   {
      if (mArr && mArr != arr)
      {
         delete mArr;
      }
      mArr = arr;
   }
   else
   {
      if (mArr)
      {
         delete mArr;
         mArr = 0;
      }
      mType = NullType;
   }
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(const Array &arr)
{
   if (mType != ArrayType)
   {
      reset();
      mType = ArrayType;
   }
   
   if (mArr)
   {
      *mArr = arr;
   }
   else
   {
      mArr = new gcore::json::Array(arr);
   }
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(const gcore::json::Value &rhs)
{
   if (this != &rhs)
   {
      if (mType != rhs.mType)
      {
         reset();
         mType = rhs.mType;
      }
      
      switch (mType)
      {
      case BooleanType:
         mBool = rhs.mBool;
         break;
      case NumberType:
         mNum = rhs.mNum;
         break;
      case StringType:
         mStr = rhs.mStr;
         break;
      case ObjectType:
         if (mObj)
         {
            *mObj = *(rhs.mObj);
         }
         else
         {
            mObj = new gcore::json::Object(*(rhs.mObj));
         }
         break;
      case ArrayType:
         if (mArr)
         {
            *mArr = *(rhs.mArr);
         }
         else
         {
            mArr = new gcore::json::Array(*(rhs.mArr));
         }
         break;
      default:
         break;
      }
   }
   return *this;
}

gcore::json::Value::Type gcore::json::Value::type() const
{
   return mType;
}

gcore::json::Value::operator bool () const throw(std::runtime_error)
{
   if (mType != BooleanType)
   {
      throw std::runtime_error("JSON value is not a boolean");
   }
   return mBool;
}

gcore::json::Value::operator int () const throw(std::runtime_error)
{
   if (mType != NumberType)
   {
      throw std::runtime_error("JSON value is not a number");
   }
   return int(mNum);
}

gcore::json::Value::operator float () const throw(std::runtime_error)
{
   if (mType != NumberType)
   {
      throw std::runtime_error("JSON value is not a number");
   }
   return float(mNum);
}

gcore::json::Value::operator double () const throw(std::runtime_error)
{
   if (mType != NumberType)
   {
      throw std::runtime_error("JSON value is not a number");
   }
   return mNum;
}

gcore::json::Value::operator const gcore::String& () const throw(std::runtime_error)
{
   if (mType != StringType)
   {
      throw std::runtime_error("JSON value is not a string");
   }
   return mStr;
}

gcore::json::Value::operator const char* () const throw(std::runtime_error)
{
   if (mType != StringType)
   {
      throw std::runtime_error("JSON value is not a string");
   }
   return mStr.c_str();
}

gcore::json::Value::operator const Object& () const throw(std::runtime_error)
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("JSON value is not an object");
   }
   return *mObj;
}

gcore::json::Value::operator const Array& () const throw(std::runtime_error)
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("JSON value is not an array");
   }
   return *mArr;
}

gcore::json::Value::operator Object& () throw(std::runtime_error)
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("JSON value is not an object");
   }
   return *mObj;
}

gcore::json::Value::operator Array& () throw(std::runtime_error)
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("JSON value is not an array");
   }
   return *mArr;
}

size_t gcore::json::Value::size() const
{
   if (mType == ArrayType)
   {
      return mArr->size();
   }
   else if (mType == ObjectType)
   {
      return mObj->size();
   }
   else
   {
      return 0;
   }
}

void gcore::json::Value::clear()
{
   if (mType == ArrayType)
   {
      mArr->clear();
   }
   else if (mType == ObjectType)
   {
      mObj->clear();
   }
}

gcore::json::ArrayConstIterator gcore::json::Value::abegin() const throw(std::runtime_error)
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("Value is not an array");
   }
   return mArr->begin();
}

gcore::json::ArrayConstIterator gcore::json::Value::aend() const throw(std::runtime_error)
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("Value is not an array");
   }
   return mArr->end();
}

gcore::json::ArrayIterator gcore::json::Value::abegin() throw(std::runtime_error)
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("Value is not an array");
   }
   return mArr->begin();
}

gcore::json::ArrayIterator gcore::json::Value::aend() throw(std::runtime_error)
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("Value is not an array");
   }
   return mArr->end();
}

const gcore::json::Value& gcore::json::Value::operator[](size_t idx) const throw(std::runtime_error)
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("Value is not an array");
   }
   return mArr->at(idx);
}

gcore::json::Value& gcore::json::Value::operator[](size_t idx) throw(std::runtime_error)
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("Value is not an array");
   }
   return mArr->at(idx);
}

gcore::json::ObjectConstIterator gcore::json::Value::obegin() const throw(std::runtime_error)
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("Value is not an object");
   }
   return mObj->begin();
}

gcore::json::ObjectConstIterator gcore::json::Value::oend() const throw(std::runtime_error)
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("Value is not an object");
   }
   return mObj->end();
}

gcore::json::ObjectConstIterator gcore::json::Value::find(const gcore::String &name) const throw(std::runtime_error)
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("Value is not an object");
   }
   return mObj->find(name);
}

gcore::json::ObjectConstIterator gcore::json::Value::find(const char *name) const throw(std::runtime_error)
{
   gcore::String _name(name);
   return this->find(_name);
}

gcore::json::ObjectIterator gcore::json::Value::obegin() throw(std::runtime_error)
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("Value is not an object");
   }
   return mObj->begin();
}

gcore::json::ObjectIterator gcore::json::Value::oend() throw(std::runtime_error)
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("Value is not an object");
   }
   return mObj->end();
}

gcore::json::ObjectIterator gcore::json::Value::find(const gcore::String &name) throw(std::runtime_error)
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("Value is not an object");
   }
   return mObj->find(name);
}

gcore::json::ObjectIterator gcore::json::Value::find(const char *name) throw(std::runtime_error)
{
   gcore::String _name(name);
   return this->find(_name);
}

const gcore::json::Value& gcore::json::Value::operator[](const gcore::String &name) const throw(std::runtime_error)
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("Value is not an object");
   }
   Object::const_iterator it = mObj->find(name);
   if (it == mObj->end())
   {
      throw std::runtime_error("Invalid object key '" + name + "'");
   }
   return it->second;
}

gcore::json::Value& gcore::json::Value::operator[](const gcore::String &name) throw(std::runtime_error)
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("Value is not an object");
   }
   return (*mObj)[name];
}

const gcore::json::Value& gcore::json::Value::operator[](const char *name) const throw(std::runtime_error)
{
   gcore::String _name(name);
   return this->operator[](_name);
}

gcore::json::Value& gcore::json::Value::operator[](const char *name) throw(std::runtime_error)
{
   gcore::String _name(name);
   return this->operator[](_name);
}

std::ostream& operator<<(std::ostream &os, const gcore::json::Value &value)
{
   switch (value.type())
   {
   case gcore::json::Value::NullType:
      os << "null";
      break;
   case gcore::json::Value::BooleanType:
      os << (bool(value) ? "true" : "false");
      break;
   case gcore::json::Value::NumberType:
      os << double(value);
      break;
   case gcore::json::Value::StringType:
      os << "\"" << (const char*)value << "\"";
      break;
   case gcore::json::Value::ObjectType:
      os << (const gcore::json::Object&)value;
      break;
   case gcore::json::Value::ArrayType:
      os << (const gcore::json::Array&)value;
      break;
   default:
      break;
   }
   return os;
}

std::ostream& operator<<(std::ostream &os, const gcore::json::Object &object)
{
   os << "{";
   size_t i = 0;
   size_t n = object.size();
   gcore::json::Object::const_iterator itend = object.end();
   for (gcore::json::Object::const_iterator it = object.begin(); it != itend; ++it, ++i)
   {
      os << "\"" << it->first.c_str() << "\": " << it->second;
      if (i + 1 < n)
      {
         os << ", ";
      }
   }
   os << "}";
   return os;
}

std::ostream& operator<<(std::ostream &os, const gcore::json::Array &array)
{
   os << "[";
   size_t i = 0;
   size_t n = array.size();
   gcore::json::Array::const_iterator itend = array.end();
   for (gcore::json::Array::const_iterator it = array.begin(); it != itend; ++it, ++i)
   {
      os << *it;
      if (i + 1 < n)
      {
         os << ", ";
      }
   }
   os << "]";
   return os;
}

gcore::json::Parser::Parser()
{
}

gcore::json::Parser::Parser(const char *path)
{
   read(path);
}

gcore::json::Parser::~Parser()
{
}

bool gcore::json::Parser::read(const char *path)
{
   // read line by line
   
   // first rule: 
   //   -> object
   // OR
   //   -> members
   // # => comment
   
   // object
   //    { }
   //    { members }
   // members
   //    pair
   //    pair, members
   // pair
   //    string : value
   // array
   //    [ ]
   //    [ elements ]
   // elements
   //    value
   //    value, elements
   // value
   //    string
   //    number
   //    object
   //    array
   //    true
   //    false
   //    null
   // string
   //    " "
   //    " chars "
   // chars
   //    char
   //    char chars
   // char
   //    any unicode character except " and \ or control char
   //    \"
   //    \\
   //    \/
   //    \b
   //    \f
   //    \n
   //    \r
   //    \t
   //    \u four-hex-digits
   // number
   //    int
   //    int frac
   //    int exp
   //    int frac exp
   // int
   //    digit
   //    digit1-9 digits
   //    - digit
   //    - digit1-9 digits
   // frac
   //    . digits
   // exp
   //    e digits
   // digits
   //    digit
   //    digit digits
   // e
   //    e
   //    e+
   //    e-
   //    E
   //    E+
   //    E-
   
   return false;
}

gcore::json::Value& gcore::json::Parser::top()
{
   return mTop;
}

const gcore::json::Value& gcore::json::Parser::top() const
{
   return mTop;
}

// ---

using namespace gcore;

int main(int, char**)
{
   json::Object top;
   json::Array ary1;
   json::Array ary2;
   json::Object obj1;
   json::Object obj2;
   
   ary1.push_back("Hello");
   ary1.push_back("World");
   ary1.push_back("Goodbye!");
   
   gcore::String val = ary1[1];
   std::cout << "Second element: " << val << std::endl;
   
   obj1["name"] = "James";
   obj1["age"] = 35;
   
   obj2["name"] = "Philip";
   obj2["age"] = 21;
   
   ary2.push_back(obj1);
   ary2.push_back(obj2);
   
   top["myflt"] = json::Value(10.0f);
   top["myarray"] = ary1;
   top["objarray"] = ary2;
   
   std::cout << "Is a number? " << (top["myflt"].type() == json::Value::NumberType) << std::endl;
   std::cout << top["objarray"][1]["name"] << std::endl;
   std::cout << top << std::endl;
   
   // --- generic access ---
   
   json::Value all(top);
   const json::Value &v = all["objarray"];
   
   for (json::ArrayConstIterator ait=v.abegin(); ait!=v.aend(); ++ait)
   {
      std::cout << "[" << (ait - v.abegin()) << "]" << std::endl;
      for (json::ObjectConstIterator oit=ait->obegin(); oit!=ait->oend(); ++oit)
      {
         std::cout << "  " << oit->first << ": " << oit->second << std::endl;
      }
   }
   return 0;
}

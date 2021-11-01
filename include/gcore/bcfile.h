/*
MIT License

Copyright (c) 2011 Gaetan Guidet

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

#ifndef __gcore_bcfile_h_
#define __gcore_bcfile_h_

#include <gcore/config.h>
#include <gcore/string.h>

namespace gcore
{
   GCORE_API void WriteInt32(std::ostream &os, long val);
   GCORE_API void WriteUint32(std::ostream &os, unsigned long val);
   GCORE_API void WriteInt16(std::ostream &os, short val);
   GCORE_API void WriteUint16(std::ostream &os, unsigned short val);
   GCORE_API void WriteFloat(std::ostream &os, float val);
   GCORE_API void WriteDouble(std::ostream &os, double val);
   GCORE_API void WriteString(std::ostream &os, const char *str);
   GCORE_API void WriteString(std::ostream &os, const String &str);

   GCORE_API bool ReadInt32(std::istream &is, long &val);
   GCORE_API bool ReadUint32(std::istream &is, unsigned long &val);
   GCORE_API bool ReadInt16(std::istream &is, short &val);
   GCORE_API bool ReadUint16(std::istream &is, unsigned short &val);
   GCORE_API bool ReadFloat(std::istream &is, float &val);
   GCORE_API bool ReadDouble(std::istream &is, double &val);
   GCORE_API bool ReadString(std::istream &is, char **str);
   GCORE_API bool ReadString(std::istream &is, String &str);
   
   class GCORE_API BCFileElement
   {
   public:
      
      virtual ~BCFileElement() {}
      
      // return size in bytes
      virtual size_t size() const = 0;
      
      virtual void writeHeader(std::ostream &os) const = 0;
      virtual void write(std::ostream &os) const = 0;
      
      virtual bool readHeader(std::istream &is) = 0;
      virtual bool read(std::istream &is) = 0;
   };
   
   class GCORE_API BCFile
   {
   protected:
      
      // At reading time, only the element offsets are read
      class ElementPlaceHolder : public BCFileElement
      {
      public:
         
         ElementPlaceHolder(unsigned long off, size_t sz=0);
         virtual ~ElementPlaceHolder();

         virtual size_t size() const;
         virtual void writeHeader(std::ostream &os) const;
         virtual void write(std::ostream &os) const;
         virtual bool readHeader(std::istream &is);
         virtual bool read(std::istream &is);

         unsigned long offset() const;
         
      protected:
         
         unsigned long mOffset;
         size_t mSize;
         char *mContent;
      };
   
   public:
    
      BCFile();
      ~BCFile();

      void clearElements();
      bool addElement(const String &name, BCFileElement *e);
      bool replaceElement(const String &name, BCFileElement *e);
      bool hasElement(const String &name) const;
      bool readElement(const String &name, BCFileElement *elt);

      bool write(const String &filepath, bool preserveData) const;
      // no read function, as we need the FileElement handlers to read properly
      // just bookkeep the offset if the different elements
      // -> that's the reason an internal file object is kept
      bool readTOC(const String &filepath);

      void doneReading();

   protected:

      // write file version 0.1
      void write_0_1(std::ofstream &ofile, size_t baseOff) const;
      
      // write file version 0.2
      void write_0_2(std::ofstream &ofile, size_t baseOff) const;      
      
      // read file version 0.1 TOC
      bool read_0_1(std::ifstream &ifile);
      
      // read file version 0.2 TOC
      bool read_0_2(std::ifstream &ifile);

   protected:

      mutable std::map<String, BCFileElement*> mElements;
      mutable std::ifstream mInFile;
      mutable std::vector<ElementPlaceHolder*> mPlaceHolders;
   };
   
}

#endif



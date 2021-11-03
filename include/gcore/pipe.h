/*
MIT License

Copyright (c) 2009 Gaetan Guidet

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

#ifndef __gcore_pipe_h_
#define __gcore_pipe_h_

#include <gcore/string.h>

namespace gcore {

#ifdef _WIN32
  
  typedef void* PipeID;
  const PipeID INVALID_PIPE = 0;
  
  inline bool IsValidPipeID(PipeID pi) {
    return (pi != 0);
  }
  
#else
  
  typedef int PipeID;
  const PipeID INVALID_PIPE = -1;
  
  inline bool IsValidPipeID(PipeID pi) {
    return (pi >= 0);
  }
  
#endif
  
  class GCORE_API Pipe {
    public:
      
      static PipeID StdInID();
      static PipeID StdOutID();
      static PipeID StdErrID();
    
    public:
    
      Pipe();
      Pipe(PipeID readId, PipeID writeId);
      Pipe(const Pipe &rhs);
      virtual ~Pipe();
      
      bool isNamed() const;
      const String& getName() const;
      bool isOwned() const;
      bool canRead() const;
      bool canWrite() const;
      bool open(const String &name);
      void close();
      bool create();
      bool create(const String &name);
      void closeRead();
      void closeWrite();
      int read(char *buffer, int size, bool retryOnInterrupt=false) const;
      int read(String &str, bool retryOnInterrupt=false) const;
      int write(const char *buffer, int size) const;
      int write(const String &str) const;
      PipeID readID() const;
      PipeID writeID() const;
      
      Pipe& operator=(const Pipe &rhs);
      
    private:

      PipeID mDesc[2];
      mutable bool mOwn; // Ownership transferred by copy constructor and assignment
      String mName;
#ifdef _WIN32
      mutable bool mConnected;
#endif
  };
}

#endif


/*

Copyright (C) 2009, 2010  Gaetan Guidet

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

#ifndef __gcore_memory_h_
#define __gcore_memory_h_

#include <gcore/list.h>

namespace gcore {

  namespace details {
    class GCORE_API ChunkAllocator {
      public:
        
        ChunkAllocator(size_t chunkSize, size_t blockSize=0);
        ChunkAllocator(const ChunkAllocator &rhs);
        ~ChunkAllocator();
        
        ChunkAllocator& operator=(const ChunkAllocator &rhs);
        
        void* allocate();
        void deallocate(void *ptr);
        
        inline size_t getBlockSize() const {
          return mBlockSize;
        }
        
        inline bool operator<(size_t sz) const {
          return (mBlockSize < sz);
        }
        
      private:
      
        struct Chunk {
          unsigned char *data;
          unsigned char firstFreeBlock;
          unsigned char numFreeBlocks;
          
          void initialize(size_t blockSize, unsigned char numBlocks);
          void* allocate(size_t blockSize);
          void deallocate(void *ptr, size_t blockSize);
          void release();
        };
        
        typedef List<Chunk> ChunkVector;
        typedef ChunkVector::iterator ChunkIterator;
        typedef ChunkVector::const_iterator ChunkConstIterator;
        
        size_t mBlockSize;
        unsigned char mNumBlocks;
        ChunkVector mChunks;
        Chunk *mLastAlloc;
        Chunk *mLastDealloc;
        mutable bool mOwns;
    };
  }
  
  class GCORE_API MemoryManager {
  
    public:
    
      static void Initialize(size_t chunkSize=8192, size_t smallSize=256);
      static MemoryManager& Instance();
      static MemoryManager* InstancePtr();
      static void DeInitialize();
      
      ~MemoryManager();
      
      void* allocate(size_t numBytes);
      void deallocate(void *ptr, size_t numBytes);
    
    private:
    
      static MemoryManager *msInstance;
    
      MemoryManager(size_t chunkSize, size_t smallSize);
    
    private:
      
      typedef List<details::ChunkAllocator> AllocatorVector;
      typedef AllocatorVector::iterator AllocatorIterator;
      typedef AllocatorVector::const_iterator AllocatorConstIterator;
      
      size_t mSmallSize;
      size_t mChunkSize;
      AllocatorVector mAllocators;
      details::ChunkAllocator *mLastAlloc;
      details::ChunkAllocator *mLastDealloc;
  };
  
  
  class GCORE_API MemoryManagedObject {
  
    public:
    
      static void* operator new(size_t sz);
      static void operator delete(void *ptr, size_t sz);
      
      MemoryManagedObject();
      virtual ~MemoryManagedObject();
  };

}


#endif


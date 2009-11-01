/*

Copyright (C) 2009  Gaetan Guidet

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

#include <gcore/memory.h>

using namespace std;

namespace gcore {

namespace details {

void ChunkAllocator::Chunk::initialize(size_t blockSize,
                                        unsigned char numBlocks) {
  // Check parameters validity
  assert(blockSize > 0);
  assert(numBlocks > 0);
  
  // Check for overflow
  assert(((numBlocks * blockSize) / blockSize) == numBlocks);
  
  // Allocate memory
  data = new unsigned char[numBlocks * blockSize];

#ifdef _DEBUG
  cout << numBlocks * blockSize << " bytes allocated @ 0x"
       << hex << reinterpret_cast<size_t>(data) << dec
       << " in " << blockSize << " bytes blocks" << endl;
#endif

  // Initialize memory
  firstFreeBlock = 0;
  numFreeBlocks = numBlocks;
  unsigned char *curBlock = data;
  unsigned char i = 0;
  for (; i != numBlocks; curBlock += blockSize) {
    *curBlock = ++i;
  }
}

void* ChunkAllocator::Chunk::allocate(size_t blockSize) {
  if (numFreeBlocks == 0) {
    return 0;
  }
  
  // did we get a valid blockSize
#ifdef _DEBUG
  cout << "Alloc " << blockSize << " bytes in chunk @ 0x"
       << hex << reinterpret_cast<size_t>(data) << dec << endl;
#endif
  assert(((firstFreeBlock * blockSize) / blockSize) == firstFreeBlock);
  
  unsigned char *block = (data + (firstFreeBlock * blockSize));
  firstFreeBlock = *block;
  --numFreeBlocks;
  return block;
}

void ChunkAllocator::Chunk::deallocate(void *ptr, size_t blockSize) {
  assert(ptr >= data);
  unsigned char *block = static_cast<unsigned char*>(ptr);
  
  // are we correctly aligned ? [did we get a valid blockSize]
  assert(((block - data) % blockSize) == 0);

#ifdef _DEBUG
  cout << "Free " << blockSize << " bytes in chunk @ 0x"
       << hex << reinterpret_cast<size_t>(data) << dec << endl;
#endif

  *block = firstFreeBlock;
  firstFreeBlock = static_cast<unsigned char>((block - data) / blockSize);

  // check for truncation
  assert(firstFreeBlock == ((block - data) / blockSize));
  ++numFreeBlocks;
}

void ChunkAllocator::Chunk::release() {

#ifdef _DEBUG
  cout << "Memory set free @ 0x"
       << hex << reinterpret_cast<size_t>(data) << dec << endl;
#endif

  delete[] data;
  data = 0;
  firstFreeBlock = 0;
  numFreeBlocks = 0;
}

// ---

// When doing temporary copy, we don't want to de-allocate memory
// Inserting a new ChunkAllocator in a vector might create a new array
// copy values, then delete the old one
// We only want the last ChunkAllocator to release the memory

ChunkAllocator::ChunkAllocator(size_t chunkSize, size_t blockSize)
  : mBlockSize(0), mNumBlocks(0), mLastAlloc(0), mLastDealloc(0), mOwns(true) {
  
  assert(blockSize > 0);
  mBlockSize = blockSize;
  size_t numBlocks = chunkSize / mBlockSize;
  
  // can have 256 blocks at most !
  if (numBlocks > 255) {
    numBlocks = 255;
  } else if (numBlocks == 0) {
    // blockSize is bigger than _CHUNK_SIZE
    numBlocks = 8 * blockSize;
  }
  mNumBlocks = static_cast<unsigned char>(numBlocks);
  
  // check for truncation
  assert(mNumBlocks == numBlocks);
}

ChunkAllocator::ChunkAllocator(const ChunkAllocator &rhs)
  : mBlockSize(rhs.mBlockSize), mNumBlocks(rhs.mNumBlocks), mChunks(rhs.mChunks),
    mLastAlloc(0), mLastDealloc(0), mOwns(true) {
  
  rhs.mOwns = false;
  
  // restore mLastAlloc/mLastDealloc pointers if needed
  if (rhs.mLastAlloc) {
    size_t idx = rhs.mLastAlloc - &(rhs.mChunks.front());
    mLastAlloc = &(mChunks.front()) + idx;
  }
  
  if (rhs.mLastDealloc) {
    size_t idx = rhs.mLastDealloc - &(rhs.mChunks.front());
    mLastDealloc = &(mChunks.front()) + idx;
  }
}

ChunkAllocator::~ChunkAllocator() {
  if (!mOwns) {
    return;
  }
  ChunkIterator it = mChunks.begin();
  while (it != mChunks.end()) {
    assert(it->numFreeBlocks == mNumBlocks);
    it->release();
    ++it;
  }
  mChunks.clear();
}

ChunkAllocator& ChunkAllocator::operator=(const ChunkAllocator &rhs) {
  ChunkAllocator tmp(rhs);
  std::swap(mBlockSize, tmp.mBlockSize);
  std::swap(mNumBlocks, tmp.mNumBlocks);
  std::swap(mLastAlloc, tmp.mLastAlloc);
  std::swap(mLastDealloc, tmp.mLastDealloc);
  std::swap(mOwns, tmp.mOwns);
  mChunks.swap(tmp.mChunks);
  return *this;
}

void* ChunkAllocator::allocate() {
  if (mLastAlloc == 0 || mLastAlloc->numFreeBlocks == 0) {
    ChunkIterator it = mChunks.begin();
    for (;; ++it) {
      if (it == mChunks.end()) {
        mChunks.reserve(mChunks.size() + 1);
        Chunk newChunk;
        newChunk.initialize(mBlockSize, mNumBlocks);
        mChunks.push_back(newChunk);
        mLastAlloc = &(mChunks.back());
        mLastDealloc = &(mChunks.front());
        break;
      }
      if (it->numFreeBlocks > 0) {
        mLastAlloc = &*it;
        break;
      }
    }
  }
  assert(mLastAlloc != 0);
  assert(mLastAlloc->numFreeBlocks > 0);
  return mLastAlloc->allocate(mBlockSize);
}

void ChunkAllocator::deallocate(void *ptr) {
  assert(!mChunks.empty());
  assert(mLastDealloc >= &(mChunks.front()));
  assert(mLastDealloc <= &(mChunks.back()));
  // figure out which chunk we have to deallocate from
  // starting with the last deallocation chunk, searching both up and down
  const size_t chunkLength = mNumBlocks * mBlockSize;
  Chunk *loBound = &(mChunks.front());
  Chunk *hiBound = &(mChunks.back()) + 1;
  Chunk *lo = mLastDealloc;
  Chunk *hi = mLastDealloc + 1;
  if (hi == hiBound) {
    hi = 0;
  }
  mLastDealloc = 0;
  for (;;) {
    if (lo) {
      if (ptr >= lo->data && ptr < lo->data + chunkLength) {
        mLastDealloc = lo;
        break;
      }
      if (lo == loBound) {
        lo = 0;
      } else {
        --lo;
      }
    }
    if (hi) {
      if (ptr >= hi->data && ptr < hi->data + chunkLength) {
        mLastDealloc = hi;
        break;
      }
      if (++hi == hiBound) {
        hi = 0;
      }
    }
  }
  assert(mLastDealloc);
  // Now deallocate
  mLastDealloc->deallocate(ptr, mBlockSize);
  if (mLastDealloc->numFreeBlocks == mNumBlocks) {
    // realease only if we can find two empty chunks
    // an empty chunk is always placed at the end of the chunk vector 
    Chunk &lastChunk = mChunks.back();
    if (&lastChunk == mLastDealloc) {
      // we deallocated from last chunk
      if (mChunks.size() > 1 && mLastDealloc[-1].numFreeBlocks == mNumBlocks) {
        lastChunk.release();
        mChunks.pop_back();
        mLastAlloc = mLastDealloc = &(mChunks.front());
      }
    } else {
      if (lastChunk.numFreeBlocks == mNumBlocks) {
        // last chunk is also empty, we can release it
        lastChunk.release();
        mChunks.pop_back();
        mLastAlloc = mLastDealloc;
      } else {
        // move this chunks to the end
        std::swap(*mLastDealloc, lastChunk);
        mLastAlloc = &(mChunks.back());
      }
    }
  }
}

} // namespace details

// ---

static inline size_t NextPowerOfTwo(size_t sz) {
  sz -= 1;
  sz = sz | (sz >> 1);
  sz = sz | (sz >> 2);
  sz = sz | (sz >> 4);
  sz = sz | (sz >> 8);
  sz = sz | (sz >> 16);
  return (sz + 1);
}

// ---

MemoryManager* MemoryManager::msInstance = 0;

void MemoryManager::Initialize(size_t chunkSize, size_t smallSize) {
  new MemoryManager(chunkSize, smallSize);
}

MemoryManager& MemoryManager::Instance() {
  assert(msInstance != 0);
  return *msInstance;
}

MemoryManager* MemoryManager::InstancePtr() {
  return msInstance;
}

void MemoryManager::DeInitialize() {
  if (msInstance) {
    delete msInstance;
  }
}

MemoryManager::MemoryManager(size_t chunkSize, size_t smallSize)
  : mSmallSize(smallSize), mChunkSize(chunkSize), mLastAlloc(0), mLastDealloc(0) {
  assert(msInstance == 0);
  msInstance = this;
}

MemoryManager::~MemoryManager() {
#ifdef _DEBUG
  cout << "Clear managed memory... " << endl;
#endif
  mAllocators.clear();
  msInstance = 0;
}
    
void* MemoryManager::allocate(size_t numBytes) {
  if (numBytes == 0) {
#ifdef _DEBUG
    cerr << "*** 0 bytes allocation detected !" << endl;
#endif
    return 0;
  }
  if (numBytes > mSmallSize) {
#ifdef _DEBUG
    cout << "### Too big for memory manager (" << numBytes << ")" << endl;
#endif
    return ::operator new(numBytes);
  } else {
    numBytes = NextPowerOfTwo(numBytes);
    if (mLastAlloc && mLastAlloc->getBlockSize() == numBytes) {
      return mLastAlloc->allocate();
    } else {
      AllocatorIterator it = std::lower_bound(mAllocators.begin(), mAllocators.end(), numBytes);
      if (it == mAllocators.end() || it->getBlockSize() != numBytes) {
        it = mAllocators.insert(it, details::ChunkAllocator(mChunkSize, numBytes));
        mLastDealloc = &(mAllocators.front());
      }
      mLastAlloc = &*it;
      return mLastAlloc->allocate();
    }
  }
}

void MemoryManager::deallocate(void *ptr, size_t numBytes) {
  if (numBytes == 0) {
#ifdef _DEBUG
    cerr << "*** 0 bytes deallocation detected !" << endl;
#endif
    return;
  }
  if (numBytes > mSmallSize) {
#ifdef _DEBUG
    cout << "### Free big memory chunk (" << numBytes << ")" << endl;
#endif
    ::operator delete(ptr);
  } else {
    numBytes = NextPowerOfTwo(numBytes);
    if (mLastDealloc && mLastDealloc->getBlockSize() == numBytes) {
      mLastDealloc->deallocate(ptr);
    } else {
      AllocatorIterator it = std::lower_bound(mAllocators.begin(), mAllocators.end(), numBytes);
      assert(it != mAllocators.end());
      assert(it->getBlockSize() == numBytes);
      mLastDealloc = &*it;
      mLastDealloc->deallocate(ptr);
    }
  }
}

// ---

void* MemoryManagedObject::operator new(size_t sz) {
  return MemoryManager::Instance().allocate(sz);
}

void MemoryManagedObject::operator delete(void *ptr, size_t sz) {
  MemoryManager::Instance().deallocate(ptr, sz);
}

MemoryManagedObject::MemoryManagedObject() {
}

MemoryManagedObject::~MemoryManagedObject() {
}

}


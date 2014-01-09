#include "anki/util/Memory.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#include "anki/util/Functions.h"
#include <limits>
#include <cstdlib>
#include <cstring>

namespace anki {

//==============================================================================
// Other                                                                       =
//==============================================================================

//==============================================================================
void* mallocAligned(PtrSize size, PtrSize alignmentBytes)
{
#if ANKI_POSIX 
#	if ANKI_OS != ANKI_OS_ANDROID
	void* out;
	int err = posix_memalign(
		&out, getAlignedRoundUp(alignmentBytes, sizeof(void*)), size);

	if(!err)
	{
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
		return out;
	}
	else
	{
		throw ANKI_EXCEPTION("mallocAligned() failed");
		return nullptr;
	}
#	else
	void* out = memalign(
		getAlignedRoundUp(alignmentBytes, sizeof(void*)), size);

	if(out)
	{
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
		return out;
	}
	else
	{
		throw ANKI_EXCEPTION("mallocAligned() failed");
		return nullptr;
	}
#	endif
#else
#	error "Unimplemented"
#endif
}

//==============================================================================
void freeAligned(void* ptr)
{
#if ANKI_POSIX
	::free(ptr);
#else
#	error "Unimplemented"
#endif
}

//==============================================================================
// StackMemoryPool                                                             =
//==============================================================================

//==============================================================================
struct MemoryBlockHeader
{
	U32 size;
};

//==============================================================================
StackMemoryPool::StackMemoryPool(PtrSize size, U32 alignmentBytes_)
	:	alignmentBytes(alignmentBytes_), 
		memsize(getAlignedRoundUp(alignmentBytes, size))
{
	ANKI_ASSERT(memsize > 0);
	memory = (U8*)mallocAligned(memsize, alignmentBytes);

	if(memory != nullptr)
	{
		// Align allocated memory
		top = memory;
	}
	else
	{
		throw ANKI_EXCEPTION("Failed to allocate memory");
	}
}

//==============================================================================
StackMemoryPool::~StackMemoryPool()
{
	if(memory != nullptr)
	{
		freeAligned(memory);
	}
}

//==============================================================================
StackMemoryPool& StackMemoryPool::operator=(StackMemoryPool&& other)
{
	if(memory != nullptr)
	{
		freeAligned(memory);
	}

	memory = other.memory;
	memsize = other.memsize;
	alignmentBytes = other.alignmentBytes;
	top.store(other.top.load());

	other.memory = nullptr;
	other.memsize = 0;
	other.top = nullptr;

	return *this;
}

//==============================================================================
PtrSize StackMemoryPool::getAllocatedSize() const
{
	return top.load() - memory;
}

//==============================================================================
void* StackMemoryPool::allocate(PtrSize size_) throw()
{
	// memory is nullptr if moved
	ANKI_ASSERT(memory != nullptr);

	PtrSize headerSize = 
		getAlignedRoundUp(alignmentBytes, sizeof(MemoryBlockHeader));
	PtrSize size = 
		getAlignedRoundUp(alignmentBytes, size_ + headerSize);

	ANKI_ASSERT(size < MAX_U32 && "Too big allocation");

	U8* out = top.fetch_add(size);

	if(out + size <= memory + memsize)
	{
		// Write the block header
		((MemoryBlockHeader*)out)->size = size;

		// Set the correct output
		out += headerSize;

		// Check alignment
		ANKI_ASSERT(isAligned(alignmentBytes, out));
	}
	else
	{
		// Error
		out = nullptr;
	}

	return out;
}

//==============================================================================
Bool StackMemoryPool::free(void* ptr) throw()
{
	// ptr shouldn't be null or not aligned. If not aligned it was not 
	// allocated by this class
	ANKI_ASSERT(ptr != nullptr && isAligned(alignmentBytes, ptr));

	// memory is nullptr if moved
	ANKI_ASSERT(memory != nullptr);

	// Correct the p
	PtrSize headerSize = 
		getAlignedRoundUp(alignmentBytes, sizeof(MemoryBlockHeader));
	U8* realptr = (U8*)ptr - headerSize;

	// realptr should be inside the pool's preallocated memory
	ANKI_ASSERT(realptr >= memory);

	// Get block size
	U32 size = ((MemoryBlockHeader*)realptr)->size;

	// Check if the size is ok
	ANKI_ASSERT(realptr + size <= memory + memsize);

	// Atomic stuff
	U8* expected = realptr + size;
	U8* desired = realptr;

	// if(top == expected) {
	//     top = desired;
	//     exchange = true;
	// } else {
	//     expected = top;
	//     exchange = false;
	// }
	Bool exchange = top.compare_exchange_strong(expected, desired);

	return exchange;
}

//==============================================================================
void StackMemoryPool::reset()
{
	// memory is nullptr if moved
	ANKI_ASSERT(memory != nullptr);

#if ANKI_DEBUG
	// Invalidate the memory
	memset(memory, 0xCC, memsize);
#endif

	top = memory;
}

//==============================================================================
// ChainMemoryPool                                                             =
//==============================================================================

//==============================================================================
ChainMemoryPool::ChainMemoryPool(
	NextChunkAllocationMethod allocMethod, 
	U32 allocMethodValue, 
	U32 alignmentBytes_)
	:	alignmentBytes(alignmentBytes_), 
		chAllocMethodValue(allocMethodValue),
		chAllocMethod(allocMethod)
{}

//==============================================================================
ChainMemoryPool::~ChainMemoryPool()
{
	// TODO
}

//==============================================================================
void* ChainMemoryPool::allocateFromChunk(Chunk* ch, PtrSize size) throw()
{
	ANKI_ASSERT(ch);
	void* mem = ch->pool.allocate(size);

	if(mem)
	{
		++ch->allocationsCount;
	}

	return mem;
}

//==============================================================================
ChainMemoryPool::Chunk* ChainMemoryPool::createNewChunk(PtrSize size) throw()
{
	//
	// Calculate preferred size
	//
	
	// Get the size of the next chunk
	PtrSize crntMaxSize;
	if(chAllocMethod == FIXED)
	{
		crntMaxSize = chAllocMethodValue;
	}
	else
	{
		// Get the size of the previous max chunk
		crntMaxSize = 0;
		for(Chunk* c : chunks)
		{
			if(c)
			{
				PtrSize poolSize = c->pool.getSize();
				crntMaxSize = std::max(crntMaxSize, poolSize);
			}
		}

		// Increase it
		if(chAllocMethod == MULTIPLY)
		{
			crntMaxSize *= chAllocMethodValue;
		}
		else
		{
			ANKI_ASSERT(chAllocMethod == ADD);
			crntMaxSize += chAllocMethodValue;
		}
	}

	// Fix the size
	size = std::max(crntMaxSize, size * 2);

	//
	// Create the chunk
	//
	Chunk* chunk = new Chunk(size, alignmentBytes);

	if(chunk)
	{
		chunks.push_back(chunk);
	}
	
	return chunk;
}

//==============================================================================
void* ChainMemoryPool::allocate(PtrSize size) throw()
{
	Chunk* ch;
	void* mem = nullptr;

	// Get chunk
	lock.lock();
	ch = crntChunk;
	lock.unlock();

	// Create new chunk if needed
	if(ch == nullptr || (mem = allocateFromChunk(ch, size)) == nullptr)
	{
		// Create new chunk
		lock.lock();

		crntChunk = createNewChunk(size);
		ch = crntChunk;

		lock.unlock();

		// Chunk creation failed
		if(ch == nullptr)
		{
			return mem;
		}
	}

	if(mem == nullptr)
	{
		ANKI_ASSERT(ch != nullptr);
		mem = allocateFromChunk(ch, size);
		ANKI_ASSERT(mem != nullptr && "The chunk should have space");
	}

	return mem;
}

} // end namespace anki

#include "anki/util/Memory.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#include "anki/util/Functions.h"
#include <limits>
#include <cstdlib>
#include <cstring>

namespace anki {

//==============================================================================
struct MemoryBlockHeader
{
	U32 size;
};

//==============================================================================
StackMemoryPool::StackMemoryPool(PtrSize size, U32 alignmentBytes_)
	: alignmentBytes(alignmentBytes_), memsize(calcAlignSize(size))
{
	ANKI_ASSERT(memsize > 0);
	memory = (U8*)::malloc(memsize);

	if(memory != nullptr)
	{
		top = (U8*)calcAlignSize((PtrSize)memory);
	}
	else
	{
		throw ANKI_EXCEPTION("malloc() failed");
	}
}

//==============================================================================
StackMemoryPool::~StackMemoryPool()
{
	if(memory != nullptr)
	{
		::free(memory);
	}
}

//==============================================================================
StackMemoryPool& StackMemoryPool::operator=(StackMemoryPool&& other)
{
	if(memory != nullptr)
	{
		::free(memory);
	}

	memory = other.memory;
	memsize = other.memsize;
	top.store(other.top.load());
	alignmentBytes = other.alignmentBytes;

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

	PtrSize memBlockSize = calcAlignSize(sizeof(MemoryBlockHeader));
	PtrSize size = 
		calcAlignSize(size_ + memBlockSize);

	ANKI_ASSERT(size < std::numeric_limits<U32>::max() && "Too big allocation");

	U8* out = top.fetch_add(size);

	if(out + size <= memory + memsize)
	{
		// Write the block header
		((MemoryBlockHeader*)out)->size = size;

		// Set the correct output
		out += memBlockSize;
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
	// memory is nullptr if moved
	ANKI_ASSERT(memory != nullptr);

	// Correct the p
	PtrSize memBlockSize = calcAlignSize(sizeof(MemoryBlockHeader));
	U8* realptr = (U8*)ptr - memBlockSize;

	// realptr should be inside the pool's preallocated memory
	ANKI_ASSERT(realptr >= memory && realptr < memory + memsize);

	// Get block size
	U32 size = ((MemoryBlockHeader*)realptr)->size;

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

	top = (U8*)calcAlignSize((PtrSize)memory);
}

//==============================================================================
PtrSize StackMemoryPool::calcAlignSize(PtrSize size) const
{
	return alignSizeRoundUp(alignmentBytes, size);
}

} // end namespace anki

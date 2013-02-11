#include "anki/util/Memory.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
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
StackMemoryPool::StackMemoryPool(PtrSize size_, U32 alignmentBits_)
	: memsize(size_), alignmentBits(alignmentBits_)
{
	ANKI_ASSERT(memsize > 0);
	memory = (U8*)::malloc(memsize);

	if(memory != nullptr)
	{
		top = memory;
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
	alignmentBits = other.alignmentBits;

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

	PtrSize size = calcAlignSize(size_ + sizeof(MemoryBlockHeader));

	ANKI_ASSERT(size < std::numeric_limits<U32>::max() && "Too big allocation");

	U8* out = top.fetch_add(size);

	if(out + size <= memory + memsize)
	{
		// Write the block header
		((MemoryBlockHeader*)out)->size = size;

		// Set the correct output
		out += sizeof(MemoryBlockHeader);
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
	U8* realptr = (U8*)ptr - sizeof(MemoryBlockHeader);

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

	top = memory;
}

//==============================================================================
PtrSize StackMemoryPool::calcAlignSize(PtrSize size) const
{
	return size + (size % (alignmentBits / 8));
}

} // end namespace anki

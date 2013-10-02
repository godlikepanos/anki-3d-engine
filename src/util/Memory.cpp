#include "anki/util/Memory.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#include <limits>
#include <cstdlib>
#include <cstring>

namespace anki {

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

	ANKI_ASSERT(size < std::numeric_limits<U32>::max() && "Too big allocation");

	U8* out = top.fetch_add(size);

	if(out + size <= memory + memsize)
	{
		// Write the block header
		((MemoryBlockHeader*)out)->size = size;

		// Set the correct output
		out += headerSize;
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
	ANKI_ASSERT(memory != nullptr && ptr != nullptr);

	// Correct the p
	PtrSize headerSize = 
		getAlignedRoundUp(alignmentBytes, sizeof(MemoryBlockHeader));
	U8* realptr = (U8*)ptr - headerSize;

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

	top = getAlignedRoundUp(alignmentBytes, memory);
}

//==============================================================================
// Other                                                                       =
//==============================================================================

//==============================================================================
void* mallocAligned(PtrSize size, PtrSize alignmentBytes)
{
#if ANKI_POSIX
	void* out;
	int err = posix_memalign(&out, alignmentBytes, size);

	if(!err)
	{
		// Make sure it's aligned
		ANKI_ASSERT(isAligned(alignmentBytes, out));
		return out;
	}
	else
	{
		return nullptr;
	}
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

} // end namespace anki

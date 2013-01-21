#include "anki/util/Memory.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#include <limits>
#include <cstdlib>

namespace anki {

//==============================================================================
struct StackedMemoryPoolBlock
{
	U32 size;
};

//==============================================================================
StackedMemoryPool::StackedMemoryPool(PtrSize size_, U32 alignmentBits_)
	: memsize(size_), alignmentBits(alignmentBits_)
{
	ANKI_ASSERT(memsize > 0);
	memory = (U8*)::malloc(memsize);

	if(memory != nullptr)
	{
		ptr = memory;
	}
	else
	{
		throw ANKI_EXCEPTION("malloc() failed");
	}
}

//==============================================================================
void* StackedMemoryPool::allocate(PtrSize size_) throw()
{
	PtrSize size = calcAlignSize(size_ + sizeof(StackedMemoryPoolBlock));

	ANKI_ASSERT(size < std::numeric_limits<U32>::max() && "Too big allocation");

	U8* out = ptr.fetch_add(size);

	if(out + size <= memory + memsize)
	{
		// Write the block
		((StackedMemoryPoolBlock*)out)->size = size;

		// Set the correct output
		out += sizeof(StackedMemoryPoolBlock);
	}
	else
	{
		// Error
		out = nullptr;
	}

	return out;
}

//==============================================================================
Bool StackedMemoryPool::free(void* p) throw()
{
	// Correct the p
	U8* realp = (U8*)p - sizeof(StackedMemoryPoolBlock);

	// Get block size
	U32 size = ((StackedMemoryPoolBlock*)realp)->size;

	// Atomic stuff
	U8* expected = realp + size;
	U8* desired = realp;

	// if(ptr == expected)
	//     ptr = desired
	// else
	//     expected = ptr
	Bool exchange = ptr.compare_exchange_strong(expected, desired);

	return exchange;
}

} // end namespace anki

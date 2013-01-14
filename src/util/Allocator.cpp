#include "anki/util/Allocator.h"
#include <iostream>

namespace anki {

//==============================================================================
// Allocator                                                                   =
//==============================================================================

namespace detail {

//==============================================================================
PtrSize AllocatorStatic::allocatedSize = 0;

//==============================================================================
void AllocatorStatic::dump()
{
#if ANKI_DEBUG_ALLOCATORS
	if(allocatedSize > 0)
	{
		std::cerr << "You have memory leak of " << allocatedSize 
			<< " bytes" << std::endl;
	}
#endif
}

//==============================================================================
void* AllocatorStatic::malloc(PtrSize size)
{
	void* out = std::malloc(size);

	if(out != nullptr)
	{
		// Zero the buffer
		memset(out, 0, size);
#if ANKI_DEBUG_ALLOCATORS
		allocatedSize += size;
#endif
	}
	else
	{
		throw ANKI_EXCEPTION("malloc() failed");
	}
	
	return out;
}

//==============================================================================
void AllocatorStatic::free(void* p, PtrSize size)
{
	std::free(p);

	if(p)
	{
#if ANKI_DEBUG_ALLOCATORS
		allocatedSize -= size;
#else
		(void)size; // Make static the analyzer happy
#endif
	}
}

} // end namespace detail

//==============================================================================
// StackAllocatorInternal                                                      =
//==============================================================================

namespace detail {

//==============================================================================
void StackAllocatorInternal::init(const PtrSize size)
{
	mpool = new detail::MemoryPool;

	if(mpool != nullptr)
	{
		mpool->memory = (U8*)::malloc(size);

		if(mpool->memory != nullptr)
		{
			mpool->size = size;
			mpool->ptr = mpool->memory;
			// Memory pool's refcounter is 1
#if ANKI_PRINT_ALLOCATOR_MESSAGES
		std::cout << "New MemoryPool created: Address: " << mpool
			<< ", size: " << size
			<< ", pool address: " << mpool->memory << std::endl;
#endif
			return;
		}
	}

	// Errors happened

	if(mpool->memory)
	{
		::free(mpool->memory);
	}

	if(mpool)
	{
		delete mpool;
	}

	std::cerr << "Stack allocator constuctor failed. I will cannot "
		"throw but I have to exit" << std::endl;
	exit(0);
}

//==============================================================================
void StackAllocatorInternal::deinit()
{
	int refCounter = mpool->refCounter.fetch_sub(1);

	if(mpool && (refCounter - 1) == 0)
	{
#if ANKI_PRINT_ALLOCATOR_MESSAGES
		std::cout << "Deleting MemoryPool " << mpool << std::endl;
#endif
		// It is safe to delete the memory pool
		ANKI_ASSERT(mpool->refCounter == 0);

		::free(mpool->memory);
		delete mpool;
	}
}

} // end namespace detail

} // end namespace anki

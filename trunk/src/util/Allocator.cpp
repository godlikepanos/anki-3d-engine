#include "anki/util/Allocator.h"
#include <iostream>

namespace anki {

#if ANKI_PRINT_ALLOCATOR_MESSAGES
#	define ANKI_ALLOCATOR_PRINT(x_) \
		std::cout << "(" << ANKI_FILE << ":" << __LINE__ << ") " << x_ \
		<< std::endl;
#else
#	define ANKI_ALLOCATOR_PRINT(x) ((void)0)
#endif

//==============================================================================
// AllocatorInternal                                                           =
//==============================================================================

namespace detail {

//==============================================================================
PtrSize AllocatorInternal::allocatedSize = 0;

//==============================================================================
void AllocatorInternal::dump()
{
#if ANKI_DEBUG_ALLOCATORS
	if(allocatedSize > 0)
	{
		ANKI_ALLOCATOR_PRINT("You have memory leak of " << allocatedSize 
			<< " bytes");
	}
#endif
}

//==============================================================================
void* AllocatorInternal::gmalloc(PtrSize size)
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
void AllocatorInternal::gfree(void* p, PtrSize size)
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
	ANKI_ASSERT(mpool == nullptr);
	mpool = new detail::MemoryPool;

	if(mpool != nullptr)
	{
		mpool->memory = (U8*)::malloc(size);

		if(mpool->memory != nullptr)
		{
			mpool->size = size;
			mpool->ptr = mpool->memory;
			// Memory pool's refcounter is 1

			ANKI_ALLOCATOR_PRINT("New MemoryPool created: Address: " << mpool
				<< ", size: " << size
				<< ", pool address: " << (void*)mpool->memory);

			return;
		}
		else
		{
			delete mpool;
		}
	}

	std::cerr << "Stack allocator constuctor failed. I will cannot "
		"throw but I have to exit" << std::endl;
	exit(0);
}

//==============================================================================
void StackAllocatorInternal::deinit()
{
	if (mpool)
	{
		I32 refCounterPrev = mpool->refCounter.fetch_sub(1);

		if(refCounterPrev == 1)
		{
			ANKI_ALLOCATOR_PRINT("Deleting MemoryPool " << mpool);

			::free(mpool->memory);
			delete mpool;
			mpool = nullptr;
		}
	}
}

//==============================================================================
void StackAllocatorInternal::copy(const StackAllocatorInternal& b)
{
	deinit();
	mpool = b.mpool;
	if(mpool)
	{
		// Retain the mpool
		++mpool->refCounter;
	}
}

//==============================================================================
Bool StackAllocatorInternal::dump()
{
	Bool ret = true;

	if(mpool)
	{
		auto diff = mpool->ptr.load() - mpool->memory;

		if(diff > 0)
		{
			ANKI_ALLOCATOR_PRINT("Lost bytes: " << diff);
			ret = false;
		}
	}

	return ret;
}

} // end namespace detail

} // end namespace anki

#include "anki/util/Allocator.h"
#include <iostream>
#include <cstring>

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

} // end namespace anki

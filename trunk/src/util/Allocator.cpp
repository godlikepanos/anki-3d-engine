#include "anki/util/Allocator.h"
#include <iostream>

namespace anki {

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
	void out = std::malloc(size);

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

} // end namespace anki

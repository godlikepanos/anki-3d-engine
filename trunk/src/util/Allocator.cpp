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
PtrSize HeapAllocatorInternal::allocatedSize = 0;

//==============================================================================
void HeapAllocatorInternal::dump()
{
#if ANKI_DEBUG_ALLOCATORS
	if(allocatedSize > 0)
	{
		ANKI_ALLOCATOR_PRINT("You have memory leak of " << allocatedSize 
			<< " bytes");
	}
#endif
}

} // end namespace detail

} // end namespace anki

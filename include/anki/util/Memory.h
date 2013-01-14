#ifndef ANKI_UTIL_MEMORY_H
#define ANKI_UTIL_MEMORY_H

#include "anki/util/Allocator.h"

namespace anki {

/// Function that imitates the new operator. The function allocates memory for
/// a number of elements and calls their constructor. The interesting thing is
/// that if the elements size is >1 then it allocates size bigger than the
/// required. The extra chunk is a number that will be used in
/// deleteObjectArray to identify the number of elements that were allocated
template<typename T, typename Alloc, typename... Args>
T* newObject(const Alloc& allocator_, const PtrSize n, Args&&... args)
{
	ANKI_ASSERT(n != 0);
	T* out;
	typename Alloc::template rebind<T>::other allocator(allocator_);

	// If the number of elements is then do a simple allocaton
	if(n == 1)
	{
		out = allocator.allocate(n);
	}
	else
	{
		// Allocate a memory block that includes the array size
		typedef typename Alloc::template rebind<U8>::other CharAlloc;
		CharAlloc charAlloc(allocator);
		U8* mem = charAlloc.allocate(sizeof(PtrSize) + n * sizeof(T));

		// Set the size of the block
		*(PtrSize*)mem = n;

		// Set the output address
		out = (T*)(mem + sizeof(PtrSize));
	}

	// Call the constuctors
	for(PtrSize i = 0; i < n; i++)
	{
		allocator.construct(&out[i], std::forward<Args>(args)...);
	}

	// Return result
	return out;
}

/// Function that imitates the delete operator
template<typename T, typename Alloc>
void deleteObject(const Alloc& allocator, T* p)
{
	ANKI_ASSERT(p);

	// Make sure the type is defined
	typedef U8 TypeMustBeComplete[sizeof(T) ? 1 : -1];
	(void) sizeof(TypeMustBeComplete);

	// Rebind allocator because the Alloc may be of another type
	typename Alloc::template rebind<T>::other alloc(allocator);

	// Call the destructor
	alloc.destroy(p);

	// Deallocate
	alloc.deallocate(p, 1);
}

/// Function that imitates the delete[] operator
template<typename T, typename Alloc>
void deleteObjectArray(const Alloc& allocator, T* p)
{
	ANKI_ASSERT(p);

	// Make sure the type is defined
	typedef U8 TypeMustBeComplete[sizeof(T) ? 1 : -1];
	(void) sizeof(TypeMustBeComplete);

	// Rebind allocator
	typename Alloc::template rebind<T>::other alloc(allocator);

	// Get the allocated block
	U8* block = (U8*)(p) - sizeof(PtrSize);

	// Get number of elements
	const PtrSize n = *(PtrSize*)block;

	// Call the destructors
	for(PtrSize i = 0; i < n; i++)
	{
		alloc.destroy(&p[i]);
	}

	// Deallocate the block
	typename Alloc::template rebind<U8>::other allocc(allocator);
	allocc.deallocate(block, n * sizeof(T) + sizeof(PtrSize));
}

} // end namespace anki

#endif

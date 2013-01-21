#ifndef ANKI_UTIL_MEMORY_H
#define ANKI_UTIL_MEMORY_H

#include "anki/util/StdTypes.h"
#include "anki/util/NonCopyable.h"
#include <atomic>

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup memory
/// @{

/// Thread safe memory pool
class StackedMemoryPool: public NonCopyable
{
public:
	/// Default constructor
	StackedMemoryPool(PtrSize size, U32 alignmentBits = 16);

	/// Move
	StackedMemoryPool(StackedMemoryPool&& other);

	~StackedMemoryPool();

	/// Allocate memory in StackedMemoryPool
	void* allocate(PtrSize size) throw();

	/// Free memory in StackedMemoryPool
	Bool free(void* ptr) throw();

private:
	/// Allocated memory
	U8* memory = nullptr;

	/// Size of the allocated memory
	PtrSize memsize = 0;

	/// Points to the memory and more specifically to the address of the next
	/// allocation
	std::atomic<U8*> ptr = {nullptr};

	/// Alignment
	U32 alignmentBits;

	/// Calculate tha aligned size
	PtrSize calcAlignSize(PtrSize size) const
	{
		return size + (size % (alignmentBits / 8));
	}
};

/// @}
/// @}

} // end namespace anki

#endif

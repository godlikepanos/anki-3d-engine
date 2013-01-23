#ifndef ANKI_UTIL_MEMORY_H
#define ANKI_UTIL_MEMORY_H

#include "anki/util/StdTypes.h"
#include "anki/util/NonCopyable.h"
#include <atomic>
#include <algorithm>

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup memory
/// @{

/// Thread safe memory pool
class StackMemoryPool: public NonCopyable
{
public:
	/// Default constructor
	StackMemoryPool(PtrSize size, U32 alignmentBits = 16);

	/// Move
	StackMemoryPool(StackMemoryPool&& other)
	{
		*this = std::move(other);
	}

	/// Destroy
	~StackMemoryPool();

	/// Move
	StackMemoryPool& operator=(StackMemoryPool&& other);

	/// Access the total size
	PtrSize getSize() const
	{
		return memsize;
	}

	/// Get the allocated size
	PtrSize getAllocatedSize() const;

	/// Allocate memory
	void* allocate(PtrSize size) throw();

	/// Free memory in StackMemoryPool. If the ptr is not the last allocation
	/// then nothing happens and the method returns false
	Bool free(void* ptr) throw();

	/// Reinit the pool. All existing allocated memory will be lost
	void reset();

private:
	/// Pre-allocated memory memory chunk
	U8* memory = nullptr;

	/// Size of the allocated memory chunk
	PtrSize memsize = 0;

	/// Points to the memory and more specifically to the top of the stack
	std::atomic<U8*> top = {nullptr};

	/// Alignment
	U32 alignmentBits;

	/// Calculate tha aligned size of an allocation
	PtrSize calcAlignSize(PtrSize size) const;
};

/// @}
/// @}

} // end namespace anki

#endif

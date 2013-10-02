#ifndef ANKI_UTIL_MEMORY_H
#define ANKI_UTIL_MEMORY_H

#include "anki/util/StdTypes.h"
#include "anki/util/Assert.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/Functions.h"
#include <atomic>
#include <algorithm>

namespace anki {

// Forward
template<typename T>
class Allocator;

/// @addtogroup util
/// @{
/// @addtogroup memory
/// @{

/// Thread safe memory pool
class StackMemoryPool: public NonCopyable
{
public:

	/// Safe alignment in bytes
	static const U SAFE_ALIGNMENT = 
#if ANKI_CPU_ARCH == ANKI_CPU_ARCH_INTEL
		16
#elif ANKI_CPU_ARCH == ANKI_CPU_ARCH_ARM
		16
#else
#	error "See file"
#endif
		;

	/// Default constructor
	StackMemoryPool(PtrSize size, U32 alignmentBytes = SAFE_ALIGNMENT);

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
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size) throw();

	/// Free memory in StackMemoryPool. If the ptr is not the last allocation
	/// then nothing happens and the method returns false
	///
	/// @param[in] ptr Memory block to deallocate
	/// @return True if the deallocation actually happened and false otherwise
	Bool free(void* ptr) throw();

	/// Reinit the pool. All existing allocated memory will be lost
	void reset();

private:
	/// Alignment of allocations
	U32 alignmentBytes;

	/// Pre-allocated memory chunk
	U8* memory = nullptr;

	/// Size of the pre-allocated memory chunk
	PtrSize memsize = 0;

	/// Points to the memory and more specifically to the top of the stack
	std::atomic<U8*> top = {nullptr};
};

/// Allocate aligned memory
extern void* mallocAligned(PtrSize size, PtrSize alignmentBytes);

/// Free aligned memory
extern void freeAligned(void* ptr);

/// @}
/// @}

} // end namespace anki

#endif

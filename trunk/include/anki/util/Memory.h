#ifndef ANKI_UTIL_MEMORY_H
#define ANKI_UTIL_MEMORY_H

#include "anki/util/StdTypes.h"
#include "anki/util/Assert.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/Thread.h"
#include <atomic>
#include <vector>
#include <memory>
#include <algorithm> // For the std::move

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup memory
/// @{

/// Allocate aligned memory
void* mallocAligned(PtrSize size, PtrSize alignmentBytes);

/// Free aligned memory
void freeAligned(void* ptr);

/// Thread safe memory pool. It's a preallocated memory pool that is used for 
/// memory allocations on top of that preallocated memory. It is mainly used by 
/// fast stack allocators
class StackMemoryPool: public NonCopyable
{
public:
	/// Default constructor
	StackMemoryPool(PtrSize size, U32 alignmentBytes = ANKI_SAFE_ALIGNMENT);

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

/// Chain memory pool. Almost similar to StackMemoryPool but more flexible and 
/// at the same time a bit slower.
class ChainMemoryPool: public NonCopyable
{
public:
	/// Chunk allocation method
	enum NextChunkAllocationMethod
	{
		FIXED,
		MULTIPLY,
		ADD
	};

	/// Default constructor
	ChainMemoryPool(
		NextChunkAllocationMethod allocMethod, 
		U32 allocMethodValue, 
		U32 alignmentBytes = ANKI_SAFE_ALIGNMENT);

	/// Destroy
	~ChainMemoryPool();

	/// Allocate memory. This operation is thread safe
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size) throw();

	/// Free memory. If the ptr is not the last allocation of the chunk
	/// then nothing happens and the method returns false
	///
	/// @param[in, out] ptr Memory block to deallocate
	/// @return True if the deallocation actually happened and false otherwise
	Bool free(void* ptr) throw();

private:
	/// A chunk of memory
	class Chunk
	{
	public:
		StackMemoryPool pool;
		std::atomic<U32> allocationsCount;

		Chunk(PtrSize size, U32 alignmentBytes)
			: pool(size, alignmentBytes), allocationsCount{0}
		{}
	};

	/// Alignment of allocations
	U32 alignmentBytes;

	/// A list of chunks
	std::vector<Chunk*> chunks;

	/// Current chunk to allocate from
	Chunk* crntChunk = nullptr;

	/// Fast thread locking
	SpinLock lock;

	/// Chunk allocation method value
	U32 chAllocMethodValue;

	/// Chunk allocation method
	U8 chAllocMethod;

	/// Allocate memory from a chunk
	void* allocateFromChunk(Chunk* ch, PtrSize size) throw();

	/// Create a new chunk
	Chunk* createNewChunk(PtrSize minSize) throw();
};

/// @}
/// @}

} // end namespace anki

#endif

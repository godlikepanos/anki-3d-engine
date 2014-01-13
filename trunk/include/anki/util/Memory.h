#ifndef ANKI_UTIL_MEMORY_H
#define ANKI_UTIL_MEMORY_H

#include "anki/util/StdTypes.h"
#include <memory>

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup memory
/// @{

/// Allocate aligned memory
void* mallocAligned(PtrSize size, PtrSize alignmentBytes) throw();

/// Free aligned memory
void freeAligned(void* ptr) throw();

/// A dummy interface to match the StackMemoryPool and ChainMemoryPool 
/// interfaces in order to be used by the same allocator template
class HeapMemoryPool
{
public:
	static void* allocate(PtrSize size, PtrSize alignment) throw()
	{
		return mallocAligned(size, alignment);
	}

	static Bool free(void* ptr) throw()
	{
		freeAligned(ptr);
		return true;
	}
};

/// Thread safe memory pool. It's a preallocated memory pool that is used for 
/// memory allocations on top of that preallocated memory. It is mainly used by 
/// fast stack allocators
class StackMemoryPool
{
	friend class ChainMemoryPool;

public:
	/// Default constructor
	StackMemoryPool()
	{}

	/// Copy constructor. It's not copying any data
	StackMemoryPool(const StackMemoryPool& other)
	{
		*this = other;
	}

	/// Constructor with parameters
	StackMemoryPool(PtrSize size, U32 alignmentBytes = ANKI_SAFE_ALIGNMENT);

	/// Destroy
	~StackMemoryPool();

	/// Copy. It will not copy any data, what it will do is add visibility of
	/// other's pool to this instance as well
	StackMemoryPool& operator=(const StackMemoryPool& other)
	{
		impl = other.impl;
		return *this;
	}

	/// Access the total size
	PtrSize getTotalSize() const;

	/// Get the allocated size
	PtrSize getAllocatedSize() const;

	/// Allocate memory
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes) throw();

	/// Free memory in StackMemoryPool. If the ptr is not the last allocation
	/// then nothing happens and the method returns false
	///
	/// @param[in, out] ptr Memory block to deallocate
	/// @return True if the deallocation actually happened and false otherwise
	Bool free(void* ptr) throw();

	/// Reinit the pool. All existing allocated memory will be lost
	void reset();

private:
	// Forward. Hide the implementation because Memory.h is the base of other
	// files and should not include them
	class Implementation;

	/// The actual implementation
	std::shared_ptr<Implementation> impl;
};

/// Chain memory pool. Almost similar to StackMemoryPool but more flexible and 
/// at the same time a bit slower.
class ChainMemoryPool
{
public:
	/// Chunk allocation method. Defines the size a newely created has chunk 
	/// compared to the last created
	enum ChunkAllocationStepMethod
	{
		FIXED, ///< All chunks have the same size
		MULTIPLY, ///< Next chuck's size will be old_chuck_size * a_value
		ADD ///< Next chuck's size will be old_chuck_size + a_value
	};

	/// Copy constructor. It's not copying any data
	ChainMemoryPool(const ChainMemoryPool& other)
	{
		*this = other;
	}

	/// Constructor with parameters
	/// @param initialChunkSize The size of the first chunk
	/// @param maxChunkSize The size of the chunks cannot exceed that number
	/// @param chunkAllocStepMethod How new chunks grow compared to the old ones
	/// @param chunkAllocStep Used along with chunkAllocStepMethod and defines
	///                       the ammount of chunk size increase 
	/// @param alignmentBytes The standard alignment of the allocations
	ChainMemoryPool(
		U32 initialChunkSize,
		U32 maxChunkSize,
		ChunkAllocationStepMethod chunkAllocStepMethod = MULTIPLY, 
		U32 chunkAllocStep = 2, 
		U32 alignmentBytes = ANKI_SAFE_ALIGNMENT);

	/// Destroy
	~ChainMemoryPool();

	/// Copy. It will not copy any data, what it will do is add visibility of
	/// other's pool to this instance as well
	ChainMemoryPool& operator=(const ChainMemoryPool& other)
	{
		impl = other.impl;
		return *this;
	}

	/// Allocate memory. This operation is thread safe
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes) throw();

	/// Free memory. If the ptr is not the last allocation of the chunk
	/// then nothing happens and the method returns false
	///
	/// @param[in, out] ptr Memory block to deallocate
	/// @return True if the deallocation actually happened and false otherwise
	Bool free(void* ptr) throw();

	/// @name Methods used for debugging
	/// @{
	PtrSize getChunksCount() const;
	/// @}

private:
	// Forward. Hide the implementation because Memory.h is the base of other
	// files and should not include them
	class Implementation;

	/// The actual implementation
	std::shared_ptr<Implementation> impl;
};

/// @}
/// @}

} // end namespace anki

#endif

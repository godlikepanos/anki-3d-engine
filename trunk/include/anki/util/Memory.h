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
	/// Allocate memory
	static void* allocate(PtrSize size, PtrSize alignment) throw()
	{
		return mallocAligned(size, alignment);
	}

	/// Free memory
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
	/// @param size The size of the pool
	/// @param alignmentBytes The maximum supported alignment for returned
	///                       memory
	StackMemoryPool(PtrSize size, PtrSize alignmentBytes = ANKI_SAFE_ALIGNMENT);

	/// Destroy
	~StackMemoryPool();

	/// Copy. It will not copy any data, what it will do is add visibility of
	/// other's pool to this instance as well
	StackMemoryPool& operator=(const StackMemoryPool& other)
	{
		impl = other.impl;
		return *this;
	}

	/// Allocate aligned memory. The operation is thread safe
	/// @param size The size to allocate
	/// @param alignmentBytes The alignment of the returned address
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes) throw();

	/// Free memory in StackMemoryPool. If the ptr is not the last allocation
	/// then nothing happens and the method returns false. The operation is
	/// threadsafe
	/// @param[in, out] ptr Memory block to deallocate
	/// @return True if the deallocation actually happened and false otherwise
	Bool free(void* ptr) throw();

	/// Reinit the pool. All existing allocated memory will be lost
	void reset();

	/// Get the total size
	PtrSize getTotalSize() const;

	/// Get the allocated size
	PtrSize getAllocatedSize() const;

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
	/// Chunk allocation method. Defines the size a newely created chunk should
	/// have compared to the last created. Used to grow chunks over the time of
	/// allocations
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
	/// @param alignmentBytes The maximum supported alignment for returned
	///                       memory
	ChainMemoryPool(
		PtrSize initialChunkSize,
		PtrSize maxChunkSize,
		ChunkAllocationStepMethod chunkAllocStepMethod = MULTIPLY, 
		PtrSize chunkAllocStep = 2, 
		PtrSize alignmentBytes = ANKI_SAFE_ALIGNMENT);

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
	/// @param size The size to allocate
	/// @param alignmentBytes The alignment of the returned address
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes) throw();

	/// Free memory. If the ptr is not the last allocation of the chunk
	/// then nothing happens and the method returns false
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

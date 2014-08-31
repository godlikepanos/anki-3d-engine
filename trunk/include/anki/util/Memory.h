// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_MEMORY_H
#define ANKI_UTIL_MEMORY_H

#include "anki/util/StdTypes.h"
#include <memory>

namespace anki {

/// @addtogroup util_memory
/// @{

/// Allocate aligned memory
void* mallocAligned(PtrSize size, PtrSize alignmentBytes) noexcept;

/// Free aligned memory
void freeAligned(void* ptr) noexcept;

/// The function signature of a memory allocation/deallocation. 
/// See allocAligned function for the explanation of arguments
using AllocAlignedCallback = void* (*)(void*, void*, PtrSize, PtrSize);

/// This is a function that allocates and deallocates heap memory. 
/// If the @a ptr is nullptr then it allocates using the @a size and 
/// @a alignment. If the @a ptr is not nullptr it deallocates the memory and
/// the @a size and @a alignment is ignored.
///
/// @param userData Used defined data
/// @param ptr The pointer to deallocate or nullptr
/// @param size The size to allocate or 0
/// @param alignment The allocation alignment or 0
/// @return On allocation mode it will return the newelly allocated block or
///         nullptr on error. On deallocation mode returns nullptr
void* allocAligned(
	void* userData, void* ptr, PtrSize size, PtrSize alignment) noexcept;

/// A dummy interface to match the StackMemoryPool and ChainMemoryPool 
/// interfaces in order to be used by the same allocator template
class HeapMemoryPool
{
public:
	/// Default constructor
	/// @note It does _nothing_ and it should stay that way. First of all,
	///       this class should have the same interface with the other pools
	///       and secondly, the default constructors of the allocators that use
	///       that pool will call that constructor and that happens a lot.
	///       If that constructor does some actual job then we have a problem.
	HeapMemoryPool()
	:	m_impl(nullptr)
	{}

	/// Copy constructor. It's not copying any data
	HeapMemoryPool(const HeapMemoryPool& other)
	:	m_impl(nullptr)
	{
		*this = other;
	}

	/// The real constructor
	/// @param allocCb The allocation function callback
	/// @param allocCbUserData The user data to pass to the allocation function
	HeapMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData);

	/// Destroy
	~HeapMemoryPool()
	{
		clear();
	}

	/// Copy. It will not copy any data, what it will do is add visibility of
	/// other's pool to this instance as well
	HeapMemoryPool& operator=(const HeapMemoryPool& other);

	/// Allocate memory
	void* allocate(PtrSize size, PtrSize alignment) noexcept;

	/// Free memory
	Bool free(void* ptr) noexcept;

	/// Return number of allocations
	U32 getAllocationsCount() const;

private:
	// Forward. Hide the implementation because Memory.h is the base of other
	// files and should not include them
	class Implementation;

	/// The actual implementation
	Implementation* m_impl;

	/// Clear the pool
	void clear();
};

/// Thread safe memory pool. It's a preallocated memory pool that is used for 
/// memory allocations on top of that preallocated memory. It is mainly used by 
/// fast stack allocators
class StackMemoryPool
{
	friend class ChainMemoryPool;

public:
	/// The type of the pool's snapshot
	using Snapshot = void*;

	/// Default constructor
	StackMemoryPool()
	:	m_impl(nullptr)
	{}

	/// Copy constructor. It's not copying any data
	StackMemoryPool(const StackMemoryPool& other)
	:	m_impl(nullptr)
	{
		*this = other;
	}

	/// Constructor with parameters
	/// @param alloc The allocation function callback
	/// @param allocUserData The user data to pass to the allocation function
	/// @param size The size of the pool
	/// @param alignmentBytes The maximum supported alignment for returned
	///                       memory
	StackMemoryPool(
		AllocAlignedCallback alloc, void* allocUserData,
		PtrSize size, PtrSize alignmentBytes = ANKI_SAFE_ALIGNMENT);

	/// Destroy
	~StackMemoryPool()
	{
		clear();
	}

	/// Copy. It will not copy any data, what it will do is add visibility of
	/// other's pool to this instance as well
	StackMemoryPool& operator=(const StackMemoryPool& other);

	/// Allocate aligned memory. The operation is thread safe
	/// @param size The size to allocate
	/// @param alignmentBytes The alignment of the returned address
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes) noexcept;

	/// Free memory in StackMemoryPool. If the ptr is not the last allocation
	/// then nothing happens and the method returns false. The operation is
	/// threadsafe
	/// @param[in, out] ptr Memory block to deallocate
	/// @return True if the deallocation actually happened and false otherwise
	Bool free(void* ptr) noexcept;

	/// Reinit the pool. All existing allocated memory will be lost
	void reset();

	/// Get the total size
	PtrSize getTotalSize() const;

	/// Get the allocated size
	PtrSize getAllocatedSize() const;

	/// Get the number of users for this pool
	U32 getUsersCount() const;

	/// Get a snapshot of the current state that can be used to reset the 
	/// pool's state later on. Not recommended on multithreading scenarios
	/// @return The current state of the pool
	Snapshot getShapshot() const;

	/// Reset the poll using a snapshot. Not recommended on multithreading 
	/// scenarios
	/// @param s The snapshot to be used
	void resetUsingSnapshot(Snapshot s);

private:
	// Forward. Hide the implementation because Memory.h is the base of other
	// files and should not include them
	class Implementation;

	/// The actual implementation
	Implementation* m_impl;

	/// Clear the pool
	void clear();
};

/// Chain memory pool. Almost similar to StackMemoryPool but more flexible and 
/// at the same time a bit slower.
class ChainMemoryPool
{
public:
	/// Chunk allocation method. Defines the size a newely created chunk should
	/// have compared to the last created. Used to grow chunks over the time of
	/// allocations
	enum class ChunkGrowMethod: U8
	{
		FIXED, ///< All chunks have the same size
		MULTIPLY, ///< Next chuck's size will be old_chuck_size * a_value
		ADD ///< Next chuck's size will be old_chuck_size + a_value
	};

	/// Default constructor
	ChainMemoryPool()
	:	m_impl(nullptr)
	{}

	/// Copy constructor. It's not copying any data
	ChainMemoryPool(const ChainMemoryPool& other)
	:	m_impl(nullptr)
	{
		*this = other;
	}

	/// Constructor with parameters
	/// @param alloc The allocation function callback
	/// @param allocUserData The user data to pass to the allocation function
	/// @param initialChunkSize The size of the first chunk
	/// @param maxChunkSize The size of the chunks cannot exceed that number
	/// @param chunkAllocStepMethod How new chunks grow compared to the old ones
	/// @param chunkAllocStep Used along with chunkAllocStepMethod and defines
	///                       the ammount of chunk size increase 
	/// @param alignmentBytes The maximum supported alignment for returned
	///                       memory
	ChainMemoryPool(
		AllocAlignedCallback alloc, void* allocUserData,
		PtrSize initialChunkSize,
		PtrSize maxChunkSize,
		ChunkGrowMethod chunkAllocStepMethod = 
			ChunkGrowMethod::MULTIPLY, 
		PtrSize chunkAllocStep = 2, 
		PtrSize alignmentBytes = ANKI_SAFE_ALIGNMENT);

	/// Destroy
	~ChainMemoryPool()
	{
		clear();
	}

	/// Copy. It will not copy any data, what it will do is add visibility of
	/// other's pool to this instance as well
	ChainMemoryPool& operator=(const ChainMemoryPool& other);

	/// Allocate memory. This operation is thread safe
	/// @param size The size to allocate
	/// @param alignmentBytes The alignment of the returned address
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes) noexcept;

	/// Free memory. If the ptr is not the last allocation of the chunk
	/// then nothing happens and the method returns false
	/// @param[in, out] ptr Memory block to deallocate
	/// @return True if the deallocation actually happened and false otherwise
	Bool free(void* ptr) noexcept;

	/// Get the number of users for this pool
	U32 getUsersCount() const;

	/// @name Methods used for optimizing future chains
	/// @{
	PtrSize getChunksCount() const;

	PtrSize getAllocatedSize() const;
	/// @}

private:
	// Forward. Hide the implementation because Memory.h is the base of other
	// files and should not include them
	class Implementation;

	/// The actual implementation
	Implementation* m_impl;

	/// Clear the pool
	void clear();
};

/// @}

} // end namespace anki

#endif

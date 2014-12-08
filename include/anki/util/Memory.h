// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_MEMORY_H
#define ANKI_UTIL_MEMORY_H

#include "anki/util/StdTypes.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/Atomic.h"

namespace anki {

// Forward
class SpinLock;

/// @addtogroup util_memory
/// @{

#define ANKI_MEM_USE_SIGNATURES ANKI_DEBUG

/// Allocate aligned memory
void* mallocAligned(PtrSize size, PtrSize alignmentBytes);

/// Free aligned memory
void freeAligned(void* ptr);

/// The function signature of a memory allocation/deallocation. 
/// See allocAligned function for the explanation of arguments
using AllocAlignedCallback = void* (*)(void*, void*, PtrSize, PtrSize);

/// An internal type.
using AllocationSignature = U32;

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
	void* userData, void* ptr, PtrSize size, PtrSize alignment);

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
	HeapMemoryPool();

	/// Destroy
	~HeapMemoryPool();

	/// The real constructor.
	/// @param allocCb The allocation function callback
	/// @param allocCbUserData The user data to pass to the allocation function
	Error create(AllocAlignedCallback allocCb, void* allocCbUserData);

	/// Allocate memory
	void* allocate(PtrSize size, PtrSize alignment);

	/// Free memory
	Bool free(void* ptr);

	/// Get refcount.
	AtomicU32& getRefcount()
	{
		return m_refcount;
	}

	/// Get the number of users for this pool.
	U32 getUsersCount() const
	{
		return m_refcount.load();
	}

	/// Return number of allocations
	U32 getAllocationsCount() const
	{
		return m_allocationsCount.load();
	}

	/// Get allocation callback.
	AllocAlignedCallback getAllocationCallback() const
	{
		return m_allocCb;
	}

	/// Get allocation callback user data.
	void* getAllocationCallbackUserData() const
	{
		return m_allocCbUserData;
	}

private:
	AtomicU32 m_refcount = {0};
	AllocAlignedCallback m_allocCb = nullptr;
	void* m_allocCbUserData = nullptr;
	AtomicU32 m_allocationsCount = {0};
#if ANKI_MEM_USE_SIGNATURES
	AllocationSignature m_signature = 0;
	static const U32 MAX_ALIGNMENT = 16;
	U32 m_headerSize = 0;
#endif
};

/// Thread safe memory pool. It's a preallocated memory pool that is used for 
/// memory allocations on top of that preallocated memory. It is mainly used by 
/// fast stack allocators
class StackMemoryPool: public NonCopyable
{
	friend class ChainMemoryPool;

public:
	/// The type of the pool's snapshot
	using Snapshot = void*;

	/// Default constructor
	StackMemoryPool();

	/// Destroy
	~StackMemoryPool();

	/// Create with parameters
	/// @param allocCb The allocation function callback
	/// @param allocCbUserData The user data to pass to the allocation function
	/// @param size The size of the pool
	/// @param alignmentBytes The maximum supported alignment for returned
	///                       memory
	Error create(
		AllocAlignedCallback allocCb, void* allocCbUserData,
		PtrSize size, PtrSize alignmentBytes = ANKI_SAFE_ALIGNMENT);

	/// Allocate aligned memory. The operation is thread safe
	/// @param size The size to allocate
	/// @param alignmentBytes The alignment of the returned address
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes);

	/// Free memory in StackMemoryPool. If the ptr is not the last allocation
	/// then nothing happens and the method returns false. The operation is
	/// threadsafe
	/// @param[in, out] ptr Memory block to deallocate
	/// @return True if the deallocation actually happened and false otherwise
	Bool free(void* ptr);

	/// Get the refcount.
	AtomicU32& getRefcount()
	{
		return m_refcount;
	}

	/// Get the number of users for this pool.
	U32 getUsersCount() const
	{
		return m_refcount.load();
	}

	/// Get allocation callback.
	AllocAlignedCallback getAllocationCallback() const
	{
		return m_allocCb;
	}

	/// Get allocation callback user data.
	void* getAllocationCallbackUserData() const
	{
		return m_allocCbUserData;
	}

	/// Return number of allocations
	U32 getAllocationsCount() const
	{
		return m_allocationsCount.load();
	}

	/// Get the total size.
	PtrSize getTotalSize() const
	{
		return m_memsize;
	}

	/// Get the allocated size.
	PtrSize getAllocatedSize() const
	{
		return m_top.load() - m_memory;
	}

	/// Get a snapshot of the current state that can be used to reset the 
	/// pool's state later on. Not recommended on multithreading scenarios
	/// @return The current state of the pool
	Snapshot getShapshot() const;

	/// Reset the poll using a snapshot. Not recommended on multithreading 
	/// scenarios
	/// @param s The snapshot to be used
	void resetUsingSnapshot(Snapshot s);

	/// Reinit the pool. All existing allocated memory will be lost
	void reset();

private:
	/// The header of each allocation
	struct MemoryBlockHeader
	{
		U8 m_size[sizeof(U32)]; ///< It's U8 to allow whatever alignment
	};

	static_assert(alignof(MemoryBlockHeader) == 1, "Alignment error");
	static_assert(sizeof(MemoryBlockHeader) == sizeof(U32), "Size error");

	/// Refcount
	AtomicU32 m_refcount = {0};

	/// User allocation function
	AllocAlignedCallback m_allocCb = nullptr;

	/// User allocation function data
	void* m_allocCbUserData = nullptr;

	/// Alignment of allocations
	PtrSize m_alignmentBytes = 0;

	/// Aligned size of MemoryBlockHeader
	PtrSize m_headerSize = 0;

	/// Pre-allocated memory chunk
	U8* m_memory = nullptr;

	/// Size of the pre-allocated memory chunk
	PtrSize m_memsize = 0;

	/// Points to the memory and more specifically to the top of the stack
	Atomic<U8*> m_top = {nullptr};

	AtomicU32 m_allocationsCount = {0};

	/// The ChainMemoryPool will set that to true.
	Bool8 m_ignoreAllocationErrors = false;
};

/// Chain memory pool. Almost similar to StackMemoryPool but more flexible and 
/// at the same time a bit slower.
class ChainMemoryPool: public NonCopyable
{
public:
	/// Chunk allocation method. Defines the size a newely created chunk should
	/// have compared to the last created. Used to grow chunks over the time of
	/// allocations
	enum class ChunkGrowMethod: U8
	{
		NONE,
		FIXED, ///< All chunks have the same size
		MULTIPLY, ///< Next chuck's size will be old_chuck_size * a_value
		ADD ///< Next chuck's size will be old_chuck_size + a_value
	};

	/// Default constructor
	ChainMemoryPool();

	/// Destroy
	~ChainMemoryPool();

	/// Constructor with parameters
	/// @param allocCb The allocation function callback
	/// @param allocCbUserData The user data to pass to the allocation function
	/// @param initialChunkSize The size of the first chunk
	/// @param maxChunkSize The size of the chunks cannot exceed that number
	/// @param chunkAllocStepMethod How new chunks grow compared to the old ones
	/// @param chunkAllocStep Used along with chunkAllocStepMethod and defines
	///                       the ammount of chunk size increase 
	/// @param alignmentBytes The maximum supported alignment for returned
	///                       memory
	Error create(
		AllocAlignedCallback allocCb, 
		void* allocCbUserData,
		PtrSize initialChunkSize,
		PtrSize maxChunkSize,
		ChunkGrowMethod chunkAllocStepMethod = ChunkGrowMethod::MULTIPLY, 
		PtrSize chunkAllocStep = 2, 
		PtrSize alignmentBytes = ANKI_SAFE_ALIGNMENT);

	/// Allocate memory. This operation is thread safe
	/// @param size The size to allocate
	/// @param alignmentBytes The alignment of the returned address
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes);

	/// Free memory. If the ptr is not the last allocation of the chunk
	/// then nothing happens and the method returns false
	/// @param[in, out] ptr Memory block to deallocate
	/// @return True if the deallocation actually happened and false otherwise
	Bool free(void* ptr);

	/// Get refcount.
	AtomicU32& getRefcount()
	{
		return m_refcount;
	}

	/// Get the number of users for this pool
	U32 getUsersCount() const
	{
		return m_refcount.load();
	}

	/// Get allocation callback.
	AllocAlignedCallback getAllocationCallback() const
	{
		return m_allocCb;
	}

	/// Get allocation callback user data.
	void* getAllocationCallbackUserData() const
	{
		return m_allocCbUserData;
	}

	/// Return number of allocations
	U32 getAllocationsCount() const
	{
		return m_allocationsCount;
	}

	/// @name Methods used for optimizing future chains.
	/// @{
	PtrSize getChunksCount() const;

	PtrSize getAllocatedSize() const;
	/// @}

private:
	/// A chunk of memory
	struct Chunk
	{
		StackMemoryPool m_pool;

		/// Used to identify if the chunk can be deleted
		U32 m_allocationsCount = 0;

		/// Next chunk in the list
		Chunk* m_next = nullptr;
	};

	/// Refcount
	AtomicU32 m_refcount = {0};

	/// User allocation function
	AllocAlignedCallback m_allocCb = nullptr;

	/// User allocation function data
	void* m_allocCbUserData = nullptr;

	/// Alignment of allocations
	PtrSize m_alignmentBytes = 0;

	/// The first chunk
	Chunk* m_headChunk = nullptr;

	/// Current chunk to allocate from
	Chunk* m_tailChunk = nullptr;

	/// Fast thread locking
	SpinLock* m_lock = nullptr;

	/// Chunk first chunk size
	PtrSize m_initSize = 0;

	/// Chunk max size
	PtrSize m_maxSize = 0;

	/// Chunk allocation method value
	U32 m_step = 0;

	/// Chunk allocation method
	ChunkGrowMethod m_method = ChunkGrowMethod::NONE;

	/// Allocations number.
	U32 m_allocationsCount = 0;

	/// Create a new chunk
	Chunk* createNewChunk(PtrSize size);

	/// Allocate from chunk
	void* allocateFromChunk(Chunk* ch, PtrSize size, PtrSize alignment);
};

/// @}

} // end namespace anki

#endif

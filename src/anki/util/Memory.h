// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/NonCopyable.h>
#include <anki/util/Atomic.h>
#include <anki/util/Assert.h>
#include <anki/util/Array.h>
#include <anki/util/Thread.h>
#include <utility> // For forward

namespace anki
{

// Forward
class SpinLock;

/// @addtogroup util_memory
/// @{

#define ANKI_MEM_USE_SIGNATURES ANKI_EXTRA_CHECKS

/// Allocate aligned memory
void* mallocAligned(PtrSize size, PtrSize alignmentBytes);

/// Free aligned memory
void freeAligned(void* ptr);

/// The function signature of a memory allocation/deallocation. See allocAligned function for the explanation of
/// arguments
using AllocAlignedCallback = void* (*)(void* userData, void* ptr, PtrSize size, PtrSize alignment);

/// An internal type.
using AllocationSignature = U32;

/// This is a function that allocates and deallocates heap memory. If the @a ptr is nullptr then it allocates using the
/// @a size and @a alignment. If the @a ptr is not nullptr it deallocates the memory and the @a size and @a alignment is
/// ignored.
///
/// @param userData Used defined data
/// @param ptr The pointer to deallocate or nullptr
/// @param size The size to allocate or 0
/// @param alignment The allocation alignment or 0
/// @return On allocation mode it will return the newelly allocated block or nullptr on error. On deallocation mode
///         returns nullptr
void* allocAligned(void* userData, void* ptr, PtrSize size, PtrSize alignment);

/// Generic memory pool. The base of HeapMemoryPool or StackMemoryPool or ChainMemoryPool.
class BaseMemoryPool : public NonCopyable
{
public:
	/// Pool type.
	enum class Type : U8
	{
		NONE,
		HEAP,
		STACK,
		CHAIN
	};

	BaseMemoryPool(Type type)
		: m_type(type)
	{
	}

	virtual ~BaseMemoryPool();

	/// Allocate memory. This operation MAY be thread safe
	/// @param size The size to allocate
	/// @param alignmentBytes The alignment of the returned address
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes);

	/// Free memory.
	/// @param[in, out] ptr Memory block to deallocate
	void free(void* ptr);

	/// Get refcount.
	Atomic<U32>& getRefcount()
	{
		return m_refcount;
	}

	/// Get number of users.
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

protected:
	/// User allocation function.
	AllocAlignedCallback m_allocCb = nullptr;

	/// User allocation function data.
	void* m_allocCbUserData = nullptr;

	/// Allocations count.
	Atomic<U32> m_allocationsCount = {0};

	/// Check if already created.
	Bool isCreated() const;

private:
	/// Type.
	Type m_type = Type::NONE;

	/// Refcount.
	Atomic<U32> m_refcount = {0};
};

/// A dummy interface to match the StackMemoryPool and ChainMemoryPool interfaces in order to be used by the same
/// allocator template.
class HeapMemoryPool : public BaseMemoryPool
{
public:
	/// Default constructor.
	HeapMemoryPool();

	/// Destroy
	~HeapMemoryPool() final;

	/// The real constructor.
	/// @param allocCb The allocation function callback
	/// @param allocCbUserData The user data to pass to the allocation function
	void create(AllocAlignedCallback allocCb, void* allocCbUserData);

	/// Allocate memory
	void* allocate(PtrSize size, PtrSize alignment);

	/// Free memory.
	/// @param[in, out] ptr Memory block to deallocate.
	void free(void* ptr);

private:
#if ANKI_MEM_USE_SIGNATURES
	AllocationSignature m_signature = 0;
	static const U32 MAX_ALIGNMENT = 64;
	U32 m_headerSize = 0;
#endif
};

/// Thread safe memory pool. It's a preallocated memory pool that is used for memory allocations on top of that
/// preallocated memory. It is mainly used by fast stack allocators
class StackMemoryPool : public BaseMemoryPool
{
public:
	/// The type of the pool's snapshot
	using Snapshot = void*;

	/// Default constructor
	StackMemoryPool();

	/// Destroy
	~StackMemoryPool() final;

	/// Create with parameters
	/// @param allocCb The allocation function callback
	/// @param allocCbUserData The user data to pass to the allocation function
	/// @param initialChunkSize The size of the first chunk.
	/// @param nextChunkScale Value that controls the next chunk.
	/// @param nextChunkBias Value that controls the next chunk.
	/// @param ignoreDeallocationErrors Method free() may fail if the ptr is not in the top of the stack. Set that to
	///        true to suppress such errors
	/// @param alignmentBytes The maximum supported alignment for returned memory
	void create(AllocAlignedCallback allocCb,
		void* allocCbUserData,
		PtrSize initialChunkSize,
		F32 nextChunkScale = 2.0,
		PtrSize nextChunkBias = 0,
		Bool ignoreDeallocationErrors = true,
		PtrSize alignmentBytes = ANKI_SAFE_ALIGNMENT);

	/// Allocate aligned memory. The operation is thread safe
	/// @param size The size to allocate
	/// @param alignmentBytes The alignment of the returned address
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes);

	/// Free memory in StackMemoryPool. It will not actially free anything.
	/// @param[in, out] ptr Memory block to deallocate
	void free(void* ptr);

	/// Reinit the pool. All existing allocated memory will be lost
	void reset();

	/// Get the current capacity of the pool. It's not thread safe.
	PtrSize getMemoryCapacity() const;

private:
	/// The memory chunk.
	class Chunk
	{
	public:
		/// The base memory of the chunk.
		U8* m_baseMem = nullptr;

		/// The moving ptr for the next allocation.
		Atomic<U8*> m_mem = {nullptr};

		/// The chunk size.
		PtrSize m_size = 0;

		/// Check that it's initialized.
		void check() const
		{
			ANKI_ASSERT(m_baseMem != nullptr);
			ANKI_ASSERT(m_mem.load() >= m_baseMem);
			ANKI_ASSERT(m_size > 0);
		}

		// Check that it's in reset state.
		void checkReset() const
		{
			ANKI_ASSERT(m_baseMem != nullptr);
			ANKI_ASSERT(m_mem.load() == m_baseMem);
			ANKI_ASSERT(m_size > 0);
		}
	};

	/// Alignment of allocations
	PtrSize m_alignmentBytes = 0;

	/// The size of the first chunk.
	PtrSize m_initialChunkSize = 0;

	/// Chunk scale.
	F32 m_nextChunkScale = 0.0;

	/// Chunk bias.
	PtrSize m_nextChunkBias = 0;

	/// Ignore deallocation errors.
	Bool8 m_ignoreDeallocationErrors = false;

	/// The current chunk. Chose the more strict memory order to avoid compiler
	/// re-ordering of instructions
	Atomic<U32, AtomicMemoryOrder::SEQ_CST> m_crntChunkIdx = {0};

	/// The max number of chunks.
	static const U MAX_CHUNKS = 256;

	/// The chunks.
	Array<Chunk, MAX_CHUNKS> m_chunks;

	/// Protect the m_crntChunkIdx.
	Mutex m_lock;
};

/// Chain memory pool. Almost similar to StackMemoryPool but more flexible and at the same time a bit slower.
class ChainMemoryPool : public BaseMemoryPool
{
public:
	/// Default constructor
	ChainMemoryPool();

	/// Destroy
	~ChainMemoryPool() final;

	/// Creates the pool.
	/// @param allocCb The allocation function callback.
	/// @param allocCbUserData The user data to pass to the allocation function.
	/// @param initialChunkSize The size of the first chunk.
	/// @param nextChunkScale Value that controls the next chunk.
	/// @param nextChunkBias Value that controls the next chunk.
	/// @param alignmentBytes The maximum supported alignment for returned memory.
	void create(AllocAlignedCallback allocCb,
		void* allocCbUserData,
		PtrSize initialChunkSize,
		F32 nextChunkScale = 2.0,
		PtrSize nextChunkBias = 0,
		PtrSize alignmentBytes = ANKI_SAFE_ALIGNMENT);

	/// Allocate memory. This operation is thread safe
	/// @param size The size to allocate
	/// @param alignmentBytes The alignment of the returned address
	/// @return The allocated memory or nullptr on failure
	void* allocate(PtrSize size, PtrSize alignmentBytes);

	/// Free memory. If the ptr is not the last allocation of the chunk then nothing happens and the method returns
	/// false.
	/// @param[in, out] ptr Memory block to deallocate
	void free(void* ptr);

	/// @name Methods used for optimizing future chains.
	/// @{
	PtrSize getChunksCount() const;

	PtrSize getAllocatedSize() const;
	/// @}

private:
	/// A chunk of memory
	struct Chunk
	{
		/// Pre-allocated memory chunk.
		U8* m_memory = nullptr;

		/// Size of the pre-allocated memory chunk
		PtrSize m_memsize = 0;

		/// Points to the memory and more specifically to the top of the stack
		U8* m_top = nullptr;

		/// Used to identify if the chunk can be deleted
		PtrSize m_allocationsCount = 0;

		/// Previous chunk in the list
		Chunk* m_prev = nullptr;

		/// Next chunk in the list
		Chunk* m_next = nullptr;
	};

	/// Alignment of allocations.
	PtrSize m_alignmentBytes = 0;

	/// The first chunk.
	Chunk* m_headChunk = nullptr;

	/// Current chunk to allocate from.
	Chunk* m_tailChunk = nullptr;

	/// Fast thread locking.
	SpinLock* m_lock = nullptr;

	/// Size of the first chunk.
	PtrSize m_initSize = 0;

	/// Chunk scale.
	F32 m_scale = 2.0;

	/// Chunk bias.
	PtrSize m_bias = 0;

	/// Cache a value.
	PtrSize m_headerSize = 0;

	/// Compute the size for the next chunk.
	/// @param size The current allocation size.
	PtrSize computeNewChunkSize(PtrSize size) const;

	/// Create a new chunk.
	Chunk* createNewChunk(PtrSize size);

	/// Allocate from chunk.
	void* allocateFromChunk(Chunk* ch, PtrSize size, PtrSize alignment);

	/// Destroy a chunk.
	void destroyChunk(Chunk* ch);
};
/// @}

} // end namespace anki

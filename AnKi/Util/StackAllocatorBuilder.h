// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Atomic.h>
#include <AnKi/Util/Common.h>

namespace anki {

/// @addtogroup util_memory
/// @{

/// This is a convenience class used to build stack memory allocators.
/// @tparam TChunk This is the type of the internally allocated chunks. This should be having the following members:
///                @code
///                TChunk* m_nextChunk;
///                Atomic<PtrSize> m_offsetInChunk;
///                PtrSize m_chunkSize;
///                @endcode
/// @tparam TInterface This is the type of the interface that contains various info. Should have the following members:
///                    @code
///                    U32 getMaxAlignment();
///                    PtrSize getInitialChunkSize();
///                    F64 getNextChunkGrowScale();
///                    PtrSize getNextChunkGrowBias();
///                    Bool ignoreDeallocationErrors();
///                    Error allocateChunk(PtrSize size, TChunk*& out);
///                    void freeChunk(TChunk* out);
///                    void recycleChunk(TChunk& out);
///                    Atomic<U32>* getAllocationCount(); // It's optional
///                    @endcode
/// @tparam TLock This an optional lock. Can be a Mutex or SpinLock or some dummy class.
template<typename TChunk, typename TInterface, typename TLock>
class StackAllocatorBuilder
{
public:
	/// Create.
	StackAllocatorBuilder() = default;

	/// Destroy.
	~StackAllocatorBuilder()
	{
		destroy();
	}

	/// Manual destroy. The destructor calls that as well.
	/// @note It's not thread safe.
	void destroy();

	/// Allocate memory.
	/// @param size The size to allocate.
	/// @param alignment The alignment of the returned address.
	/// @param[out] chunk The chunk that the memory belongs to.
	/// @param[out] offset The offset inside the chunk.
	/// @note This is thread safe with itself.
	Error allocate(PtrSize size, PtrSize alignment, TChunk*& chunk, PtrSize& offset);

	/// Free memory. Doesn't do something special, only some bookkeeping.
	void free();

	/// Reset all the memory chunks.
	/// @note Not thread safe. Don't call it while calling allocate.
	void reset();

	/// Access the interface.
	/// @note Not thread safe. Don't call it while calling allocate.
	TInterface& getInterface()
	{
		return m_interface;
	}

	/// Access the interface.
	/// @note Not thread safe. Don't call it while calling allocate.
	const TInterface& getInterface() const
	{
		return m_interface;
	}

	/// Get the total memory comsumed by the chunks.
	/// @note Not thread safe. Don't call it while calling allocate.
	PtrSize getMemoryCapacity() const
	{
		return m_memoryCapacity;
	}

private:
	/// The current chunk. Chose the more strict memory order to avoid compiler re-ordering of instructions
	Atomic<TChunk*, AtomicMemoryOrder::kSeqCst> m_crntChunk = {nullptr};

	/// The beginning of the chunk list.
	TChunk* m_chunksListHead = nullptr;

	/// The memory allocated by all chunks.
	PtrSize m_memoryCapacity = 0;

	/// Number of chunks allocated.
	U32 m_chunkCount = 0;

	/// The interface as decribed in the class docs.
	TInterface m_interface;

	/// An optional lock.
	TLock m_lock;
};
/// @}

} // end namespace anki

#include <AnKi/Util/StackAllocatorBuilder.inl.h>

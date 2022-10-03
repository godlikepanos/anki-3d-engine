// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/DynamicArray.h>

namespace anki {

/// @addtogroup util_memory
/// @{

/// @memberof BuddyAllocatorBuilder
class BuddyAllocatorBuilderStats
{
public:
	PtrSize m_userAllocatedSize;
	PtrSize m_realAllocatedSize;
	F32 m_externalFragmentation;
	F32 m_internalFragmentation;
};

/// This is a generic implementation of a buddy allocator.
/// @tparam kMaxMemoryRangeLog2 The max memory to allocate.
/// @tparam TLock This an optional lock. Can be a Mutex or SpinLock or some dummy class.
/// @tparam TMemPool The type of the pool to be used in internal CPU allocations.
template<U32 kMaxMemoryRangeLog2, typename TLock, typename TMemPool = MemoryPoolPtrWrapper<HeapMemoryPool>>
class BuddyAllocatorBuilder
{
public:
	/// The type of the address.
	using Address = std::conditional_t<(kMaxMemoryRangeLog2 > 32), PtrSize, U32>;

	using InternalMemoryPool = TMemPool;

	BuddyAllocatorBuilder()
	{
	}

	/// @copydoc init
	BuddyAllocatorBuilder(InternalMemoryPool pool, U32 maxMemoryRangeLog2)
	{
		init(pool, maxMemoryRangeLog2);
	}

	BuddyAllocatorBuilder(const BuddyAllocatorBuilder&) = delete; // Non-copyable

	~BuddyAllocatorBuilder()
	{
		destroy();
	}

	BuddyAllocatorBuilder& operator=(const BuddyAllocatorBuilder&) = delete; // Non-copyable

	/// Init the allocator.
	/// @param pool The memory pool to be used for internal structures of the BuddyAllocatorBuilder.
	/// @param maxMemoryRangeLog2 The max memory to allocate.
	void init(InternalMemoryPool pool, U32 maxMemoryRangeLog2);

	/// Destroy the allocator.
	void destroy();

	/// Allocate memory.
	/// @param size The size of the allocation.
	/// @param alignment The returned address should have this alignment.
	/// @param[out] address The returned address if the allocation didn't fail. It will stay untouched if it failed.
	/// @return True if the allocation succeeded.
	[[nodiscard]] Bool allocate(PtrSize size, PtrSize alignment, Address& address);

	/// Free memory.
	/// @param address The address to free.
	/// @param alignment The alignment of the original allocation.
	/// @param size The size of the allocation.
	void free(Address address, PtrSize size, PtrSize alignment);

	/// Print a debug representation of the internal structures.
	void debugPrint() const;

	/// Get some info.
	void getStats(BuddyAllocatorBuilderStats& stats) const;

private:
	template<typename T>
	static constexpr T pow2(T exp)
	{
		return T(1) << exp;
	}

	static constexpr U32 log2(PtrSize v)
	{
		ANKI_ASSERT(isPowerOfTwo(v));
		return U32(__builtin_ctzll(v));
	}

	using FreeList = DynamicArray<Address, PtrSize>;
	InternalMemoryPool m_pool;
	DynamicArray<FreeList> m_freeLists;
	PtrSize m_maxMemoryRange = 0;
	PtrSize m_userAllocatedSize = 0; ///< The total ammount of memory requested by the user.
	PtrSize m_realAllocatedSize = 0; ///< The total ammount of memory actually allocated.
	mutable TLock m_mutex;

	U32 orderCount() const
	{
		return m_freeLists.getSize();
	}

	PtrSize popFree(U32 order)
	{
		ANKI_ASSERT(m_freeLists[order].getSize() > 0);
		const PtrSize address = m_freeLists[order].getBack();
		m_freeLists[order].popBack(m_pool);
		ANKI_ASSERT(address < m_maxMemoryRange);
		return address;
	}

	void freeInternal(PtrSize address, PtrSize size);
};
/// @}

} // end namespace anki

#include <AnKi/Util/BuddyAllocatorBuilder.inl.h>

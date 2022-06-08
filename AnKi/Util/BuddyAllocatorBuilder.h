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
/// @tparam T_MAX_MEMORY_RANGE_LOG2 The max memory to allocate.
/// @tparam TLock This an optional lock. Can be a Mutex or SpinLock or some dummy class.
template<U32 T_MAX_MEMORY_RANGE_LOG2, typename TLock>
class BuddyAllocatorBuilder
{
public:
	/// The type of the address.
	using Address = std::conditional_t<(T_MAX_MEMORY_RANGE_LOG2 > 32), PtrSize, U32>;

	BuddyAllocatorBuilder()
	{
	}

	/// @copydoc init
	BuddyAllocatorBuilder(GenericMemoryPoolAllocator<U8> alloc, U32 maxMemoryRangeLog2)
	{
		init(alloc, maxMemoryRangeLog2);
	}

	BuddyAllocatorBuilder(const BuddyAllocatorBuilder&) = delete; // Non-copyable

	~BuddyAllocatorBuilder()
	{
		destroy();
	}

	BuddyAllocatorBuilder& operator=(const BuddyAllocatorBuilder&) = delete; // Non-copyable

	/// Init the allocator.
	/// @param alloc The allocator used for internal structures of the BuddyAllocatorBuilder.
	/// @param maxMemoryRangeLog2 The max memory to allocate.
	void init(GenericMemoryPoolAllocator<U8> alloc, U32 maxMemoryRangeLog2);

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
	GenericMemoryPoolAllocator<U8> m_alloc;
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
		m_freeLists[order].popBack(m_alloc);
		ANKI_ASSERT(address < m_maxMemoryRange);
		return address;
	}

	void freeInternal(PtrSize address, PtrSize size);
};
/// @}

} // end namespace anki

#include <AnKi/Util/BuddyAllocatorBuilder.inl.h>

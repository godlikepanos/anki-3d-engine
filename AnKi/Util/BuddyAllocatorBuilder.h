// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/DynamicArray.h>

namespace anki {

/// @addtogroup util_memory
/// @{

/// This is a generic implementation of a buddy allocator.
/// @tparam T_MAX_MEMORY_RANGE_LOG2 The max memory to allocate.
template<U32 T_MAX_MEMORY_RANGE_LOG2 = 32>
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
	/// @param[out] address The returned address if the allocation didn't fail.
	/// @return True if the allocation succeeded.
	ANKI_USE_RESULT Bool allocate(PtrSize size, Address& address);

	/// Free memory.
	/// @param address The address to free.
	/// @param size The size of the allocation.
	void free(Address address, PtrSize size);

	/// Print a debug representation of the internal structures.
	void debugPrint() const;

	/// Get some info.
	void getInfo(PtrSize& userAllocatedSize, PtrSize& realAllocatedSize) const
	{
		userAllocatedSize = m_userAllocatedSize;
		realAllocatedSize = m_realAllocatedSize;
	}

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
	PtrSize m_userAllocatedSize = 0;
	PtrSize m_realAllocatedSize = 0;

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

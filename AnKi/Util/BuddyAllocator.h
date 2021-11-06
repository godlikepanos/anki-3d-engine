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
template<U32 T_MAX_MEMORY_RANGE_LOG2>
class BuddyAllocator
{
public:
	/// The type of the address.
	using Address = std::conditional_t<(T_MAX_MEMORY_RANGE_LOG2 > 32), PtrSize, U32>;

	~BuddyAllocator()
	{
		ANKI_ASSERT(m_allocationCount == 0 && "Forgot to deallocate");
	}

	/// Allocate memory.
	/// @param alloc The allocator used for internal structures of the BuddyAllocator.
	/// @param size The size of the allocation.
	/// @param[out] address The returned address if the allocation didn't fail.
	/// @return True if the allocation succeeded.
	template<typename TAllocator>
	ANKI_USE_RESULT Bool allocate(TAllocator alloc, PtrSize size, Address& address);

	/// Free memory.
	/// @param alloc The allocator used for internal structures of the BuddyAllocator.
	/// @param address The address to free.
	/// @param size The size of the allocation.
	template<typename TAllocator>
	void free(TAllocator alloc, Address address, PtrSize size);

	/// Print a debug representation of the internal structures.
	void debugPrint();

private:
	/// Because we need a constexpr version of pow.
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

	static constexpr U32 ORDER_COUNT = T_MAX_MEMORY_RANGE_LOG2 + 1;
	static constexpr PtrSize MAX_MEMORY_RANGE = pow2<PtrSize>(T_MAX_MEMORY_RANGE_LOG2);

	using FreeList = DynamicArray<Address, PtrSize>;
	Array<FreeList, ORDER_COUNT> m_freeLists;
	U32 m_allocationCount = 0;

	template<typename TAllocator>
	PtrSize popFree(TAllocator& alloc, U32 order)
	{
		ANKI_ASSERT(m_freeLists[order].getSize() > 0);
		const PtrSize address = m_freeLists[order].getBack();
		m_freeLists[order].popBack(alloc);
		ANKI_ASSERT(address < MAX_MEMORY_RANGE);
		return address;
	}

	template<typename TAllocator>
	void freeInternal(TAllocator& alloc, PtrSize address, PtrSize size);
};
/// @}

} // end namespace anki

#include <AnKi/Util/BuddyAllocator.inl.h>

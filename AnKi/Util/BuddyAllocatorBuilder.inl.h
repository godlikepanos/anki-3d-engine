// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/BuddyAllocatorBuilder.h>

namespace anki {

template<U32 kMaxMemoryRangeLog2, typename TLock, typename TMemPool>
void BuddyAllocatorBuilder<kMaxMemoryRangeLog2, TLock, TMemPool>::init(InternalMemoryPool pool, U32 maxMemoryRangeLog2)
{
	ANKI_ASSERT(maxMemoryRangeLog2 >= 1 && maxMemoryRangeLog2 <= kMaxMemoryRangeLog2);
	ANKI_ASSERT(m_freeLists.getSize() == 0 && m_userAllocatedSize == 0 && m_realAllocatedSize == 0);

	const U32 orderCount = maxMemoryRangeLog2 + 1;
	m_maxMemoryRange = pow2<PtrSize>(maxMemoryRangeLog2);

	m_pool = pool;
	m_freeLists.create(m_pool, orderCount);
}

template<U32 kMaxMemoryRangeLog2, typename TLock, typename TMemPool>
void BuddyAllocatorBuilder<kMaxMemoryRangeLog2, TLock, TMemPool>::destroy()
{
	ANKI_ASSERT(m_userAllocatedSize == 0 && "Forgot to free all memory");
	m_freeLists.destroy(m_pool);
	m_maxMemoryRange = 0;
	m_userAllocatedSize = 0;
	m_realAllocatedSize = 0;
}

template<U32 kMaxMemoryRangeLog2, typename TLock, typename TMemPool>
Bool BuddyAllocatorBuilder<kMaxMemoryRangeLog2, TLock, TMemPool>::allocate(PtrSize size, PtrSize alignment,
																		   Address& outAddress)
{
	ANKI_ASSERT(size > 0 && size <= m_maxMemoryRange);

	PtrSize alignedSize = nextPowerOfTwo(size);
	U32 order = log2(alignedSize);
	const PtrSize orderSize = pow2<PtrSize>(order);

	// The alignment for the requested "size" is the "orderSize". If the "orderSize" doesn't satisfy the "alignment"
	// parameter then we need to align the allocation address
	const Bool needsPadding = !isAligned(alignment, orderSize);

	if(needsPadding)
	{
		// We need more space to accommodate possible unaligned allocation address
		alignedSize = nextPowerOfTwo(size + alignment);

		// Re-calcuate the order as well
		order = log2(alignedSize);
	}

	LockGuard<TLock> lock(m_mutex);

	// Lazy initialize
	if(m_userAllocatedSize == 0)
	{
		const Address startingAddress = 0;
		m_freeLists.getBack().create(m_pool, 1, startingAddress);
	}

	// Find the order to start the search
	while(m_freeLists[order].getSize() == 0)
	{
		++order;
		if(order >= m_freeLists.getSize())
		{
			// Out of memory
			return false;
		}
	}

	// Iterate
	PtrSize address = popFree(order);
	while(true)
	{
		const PtrSize orderSize = pow2<PtrSize>(order);
		if(orderSize == alignedSize)
		{
			// Found the address
			break;
		}

		const PtrSize buddyAddress = address + orderSize / 2;
		ANKI_ASSERT(buddyAddress < m_maxMemoryRange && buddyAddress <= getMaxNumericLimit<Address>());

		ANKI_ASSERT(order > 0);
		m_freeLists[order - 1].emplaceBack(m_pool, Address(buddyAddress));

		--order;
	}

	// Align the returned address if needed
	if(needsPadding)
	{
		alignRoundUp(alignment, address);
	}

	ANKI_ASSERT(address + size <= m_maxMemoryRange);
	ANKI_ASSERT(isAligned(alignment, address));
	m_userAllocatedSize += size;
	m_realAllocatedSize += alignedSize;
	ANKI_ASSERT(address <= getMaxNumericLimit<Address>());
	outAddress = Address(address);
	return true;
}

template<U32 kMaxMemoryRangeLog2, typename TLock, typename TMemPool>
void BuddyAllocatorBuilder<kMaxMemoryRangeLog2, TLock, TMemPool>::free(Address address, PtrSize size, PtrSize alignment)
{
	PtrSize alignedSize = nextPowerOfTwo(size);
	U32 order = log2(alignedSize);
	const PtrSize orderSize = pow2<PtrSize>(order);

	// See allocate()
	const Bool needsPadding = !isAligned(alignment, orderSize);

	if(needsPadding)
	{
		alignedSize = nextPowerOfTwo(size + alignment);

		// Address was rounded up on allocate(), do the opposite
		alignRoundDown(orderSize, address);
	}

	LockGuard<TLock> lock(m_mutex);

	freeInternal(address, alignedSize);

	ANKI_ASSERT(m_userAllocatedSize >= size);
	m_userAllocatedSize -= size;
	ANKI_ASSERT(m_realAllocatedSize >= alignedSize);
	m_realAllocatedSize -= alignedSize;

	// Some checks
	if(m_userAllocatedSize == 0)
	{
		ANKI_ASSERT(m_realAllocatedSize == 0);
		for([[maybe_unused]] const FreeList& freeList : m_freeLists)
		{
			ANKI_ASSERT(freeList.getSize() == 0);
		}
	}
}

template<U32 kMaxMemoryRangeLog2, typename TLock, typename TMemPool>
void BuddyAllocatorBuilder<kMaxMemoryRangeLog2, TLock, TMemPool>::freeInternal(PtrSize address, PtrSize size)
{
	ANKI_ASSERT(size);
	ANKI_ASSERT(isPowerOfTwo(size));
	ANKI_ASSERT(address + size <= m_maxMemoryRange);

	if(size == m_maxMemoryRange)
	{
		return;
	}

	// Find if the buddy is in the left side of the memory address space or the right one
	const Bool buddyIsLeft = ((address / size) % 2) != 0;
	PtrSize buddyAddress;
	if(buddyIsLeft)
	{
		ANKI_ASSERT(address >= size);
		buddyAddress = address - size;
	}
	else
	{
		buddyAddress = address + size;
	}

	ANKI_ASSERT(buddyAddress + size <= m_maxMemoryRange);

	// Adjust the free lists
	const U32 order = log2(size);
	Bool buddyFound = false;
	for(PtrSize i = 0; i < m_freeLists[order].getSize(); ++i)
	{
		if(m_freeLists[order][i] == buddyAddress)
		{
			m_freeLists[order].erase(m_pool, m_freeLists[order].getBegin() + i);

			freeInternal((buddyIsLeft) ? buddyAddress : address, size * 2);
			buddyFound = true;
			break;
		}
	}

	if(!buddyFound)
	{
		ANKI_ASSERT(address <= getMaxNumericLimit<Address>());
		m_freeLists[order].emplaceBack(m_pool, Address(address));
	}
}

template<U32 kMaxMemoryRangeLog2, typename TLock, typename TMemPool>
void BuddyAllocatorBuilder<kMaxMemoryRangeLog2, TLock, TMemPool>::debugPrint() const
{
	constexpr PtrSize kMaxMemoryRange = pow2<PtrSize>(kMaxMemoryRangeLog2);

	// Allocate because we can't possibly have that in the stack
	BitSet<kMaxMemoryRange>* freeBytes = newInstance<BitSet<kMaxMemoryRange>>(m_pool, false);

	LockGuard<TLock> lock(m_mutex);

	for(I32 order = orderCount() - 1; order >= 0; --order)
	{
		const PtrSize orderSize = pow2<PtrSize>(order);
		freeBytes->unsetAll();

		printf("%d: ", order);
		for(PtrSize address : m_freeLists[order])
		{
			for(PtrSize byte = address; byte < address + orderSize; ++byte)
			{
				freeBytes->set(byte);
			}
		}

		for(PtrSize i = 0; i < m_maxMemoryRange; ++i)
		{
			putc(freeBytes->get(i) ? 'F' : '?', stdout);
		}

		printf("\n");
	}
	deleteInstance(m_pool, freeBytes);
}

template<U32 kMaxMemoryRangeLog2, typename TLock, typename TMemPool>
void BuddyAllocatorBuilder<kMaxMemoryRangeLog2, TLock, TMemPool>::getStats(BuddyAllocatorBuilderStats& stats) const
{
	LockGuard<TLock> lock(m_mutex);

	stats.m_userAllocatedSize = m_userAllocatedSize;
	stats.m_realAllocatedSize = m_realAllocatedSize;

	// Compute external fragmetation (wikipedia has the definition)
	U32 order = 0;
	U32 orderWithTheBiggestBlock = kMaxU32;
	for(const FreeList& list : m_freeLists)
	{
		if(list.getSize())
		{
			orderWithTheBiggestBlock = order;
		}
		++order;
	}
	const PtrSize biggestBlockSize =
		(orderWithTheBiggestBlock == kMaxU32) ? m_maxMemoryRange : pow2<PtrSize>(orderWithTheBiggestBlock);
	const PtrSize realFreeMemory = m_maxMemoryRange - m_realAllocatedSize;
	stats.m_externalFragmentation = F32(1.0 - F64(biggestBlockSize) / F64(realFreeMemory));

	// Internal fragmentation
	stats.m_internalFragmentation = F32(1.0 - F64(m_userAllocatedSize) / F64(m_realAllocatedSize));
}

} // end namespace anki

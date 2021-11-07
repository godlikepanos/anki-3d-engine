// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/BuddyAllocator.h>

namespace anki {

template<U32 T_MAX_MEMORY_RANGE_LOG2>
template<typename TAllocator>
Bool BuddyAllocator<T_MAX_MEMORY_RANGE_LOG2>::allocate(TAllocator alloc, PtrSize size, Address& outAddress)
{
	ANKI_ASSERT(size > 0 && size <= MAX_MEMORY_RANGE);

	// Lazy initialize
	if(m_userAllocatedSize == 0)
	{
		const Address startingAddress = 0;
		m_freeLists[ORDER_COUNT - 1].create(alloc, 1, startingAddress);
	}

	// Find the order to start the search
	const PtrSize alignedSize = nextPowerOfTwo(size);
	U32 order = log2(alignedSize);

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
	PtrSize address = popFree(alloc, order);
	while(true)
	{
		const PtrSize orderSize = pow2<PtrSize>(order);
		if(orderSize == alignedSize)
		{
			// Found the address
			break;
		}

		const PtrSize buddyAddress = address + orderSize / 2;
		ANKI_ASSERT(buddyAddress < MAX_MEMORY_RANGE && buddyAddress <= getMaxNumericLimit<Address>());

		ANKI_ASSERT(order > 0);
		m_freeLists[order - 1].emplaceBack(alloc, Address(buddyAddress));

		--order;
	}

	ANKI_ASSERT(address + alignedSize <= MAX_MEMORY_RANGE);
	m_userAllocatedSize += size;
	m_realAllocatedSize += alignedSize;
	ANKI_ASSERT(address <= getMaxNumericLimit<Address>());
	outAddress = Address(address);
	return true;
}

template<U32 T_MAX_MEMORY_RANGE_LOG2>
template<typename TAllocator>
void BuddyAllocator<T_MAX_MEMORY_RANGE_LOG2>::free(TAllocator alloc, Address address, PtrSize size)
{
	const PtrSize alignedSize = nextPowerOfTwo(size);
	freeInternal(alloc, address, alignedSize);

	ANKI_ASSERT(m_userAllocatedSize >= size);
	m_userAllocatedSize -= size;
	ANKI_ASSERT(m_realAllocatedSize >= alignedSize);
	m_realAllocatedSize -= alignedSize;

	// Some checks
	if(m_userAllocatedSize == 0)
	{
		ANKI_ASSERT(m_realAllocatedSize == 0);
		for(const FreeList& freeList : m_freeLists)
		{
			ANKI_ASSERT(freeList.getSize() == 0);
		}
	}
}

template<U32 T_MAX_MEMORY_RANGE_LOG2>
template<typename TAllocator>
void BuddyAllocator<T_MAX_MEMORY_RANGE_LOG2>::freeInternal(TAllocator& alloc, PtrSize address, PtrSize size)
{
	ANKI_ASSERT(isPowerOfTwo(size));
	ANKI_ASSERT(address + size <= MAX_MEMORY_RANGE);

	if(size == MAX_MEMORY_RANGE)
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

	ANKI_ASSERT(buddyAddress + size <= MAX_MEMORY_RANGE);

	// Adjust the free lists
	const U32 order = log2(size);
	Bool buddyFound = false;
	for(PtrSize i = 0; i < m_freeLists[order].getSize(); ++i)
	{
		if(m_freeLists[order][i] == buddyAddress)
		{
			m_freeLists[order].erase(alloc, m_freeLists[order].getBegin() + i);

			freeInternal(alloc, (buddyIsLeft) ? buddyAddress : address, size * 2);
			buddyFound = true;
			break;
		}
	}

	if(!buddyFound)
	{
		ANKI_ASSERT(address <= getMaxNumericLimit<Address>());
		m_freeLists[order].emplaceBack(alloc, Address(address));
	}
}

template<U32 T_MAX_MEMORY_RANGE_LOG2>
void BuddyAllocator<T_MAX_MEMORY_RANGE_LOG2>::debugPrint() const
{
	BitSet<MAX_MEMORY_RANGE>* freeBytes = new BitSet<MAX_MEMORY_RANGE>(false);
	for(I32 order = ORDER_COUNT - 1; order >= 0; --order)
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

		for(PtrSize i = 0; i < MAX_MEMORY_RANGE; ++i)
		{
			putc(freeBytes->get(i) ? 'F' : '?', stdout);
		}

		printf("\n");
	}
	delete freeBytes;
}

} // end namespace anki

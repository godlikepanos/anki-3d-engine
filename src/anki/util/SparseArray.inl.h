// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/SparseArray.h>

namespace anki
{

template<typename T, typename TIndex>
template<typename TAlloc>
void SparseArray<T, TIndex>::destroy(TAlloc& alloc)
{
	if(m_elements)
	{
		for(Index i = 0; i < m_capacity; ++i)
		{
			if(m_elements[i].m_alive)
			{
				m_elements[i].m_value.~Value();
			}
		}

		alloc.deallocate(m_elements, m_capacity);
	}

	resetMembers();
}

template<typename T, typename TIndex>
template<typename TAlloc, typename... TArgs>
void SparseArray<T, TIndex>::emplace(TAlloc& alloc, Index idx, TArgs&&... args)
{
	if(m_capacity == 0 || calcLoadFactor() > m_maxLoadFactor)
	{
		grow(alloc);
	}

	Value tmp(std::forward<TArgs>(args)...);
	m_elementCount += insert(alloc, idx, tmp);
}

template<typename T, typename TIndex>
template<typename TAlloc>
U32 SparseArray<T, TIndex>::insert(TAlloc& alloc, Index idx, Value& val)
{
start:
	Index desiredPos = mod(idx);
	Index endPos = mod(desiredPos + m_probeCount);
	Index pos = desiredPos;

	while(pos != endPos)
	{
		Element& el = m_elements[pos];

		if(!el.m_alive)
		{
			// Empty slot was found, construct in-place

			el.m_alive = true;
			el.m_idx = idx;
			::new(&el.m_value) Value(std::move(val));

			return 1;
		}
		else if(el.m_idx == idx)
		{
			// Same index was found, replace

			el.m_idx = idx;
			el.m_value.~Value();
			::new(&el.m_value) Value(std::move(val));

			return 0;
		}

		// Do the robin-hood
		const Index otherDesiredPos = mod(el.m_idx);
		if(distanceFromDesired(pos, otherDesiredPos) < distanceFromDesired(pos, desiredPos))
		{
			// Swap
			std::swap(val, el.m_value);
			std::swap(idx, el.m_idx);
			goto start;
		}

		pos = mod(pos + 1u);
	}

	// Didn't found an empty place, need to grow and try again
	grow(alloc);
	goto start;

	ANKI_ASSERT(0);
	return 0;
}

template<typename T, typename TIndex>
template<typename TAlloc>
void SparseArray<T, TIndex>::grow(TAlloc& alloc)
{
	if(m_capacity == 0)
	{
		ANKI_ASSERT(m_elementCount == 0);
		m_capacity = m_initialStorageSize;
		m_elements =
			static_cast<Element*>(alloc.getMemoryPool().allocate(m_capacity * sizeof(Element), alignof(Element)));
		memset(m_elements, 0, m_capacity * sizeof(Element));
		return;
	}

	// Find from where we start
	Index startPos = ~Index(0);
	for(U i = 0; i < m_capacity; ++i)
	{
		if(m_elements[i].m_alive)
		{
			const Index desiredPos = mod(m_elements[i].m_idx);
			if(desiredPos <= i)
			{
				startPos = i;
				break;
			}
		}
	}
	ANKI_ASSERT(startPos != ~Index(0));

	// Allocate new storage
	Element* oldElements = m_elements;
	const U32 oldCapacity = m_capacity;
	const U32 oldElementCount = m_elementCount;
	(void)oldElementCount;

	m_capacity *= 2;
	m_elements = static_cast<Element*>(alloc.getMemoryPool().allocate(m_capacity * sizeof(Element), alignof(Element)));
	memset(m_elements, 0, m_capacity * sizeof(Element));
	m_elementCount = 0;

	// Start re-inserting
	U count = oldCapacity;
	Index pos = startPos;
	while(count--)
	{
		if(oldElements[pos].m_alive)
		{
			Element& el = oldElements[pos];
			U32 c = insert(alloc, el.m_idx, el.m_value);
			ANKI_ASSERT(c > 0);
			m_elementCount += c;
		}

		pos = mod(pos + 1, oldCapacity);
	}

	ANKI_ASSERT(oldElementCount == m_elementCount);

	// Finalize
	alloc.getMemoryPool().free(oldElements);
}

template<typename T, typename TIndex>
template<typename TAlloc>
void SparseArray<T, TIndex>::erase(TAlloc& alloc, Iterator it)
{
	ANKI_ASSERT(it.m_array == this);
	ANKI_ASSERT(it.m_elementIdx != MAX_U32);
	ANKI_ASSERT(m_elementCount > 0);

	(void)alloc;

	const Index pos = it.m_elementIdx;
	ANKI_ASSERT(pos < m_capacity);
	ANKI_ASSERT(m_elements[pos].m_alive);

	// Delete the element in the given pos
	m_elements[pos].m_value.~Value();
	m_elements[pos].m_alive = false;
	--m_elementCount;

	// Shift elements
	Index nextPos = pos;
	while(true)
	{
		const Index crntPos = nextPos;
		nextPos = mod(nextPos + 1);

		Element& nextEl = m_elements[nextPos];

		if(!nextEl.m_alive)
		{
			// On gaps, stop
			break;
		}

		const Index nextDesiredPos = mod(nextEl.m_idx);
		if(nextDesiredPos == nextPos)
		{
			// The element is where it want's to be, stop
			break;
		}

		// Shift left
		Element& crntEl = m_elements[crntPos];
		crntEl.m_value = std::move(nextEl.m_value);
		crntEl.m_idx = nextEl.m_value;
		crntEl.m_alive = true;
		nextEl.m_alive = false;
	}

	// If you erased everything destroy the storage
	if(m_elementCount == 0)
	{
		destroy(alloc);
	}
}

template<typename T, typename TIndex>
void SparseArray<T, TIndex>::validate() const
{
	if(m_capacity == 0)
	{
		ANKI_ASSERT(m_elementCount == 0 && m_elements == 0);
		return;
	}

	ANKI_ASSERT(m_elementCount < m_capacity);

	// Find from where we start
	Index startPos = ~Index(0);
	for(U i = 0; i < m_capacity; ++i)
	{
		if(m_elements[i].m_alive)
		{
			const Index desiredPos = mod(m_elements[i].m_idx);
			if(desiredPos <= i)
			{
				startPos = i;
				break;
			}
		}
	}

	// Start iterating
	U elementCount = 0;
	U count = m_capacity;
	Index pos = startPos;
	Index prevPos = ~Index(0);
	while(count--)
	{
		if(m_elements[pos].m_alive)
		{
			const Index myDesiredPos = mod(m_elements[pos].m_idx);
			const Index myDistanceFromDesired = distanceFromDesired(pos, myDesiredPos);
			ANKI_ASSERT(myDistanceFromDesired < m_probeCount);

			if(prevPos != ~Index(0))
			{
				Index prevDesiredPos = mod(m_elements[prevPos].m_idx);
				ANKI_ASSERT(myDesiredPos >= prevDesiredPos);
			}

			++elementCount;
			prevPos = pos;
		}
		else
		{
			prevPos = ~Index(0);
		}

		pos = mod(pos + 1);
	}

	ANKI_ASSERT(m_elementCount == elementCount);
}

template<typename T, typename TIndex>
TIndex SparseArray<T, TIndex>::findInternal(Index idx) const
{
	if(ANKI_UNLIKELY(m_elementCount == 0))
	{
		return MAX_U32;
	}

	const Index desiredPos = mod(idx);
	const Index endPos = mod(desiredPos + m_probeCount);
	Index pos = desiredPos;
	while(pos != endPos)
	{
		if(m_elements[pos].m_alive && m_elements[pos].m_idx == idx)
		{
			return pos;
		}

		pos = mod(pos + 1);
	}

	return MAX_U32;
}

} // end namespace anki

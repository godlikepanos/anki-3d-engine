// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/DynamicArray.h>

namespace anki {

template<typename T, typename TMemoryPool, typename TSize>
void DynamicArray<T, TMemoryPool, TSize>::resizeStorage(Size newSize)
{
	if(newSize > m_capacity)
	{
		// Need to grow

		m_capacity = (newSize > Size(F64(m_capacity) * kGrowScale)) ? newSize : Size(F64(m_capacity) * kGrowScale);
		Value* newStorage = static_cast<Value*>(m_pool.allocate(m_capacity * sizeof(Value), alignof(Value)));

		// Move old elements to the new storage
		if(m_data)
		{
			for(Size i = 0; i < m_size; ++i)
			{
				callConstructor(newStorage[i], std::move(m_data[i]));
				m_data[i].~T();
			}

			m_pool.free(m_data);
		}

		m_data = newStorage;
	}
	else if(newSize < m_size)
	{
		ANKI_ASSERT(m_capacity > 0);
		ANKI_ASSERT(m_size > 0);
		ANKI_ASSERT(m_data);

		// Delete remaining stuff
		for(Size i = newSize; i < m_size; ++i)
		{
			m_data[i].~T();
		}

		m_size = newSize;

		if(newSize < Size(F64(m_capacity) / kShrinkScale) || newSize == 0)
		{
			// Need to shrink

			m_capacity = newSize;
			if(newSize)
			{
				Value* newStorage = static_cast<Value*>(m_pool.allocate(m_capacity * sizeof(Value), alignof(Value)));

				for(Size i = 0; i < m_size; ++i)
				{
					callConstructor(newStorage[i], std::move(m_data[i]));
					m_data[i].~T();
				}

				m_pool.free(m_data);
				m_data = newStorage;
			}
			else
			{
				m_pool.free(m_data);
				m_data = nullptr;
			}
		}
	}
}

template<typename T, typename TMemoryPool, typename TSize>
template<typename... TArgs>
void DynamicArray<T, TMemoryPool, TSize>::resize(Size newSize, TArgs... args)
{
	if constexpr(std::is_copy_constructible<T>::value)
	{
		const Bool willGrow = newSize > m_size;
		resizeStorage(newSize);

		if(willGrow)
		{
			// Fill with new values
			for(TSize i = m_size; i < newSize; ++i)
			{
				::new(&m_data[i]) T(args...);
			}

			m_size = newSize;
		}
	}
	else
	{
		ANKI_ASSERT(m_size == 0 && "Cannot resize storage for non-copyable");
		m_data = static_cast<T*>(m_pool.allocate(sizeof(T) * newSize, alignof(T)));
		for(TSize i = 0; i < newSize; ++i)
		{
			::new(&m_data[i]) T(args...);
		}
		m_size = newSize;
		m_capacity = newSize;
	}

	ANKI_ASSERT(m_size <= m_capacity);
	ANKI_ASSERT(m_size == newSize);
}

template<typename T, typename TMemoryPool, typename TSize>
template<typename... TArgs>
typename DynamicArray<T, TMemoryPool, TSize>::Iterator
DynamicArray<T, TMemoryPool, TSize>::emplaceAt(ConstIterator where, TArgs&&... args)
{
	const Value* wherePtr = where;
	Size outIdx = getMaxNumericLimit<Size>();

	if(wherePtr != nullptr)
	{
		// The "where" arg points to an element inside the array or the end.

		// Preconditions
		ANKI_ASSERT(wherePtr >= m_data);
		ANKI_ASSERT(wherePtr <= m_data + m_size);
		ANKI_ASSERT(!isEmpty());

		const Size oldSize = m_size;

		const Size whereIdx = Size(wherePtr - m_data); // Get that before grow the storage
		ANKI_ASSERT(whereIdx >= 0u && whereIdx <= oldSize);

		// Resize storage
		resizeStorage(oldSize + 1u);

		Size elementsToMoveRight = oldSize - whereIdx;
		if(elementsToMoveRight == 0)
		{
			// "where" arg points to the end of the array

			outIdx = oldSize;
		}
		else
		{
			// Construct the last element because we will move to it
			callConstructor(m_data[oldSize]);

			// Move the elements one place to the right
			while(elementsToMoveRight--)
			{
				const Size idx = whereIdx + elementsToMoveRight;

				m_data[idx + 1] = std::move(m_data[idx]);
			}

			// Even if it's moved, call the destructor
			m_data[whereIdx].~Value();

			// Construct our object
			outIdx = whereIdx;
		}
	}
	else
	{
		// The "where" arg points to an empty array. Easy to handle

		ANKI_ASSERT(isEmpty());

		resizeStorage(1);
		outIdx = 0;
	}

	// Construct the new object
	ANKI_ASSERT(outIdx != getMaxNumericLimit<Size>());
	callConstructor(m_data[outIdx], std::forward<TArgs>(args)...);

	// Increase the size because resizeStorage will not
	++m_size;

	return &m_data[outIdx];
}

template<typename T, typename TMemoryPool, typename TSize>
void DynamicArray<T, TMemoryPool, TSize>::erase(ConstIterator first, ConstIterator last)
{
	ANKI_ASSERT(first != last);
	ANKI_ASSERT(m_data);
	ANKI_ASSERT(first >= m_data && first < m_data + m_size);
	ANKI_ASSERT(last > m_data && last <= m_data + m_size);

	// Move from the back to close the gap
	const Size firsti = Size(first - m_data);
	const Size lasti = Size(last - m_data);
	const Size toMove = m_size - lasti;
	for(Size i = 0; i < toMove; ++i)
	{
		m_data[firsti + i] = std::move(m_data[lasti + i]);
	}

	// Resize storage
	const Size newSize = m_size - Size(last - first);
	resizeStorage(newSize);
}

} // end namespace anki

// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/DynamicArray.h>

namespace anki
{

template<typename T>
template<typename TAllocator>
void DynamicArray<T>::resizeStorage(TAllocator& alloc, PtrSize newSize)
{
	if(newSize > m_capacity)
	{
		// Need to grow

		m_capacity = (newSize > m_capacity * GROW_SCALE) ? newSize : (m_capacity * GROW_SCALE);
		Value* newStorage =
			static_cast<Value*>(alloc.getMemoryPool().allocate(m_capacity * sizeof(Value), alignof(Value)));

		// Move old elements to the new storage
		if(m_data)
		{
			for(PtrSize i = 0; i < m_size; ++i)
			{
				::new(&newStorage[i]) Value(std::move(m_data[i]));
				m_data[i].~T();
			}

			alloc.getMemoryPool().free(m_data);
		}

		m_data = newStorage;
	}
	else if(newSize < m_size)
	{
		ANKI_ASSERT(m_capacity > 0);
		ANKI_ASSERT(m_size > 0);
		ANKI_ASSERT(m_data);

		// Delete remaining stuff
		for(U i = newSize; i < m_size; ++i)
		{
			m_data[i].~T();
		}

		m_size = newSize;

		if(newSize < m_capacity / SHRINK_SCALE || newSize == 0)
		{
			// Need to shrink

			m_capacity = newSize;
			if(newSize)
			{
				Value* newStorage =
					static_cast<Value*>(alloc.getMemoryPool().allocate(m_capacity * sizeof(Value), alignof(Value)));

				for(PtrSize i = 0; i < m_size; ++i)
				{
					::new(&newStorage[i]) Value(std::move(m_data[i]));
					m_data[i].~T();
				}

				alloc.getMemoryPool().free(m_data);
				m_data = newStorage;
			}
			else
			{
				alloc.getMemoryPool().free(m_data);
				m_data = nullptr;
			}
		}
	}
}

template<typename T>
template<typename TAllocator>
void DynamicArray<T>::resize(TAllocator alloc, PtrSize newSize, const Value& v)
{
	const Bool willGrow = newSize > m_size;
	resizeStorage(alloc, newSize);

	if(willGrow)
	{
		// Fill with new values
		for(U i = m_size; i < newSize; ++i)
		{
			::new(&m_data[i]) Value(v);
		}

		m_size = newSize;
	}

	ANKI_ASSERT(m_size <= m_capacity);
	ANKI_ASSERT(m_size == newSize);
}

template<typename T>
template<typename TAllocator>
void DynamicArray<T>::resize(TAllocator alloc, PtrSize newSize)
{
	const Bool willGrow = newSize > m_size;
	resizeStorage(alloc, newSize);

	if(willGrow)
	{
		// Fill with new values
		for(U i = m_size; i < newSize; ++i)
		{
			::new(&m_data[i]) Value();
		}

		m_size = newSize;
	}

	ANKI_ASSERT(m_size <= m_capacity);
	ANKI_ASSERT(m_size == newSize);
}

} // end namespace anki
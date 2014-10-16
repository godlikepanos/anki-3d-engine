// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_D_ARRAY_H
#define ANKI_UTIL_D_ARRAY_H

#include "anki/util/Allocator.h"
#include "anki/util/NonCopyable.h"

namespace anki {

/// @addtogroup util_containers
/// @{

/// Dynamic array.
template<typename T, typename TAlloc = HeapAllocator<T>>
class DArray: public NonCopyable
{
public:
	using Value = T;
	using Allocator = TAlloc;
	using Iterator = Value*;
	using ConstIterator = const Value*;
	using Reference = Value&;
	using ConstReference = const Value&;

	// STL compatible
	using iterator = Iterator;
	using const_iterator = ConstIterator;
	using reference = Reference;
	using const_reference = ConstReference;

	DArray()
	:	m_data(nullptr),
		m_size(0)
	{}

	DArray(DArray&& b)
	:	DArray()
	{
		move(b);
	}

	~DArray()
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0 
			&& "Requires manual destruction");
	}

	DArray& operator=(DArray&& b)
	{
		move(b);
		return *this;
	}

	Reference operator[](const PtrSize n)
	{
		ANKI_ASSERT(n < m_size);
		return m_data[n];
	}

	ConstReference operator[](const PtrSize n) const
	{
		ANKI_ASSERT(n < m_size);
		return m_data[n];
	}

	/// Make it compatible with the C++11 range based for loop.
	Iterator begin()
	{
		return &m_data[0];
	}

	/// Make it compatible with the C++11 range based for loop.
	ConstIterator begin() const
	{
		return &m_data[0];
	}

	/// Make it compatible with the C++11 range based for loop.
	Iterator end()
	{
		return &m_data[0] + m_size;
	}

	/// Make it compatible with the C++11 range based for loop.
	ConstIterator end() const
	{
		return &m_data[0] + m_size;
	}

	/// Make it compatible with STL.
	Reference front() 
	{
		return m_data[0];
	}

	/// Make it compatible with STL.
	ConstReference front() const
	{
		return m_data[0];
	}

	/// Make it compatible with STL.
	Reference back() 
	{
		return m_data[m_size - 1];
	}

	/// Make it compatible with STL.
	ConstReference back() const
	{
		return m_data[m_size - 1];
	}

	PtrSize getSize() const
	{
		return m_size;
	}

	/// Create the array.
	template<typename... TArgs>
	ANKI_USE_RESULT Error create(Allocator alloc, PtrSize size, TArgs... args)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0);
		ANKI_ASSERT(size);
		Error err = ErrorCode::NONE;

		destroy(alloc);

		m_data = alloc.template newArray<Value>(size, args...);
		if(m_data)
		{
			m_size = size;
		}
		else
		{
			err = ErrorCode::OUT_OF_MEMORY;
		}

		return err;
	}

	/// Destroy the array.
	void destroy(Allocator alloc)
	{
		if(m_data)
		{
			ANKI_ASSERT(m_size > 0);
			alloc.deleteArray(m_data, m_size);

			m_data = nullptr;
			m_size = 0;
		}

		ANKI_ASSERT(m_data == nullptr && m_size == 0);
	}

private:
	Value* m_data;
	U32 m_size;

	void move(DArray& b)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0
			&& "Cannot move before destroying");
		m_data = b.m_data;
		b.m_data = nullptr;
		m_size = b.m_size;
		b.m_size = 0;
	}
};

/// @}

} // end namespace anki

#endif

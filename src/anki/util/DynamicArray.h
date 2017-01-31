// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Allocator.h>
#include <anki/util/NonCopyable.h>
#include <anki/util/Functions.h>

namespace anki
{

/// @addtogroup util_containers
/// @{

/// The base of DynamicArray and WeakArray.
template<typename T>
class DynamicArrayBase
{
public:
	using Value = T;
	using Iterator = Value*;
	using ConstIterator = const Value*;
	using Reference = Value&;
	using ConstReference = const Value&;

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

	Iterator getBegin()
	{
		return m_data;
	}

	ConstIterator getBegin() const
	{
		return m_data;
	}

	Iterator getEnd()
	{
		return m_data + m_size;
	}

	ConstIterator getEnd() const
	{
		return m_data + m_size;
	}

	/// Make it compatible with the C++11 range based for loop.
	Iterator begin()
	{
		return getBegin();
	}

	/// Make it compatible with the C++11 range based for loop.
	ConstIterator begin() const
	{
		return getBegin();
	}

	/// Make it compatible with the C++11 range based for loop.
	Iterator end()
	{
		return getEnd();
	}

	/// Make it compatible with the C++11 range based for loop.
	ConstIterator end() const
	{
		return getEnd();
	}

	/// Get first element.
	Reference getFront()
	{
		ANKI_ASSERT(!isEmpty());
		return m_data[0];
	}

	/// Get first element.
	ConstReference getFront() const
	{
		ANKI_ASSERT(!isEmpty());
		return m_data[0];
	}

	/// Get last element.
	Reference getBack()
	{
		ANKI_ASSERT(!isEmpty());
		return m_data[m_size - 1];
	}

	/// Get last element.
	ConstReference getBack() const
	{
		ANKI_ASSERT(!isEmpty());
		return m_data[m_size - 1];
	}

	PtrSize getSize() const
	{
		return m_size;
	}

	PtrSize getByteSize() const
	{
		return m_size * sizeof(Value);
	}

	Bool isEmpty() const
	{
		return m_size == 0;
	}

	PtrSize getSizeInBytes() const
	{
		return m_size * sizeof(Value);
	}

protected:
	DynamicArrayBase(Value* v, PtrSize size)
		: m_data(v)
		, m_size(size)
	{
	}

	Value* m_data;
	PtrSize m_size;
};

/// Dynamic array with manual destruction. It doesn't hold the allocator and that makes it compact. At the same time
/// that requires manual destruction. Used in permanent classes.
template<typename T>
class DynamicArray : public NonCopyable, public DynamicArrayBase<T>
{
public:
	using Base = DynamicArrayBase<T>;
	using Base::m_data;
	using Base::m_size;
	using typename Base::Value;

	DynamicArray()
		: Base(nullptr, 0)
	{
	}

	/// Move.
	DynamicArray(DynamicArray&& b)
		: DynamicArray()
	{
		*this = std::move(b);
	}

	~DynamicArray()
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0 && "Requires manual destruction");
	}

	/// Move.
	DynamicArray& operator=(DynamicArray&& b)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0 && "Cannot move before destroying");
		m_data = b.m_data;
		b.m_data = nullptr;
		m_size = b.m_size;
		b.m_size = 0;
		return *this;
	}

	/// Create the array.
	template<typename TAllocator>
	void create(TAllocator alloc, PtrSize size)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0);

		if(size > 0)
		{
			m_data = alloc.template newArray<Value>(size);
			m_size = size;
		}
	}

	/// Create the array.
	template<typename TAllocator>
	void create(TAllocator alloc, PtrSize size, const Value& v)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0);

		if(size > 0)
		{
			m_data = alloc.template newArray<Value>(size, v);
			m_size = size;
		}
	}

	/// Grow the array.
	template<typename TAllocator>
	void resize(TAllocator alloc, PtrSize size)
	{
		ANKI_ASSERT(size > 0);
		DynamicArray newArr;
		newArr.create(alloc, size);

		PtrSize minSize = min<PtrSize>(size, m_size);
		for(U i = 0; i < minSize; i++)
		{
			newArr[i] = std::move((*this)[i]);
		}

		destroy(alloc);
		*this = std::move(newArr);
	}

	/// Destroy the array.
	template<typename TAllocator>
	void destroy(TAllocator alloc)
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
};

/// Dynamic array with automatic destruction. It's the same as DynamicArray but it holds the allocator in order to
/// perform automatic destruction. Use it for temp operations and on transient classes.
template<typename T>
class DynamicArrayAuto : public DynamicArray<T>
{
public:
	using Base = DynamicArray<T>;
	using Base::m_data;
	using Base::m_size;
	using typename Base::Value;

	template<typename TAllocator>
	DynamicArrayAuto(TAllocator alloc)
		: Base()
		, m_alloc(alloc)
	{
	}

	/// Move.
	DynamicArrayAuto(DynamicArrayAuto&& b)
		: Base()
	{
		*this = std::move(b);
	}

	~DynamicArrayAuto()
	{
		Base::destroy(m_alloc);
	}

	/// Move.
	DynamicArrayAuto& operator=(DynamicArrayAuto&& b)
	{
		Base::destroy(m_alloc);

		m_data = b.m_data;
		b.m_data = nullptr;
		m_size = b.m_size;
		b.m_size = 0;
		m_alloc = b.m_alloc;
		b.m_alloc = {};
		return *this;
	}

	/// Create the array.
	void create(PtrSize size)
	{
		Base::create(m_alloc, size);
	}

	/// Create the array.
	void create(PtrSize size, const Value& v)
	{
		Base::create(m_alloc, size, v);
	}

	/// Grow the array.
	void resize(PtrSize size)
	{
		Base::resize(m_alloc, size);
	}

private:
	GenericMemoryPoolAllocator<T> m_alloc;
};

/// Array with preallocated memory.
template<typename T>
class WeakArray : public DynamicArrayBase<T>
{
public:
	using Base = DynamicArrayBase<T>;
	using Base::m_data;
	using Base::m_size;

	WeakArray()
		: Base(nullptr, 0)
	{
	}

	WeakArray(T* mem, PtrSize size)
		: Base(mem, size)
	{
		if(size)
		{
			ANKI_ASSERT(mem);
		}
	}

	/// Copy.
	WeakArray(const WeakArray& b)
		: Base(b.m_data, b.m_size)
	{
	}

	/// Move.
	WeakArray(WeakArray&& b)
		: WeakArray()
	{
		*this = std::move(b);
	}

	~WeakArray()
	{
#if ANKI_EXTRA_CHECKS
		m_data = nullptr;
		m_size = 0;
#endif
	}

	/// Copy.
	WeakArray& operator=(const WeakArray& b)
	{
		m_data = b.m_data;
		m_size = b.m_size;
		return *this;
	}

	/// Move.
	WeakArray& operator=(WeakArray&& b)
	{
		m_data = b.m_data;
		b.m_data = nullptr;
		m_size = b.m_size;
		b.m_size = 0;
		return *this;
	}
};
/// @}

} // end namespace anki

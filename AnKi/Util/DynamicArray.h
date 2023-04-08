// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/MemoryPool.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/Forward.h>

namespace anki {

/// @addtogroup util_containers
/// @{

/// Dynamic array with manual destruction. It doesn't hold the allocator and that makes it compact. At the same time
/// that requires manual destruction. Used in permanent classes.
/// @tparam T The type this array will hold.
/// @tparam TSize The type that denotes the maximum number of elements of the array.
template<typename T, typename TMemoryPool = SingletonMemoryPoolWrapper<DefaultMemoryPool>, typename TSize = U32>
class DynamicArray
{
public:
	using Value = T;
	using Iterator = Value*;
	using ConstIterator = const Value*;
	using Reference = Value&;
	using ConstReference = const Value&;
	using Size = TSize;

	static constexpr F32 kGrowScale = 2.0f;
	static constexpr F32 kShrinkScale = 2.0f;

	DynamicArray(const TMemoryPool& pool = TMemoryPool())
		: m_pool(pool)
	{
	}

	/// Copy.
	DynamicArray(const DynamicArray& b)
	{
		*this = b;
	}

	/// Move.
	DynamicArray(DynamicArray&& b)
		: DynamicArray()
	{
		*this = std::move(b);
	}

	~DynamicArray()
	{
		destroy();
	}

	/// Move.
	DynamicArray& operator=(DynamicArray&& b)
	{
		destroy();
		m_pool = b.m_pool;
		m_data = b.m_data;
		b.m_data = nullptr;
		m_size = b.m_size;
		b.m_size = 0;
		m_capacity = b.m_capacity;
		b.m_capacity = 0;
		return *this;
	}

	/// Copy and trim extra capacity.
	DynamicArray& operator=(const DynamicArray& b)
	{
		destroy();
		m_pool = b.m_pool;
		m_size = b.m_size;
		m_capacity = b.m_size;
		m_data = static_cast<T*>(m_pool.allocate(sizeof(T) * m_size, alignof(T)));
		for(TSize i = 0; i < m_size; ++i)
		{
			::new(&m_data[i]) T(b.m_data[i]);
		}
		return *this;
	}

	Reference operator[](const Size n)
	{
		ANKI_ASSERT(n < m_size);
		return m_data[n];
	}

	ConstReference operator[](const Size n) const
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

	Size getSize() const
	{
		return m_size;
	}

	Bool isEmpty() const
	{
		return m_size == 0;
	}

	PtrSize getSizeInBytes() const
	{
		return m_size * sizeof(Value);
	}

	/// Destroy the array.
	void destroy()
	{
		if(m_data)
		{
			ANKI_ASSERT(m_size > 0);
			ANKI_ASSERT(m_capacity > 0);
			deleteArray(m_pool, m_data, m_size);

			m_data = nullptr;
			m_size = 0;
			m_capacity = 0;
		}

		ANKI_ASSERT(m_data == nullptr && m_size == 0 && m_capacity == 0);
	}

	/// Grow or create the array. @a T needs to be copyable and moveable else you might get an assertion.
	template<typename... TArgs>
	void resize(Size size, TArgs... args);

	/// Push back value.
	template<typename... TArgs>
	Iterator emplaceBack(TArgs&&... args)
	{
		resizeStorage(m_size + 1);
		callConstructor(m_data[m_size], std::forward<TArgs>(args)...);
		++m_size;
		return &m_data[m_size - 1];
	}

	/// Remove the last value.
	void popBack()
	{
		if(m_size > 0)
		{
			resizeStorage(m_size - 1);
		}
	}

	/// Emplace a new element at a specific position. @a T needs to be movable and default constructible.
	/// @param where Points to the position to emplace. Should be less or equal to what getEnd() returns.
	/// @param args  Constructor arguments.
	template<typename... TArgs>
	Iterator emplaceAt(ConstIterator where, TArgs&&... args);

	/// Removes the (first, last] elements.
	/// @param first Points to the position of the first element to remove.
	/// @param last Points to the position of the last element to remove minus one.
	void erase(ConstIterator first, ConstIterator last);

	/// Removes one element.
	/// @param at Points to the position of the element to remove.
	void erase(ConstIterator at)
	{
		erase(at, at + 1);
	}

	/// Validate it. Will only work when assertions are enabled.
	void validate() const
	{
		if(m_data)
		{
			ANKI_ASSERT(m_size > 0 && m_capacity > 0);
			ANKI_ASSERT(m_size <= m_capacity);
		}
		else
		{
			ANKI_ASSERT(m_size == 0 && m_capacity == 0);
		}
	}

	/// Move the data from this object. It's like moving (operator or constructor) but instead of moving to another
	/// object of the same type it moves to 3 values.
	void moveAndReset(Value*& data, Size& size, Size& storageSize)
	{
		data = m_data;
		size = m_size;
		storageSize = m_capacity;
		m_data = nullptr;
		m_size = 0;
		m_capacity = 0;
	}

	/// @copydoc moveAndReset
	void moveAndReset(WeakArray<Value, Size>& array)
	{
		Value* data;
		Size size;
		Size storageSize;
		moveAndReset(data, size, storageSize);
		array.setArray(data, size);
	}

	/// Resizes the storage but DOESN'T CONSTRUCT ANY ELEMENTS. It only moves or destroys.
	void resizeStorage(Size newSize);

	TMemoryPool& getMemoryPool()
	{
		return m_pool;
	}

protected:
	TMemoryPool m_pool;
	Value* m_data = nullptr;
	Size m_size = 0;
	Size m_capacity = 0;
};
/// @}

} // end namespace anki

#include <AnKi/Util/DynamicArray.inl.h>

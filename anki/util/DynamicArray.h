// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Allocator.h>
#include <anki/util/Functions.h>

namespace anki
{

// Forward
template<typename T, typename TSize>
class DynamicArrayAuto;

/// @addtogroup util_containers
/// @{

/// Dynamic array with manual destruction. It doesn't hold the allocator and that makes it compact. At the same time
/// that requires manual destruction. Used in permanent classes.
/// @tparam T The type this array will hold.
/// @tparam TSize The type that denotes the maximum number of elements of the array.
template<typename T, typename TSize = U32>
class DynamicArray
{
public:
	using Value = T;
	using Iterator = Value*;
	using ConstIterator = const Value*;
	using Reference = Value&;
	using ConstReference = const Value&;
	using Size = TSize;

	static constexpr F32 GROW_SCALE = 2.0f;
	static constexpr F32 SHRINK_SCALE = 2.0f;

	DynamicArray()
		: m_data(nullptr)
		, m_size(0)
		, m_capacity(0)
	{
	}

	// Non-copyable
	DynamicArray(const DynamicArray& b) = delete;

	/// Move.
	DynamicArray(DynamicArray&& b)
		: DynamicArray()
	{
		*this = std::move(b);
	}

	~DynamicArray()
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0 && m_capacity == 0 && "Requires manual destruction");
	}

	/// Move.
	DynamicArray& operator=(DynamicArray&& b)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0 && "Cannot move before destroying");
		m_data = b.m_data;
		b.m_data = nullptr;
		m_size = b.m_size;
		b.m_size = 0;
		m_capacity = b.m_capacity;
		b.m_capacity = 0;
		return *this;
	}

	/// Move DynamicArrayAuto to this.
	DynamicArray& operator=(DynamicArrayAuto<T, TSize>&& b);

	// Non-copyable
	DynamicArray& operator=(const DynamicArray& b) = delete;

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

	/// Only create the array. Useful if @a T is non-copyable or movable .
	template<typename TAllocator>
	void create(TAllocator alloc, Size size)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0 && m_capacity == 0);
		if(size > 0)
		{
			m_data = alloc.template newArray<Value>(size);
			m_size = size;
			m_capacity = size;
		}
	}

	/// Only create the array. Useful if @a T is non-copyable or movable .
	template<typename TAllocator>
	void create(TAllocator alloc, Size size, const Value& v)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0 && m_capacity == 0);
		if(size > 0)
		{
			m_data = alloc.template newArray<Value>(size, v);
			m_size = size;
			m_capacity = size;
		}
	}

	/// Grow or create the array. @a T needs to be copyable and moveable.
	template<typename TAllocator>
	void resize(TAllocator alloc, Size size, const Value& v);

	/// Grow or create the array. @a T needs to be copyable, moveable and default constructible.
	template<typename TAllocator>
	void resize(TAllocator alloc, Size size);

	/// Push back value.
	template<typename TAllocator, typename... TArgs>
	Iterator emplaceBack(TAllocator alloc, TArgs&&... args)
	{
		resizeStorage(alloc, m_size + 1);
		alloc.construct(&m_data[m_size], std::forward<TArgs>(args)...);
		++m_size;
		return &m_data[m_size - 1];
	}

	/// Remove the last value.
	template<typename TAllocator>
	void popBack(TAllocator alloc)
	{
		if(m_size > 0)
		{
			resizeStorage(alloc, m_size - 1);
		}
	}

	/// Emplace a new element at a specific position. @a T needs to be movable and default constructible.
	/// @param alloc The allocator.
	/// @param where Points to the position to emplace. Should be less or equal to what getEnd() returns.
	/// @param args  Constructor arguments.
	template<typename TAllocator, typename... TArgs>
	Iterator emplaceAt(TAllocator alloc, ConstIterator where, TArgs&&... args);

	/// Destroy the array.
	template<typename TAllocator>
	void destroy(TAllocator alloc)
	{
		if(m_data)
		{
			ANKI_ASSERT(m_size > 0);
			ANKI_ASSERT(m_capacity > 0);
			alloc.deleteArray(m_data, m_size);

			m_data = nullptr;
			m_size = 0;
			m_capacity = 0;
		}

		ANKI_ASSERT(m_data == nullptr && m_size == 0 && m_capacity == 0);
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

	/// Resizes the storage but DOESN'T CONSTRUCT ANY ELEMENTS. It only moves or destroys.
	template<typename TAllocator>
	void resizeStorage(TAllocator alloc, Size newSize);

protected:
	Value* m_data;
	Size m_size;
	Size m_capacity = 0;
};

/// Dynamic array with automatic destruction. It's the same as DynamicArray but it holds the allocator in order to
/// perform automatic destruction. Use it for temp operations and on transient classes.
template<typename T, typename TSize = U32>
class DynamicArrayAuto : public DynamicArray<T, TSize>
{
public:
	using Base = DynamicArray<T, TSize>;
	using Base::m_capacity;
	using Base::m_data;
	using Base::m_size;
	using typename Base::Value;
	using typename Base::Iterator;
	using typename Base::ConstIterator;
	using typename Base::Size;

	template<typename TAllocator>
	DynamicArrayAuto(TAllocator alloc)
		: Base()
		, m_alloc(alloc)
	{
	}

	/// And resize
	template<typename TAllocator>
	DynamicArrayAuto(TAllocator alloc, Size size)
		: Base()
		, m_alloc(alloc)
	{
		resize(size);
	}

	/// With default value
	template<typename TAllocator>
	DynamicArrayAuto(TAllocator alloc, Size size, const T& v)
		: Base()
		, m_alloc(alloc)
	{
		create(size, v);
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

	/// Copy.
	DynamicArrayAuto(const DynamicArrayAuto& b)
		: Base()
		, m_alloc(b.m_alloc)
	{
		if(b.getSize())
		{
			create(b.getSize());
			for(Size i = 0; i < b.getSize(); ++i)
			{
				(*this)[i] = b[i];
			}
		}
	}

	/// Move.
	DynamicArrayAuto& operator=(DynamicArrayAuto&& b)
	{
		Base::destroy(m_alloc);

		m_data = b.m_data;
		b.m_data = nullptr;
		m_size = b.m_size;
		b.m_size = 0;
		m_capacity = b.m_capacity;
		b.m_capacity = 0;
		m_alloc = b.m_alloc;
		b.m_alloc = {};
		return *this;
	}

	/// Copy.
	DynamicArrayAuto& operator=(const DynamicArrayAuto& b)
	{
		destroy();
		if(b.getSize())
		{
			create(b.getSize());
			for(Size i = 0; i < b.getSize(); ++i)
			{
				(*this)[i] = b[i];
			}
		}
		return *this;
	}

	/// @copydoc DynamicArray::create
	void create(Size size)
	{
		Base::create(m_alloc, size);
	}

	/// @copydoc DynamicArray::create
	void create(Size size, const Value& v)
	{
		Base::create(m_alloc, size, v);
	}

	/// @copydoc DynamicArray::destroy
	void destroy()
	{
		Base::destroy(m_alloc);
	}

	/// @copydoc DynamicArray::resize
	void resize(Size size, const Value& v)
	{
		Base::resize(m_alloc, size, v);
	}

	/// @copydoc DynamicArray::emplaceBack
	template<typename... TArgs>
	Iterator emplaceBack(TArgs&&... args)
	{
		return Base::emplaceBack(m_alloc, std::forward<TArgs>(args)...);
	}

	/// @copydoc DynamicArray::emplaceAt
	template<typename... TArgs>
	Iterator emplaceAt(ConstIterator where, TArgs&&... args)
	{
		return Base::emplaceAt(m_alloc, where, std::forward<TArgs>(args)...);
	}

	/// @copydoc DynamicArray::resize
	void resize(Size size)
	{
		Base::resize(m_alloc, size);
	}

	/// @copydoc DynamicArray::moveAndReset
	void moveAndReset(Value*& data, Size& size, Size& storageSize)
	{
		Base::moveAndReset(data, size, storageSize);
		// Don't touch the m_alloc
	}

	/// @copydoc DynamicArray::resizeStorage
	void resizeStorage(Size newSize)
	{
		Base::resizeStorage(m_alloc, newSize);
	}

	/// Get the allocator.
	const GenericMemoryPoolAllocator<T>& getAllocator() const
	{
		return m_alloc;
	}

private:
	GenericMemoryPoolAllocator<T> m_alloc;
};
/// @}

} // end namespace anki

#include <anki/util/DynamicArray.inl.h>

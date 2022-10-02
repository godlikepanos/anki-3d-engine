// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
template<typename T, typename TSize>
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

	/// Move DynamicArrayRaii to this.
	template<typename TMemPool>
	DynamicArray& operator=(DynamicArrayRaii<T, TSize, TMemPool>&& b);

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
	template<typename TMemPool>
	void create(TMemPool& pool, Size size)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0 && m_capacity == 0);
		if(size > 0)
		{
			m_data = newArray<Value>(pool, size);
			m_size = size;
			m_capacity = size;
		}
	}

	/// Only create the array. Useful if @a T is non-copyable or movable .
	template<typename TMemPool>
	void create(TMemPool& pool, Size size, const Value& v)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0 && m_capacity == 0);
		if(size > 0)
		{
			m_data = newArray<Value>(pool, size, v);
			m_size = size;
			m_capacity = size;
		}
	}

	/// Destroy the array.
	template<typename TMemPool>
	void destroy(TMemPool& pool)
	{
		if(m_data)
		{
			ANKI_ASSERT(m_size > 0);
			ANKI_ASSERT(m_capacity > 0);
			deleteArray(pool, m_data, m_size);

			m_data = nullptr;
			m_size = 0;
			m_capacity = 0;
		}

		ANKI_ASSERT(m_data == nullptr && m_size == 0 && m_capacity == 0);
	}

	/// Grow or create the array. @a T needs to be copyable and moveable.
	template<typename TMemPool>
	void resize(TMemPool& pool, Size size, const Value& v);

	/// Grow or create the array. @a T needs to be copyable, moveable and default constructible.
	template<typename TMemPool>
	void resize(TMemPool& pool, Size size);

	/// Push back value.
	template<typename TMemPool, typename... TArgs>
	Iterator emplaceBack(TMemPool& pool, TArgs&&... args)
	{
		resizeStorage(pool, m_size + 1);
		callConstructor(m_data[m_size], std::forward<TArgs>(args)...);
		++m_size;
		return &m_data[m_size - 1];
	}

	/// Remove the last value.
	template<typename TMemPool>
	void popBack(TMemPool& pool)
	{
		if(m_size > 0)
		{
			resizeStorage(pool, m_size - 1);
		}
	}

	/// Emplace a new element at a specific position. @a T needs to be movable and default constructible.
	/// @param pool The memory pool.
	/// @param where Points to the position to emplace. Should be less or equal to what getEnd() returns.
	/// @param args  Constructor arguments.
	template<typename TMemPool, typename... TArgs>
	Iterator emplaceAt(TMemPool& pool, ConstIterator where, TArgs&&... args);

	/// Removes the (first, last] elements.
	/// @param pool The memory pool.
	/// @param first Points to the position of the first element to remove.
	/// @param last Points to the position of the last element to remove minus one.
	template<typename TMemPool>
	void erase(TMemPool& pool, ConstIterator first, ConstIterator last);

	/// Removes one element.
	/// @param pool The memory pool.
	/// @param at Points to the position of the element to remove.
	template<typename TMemPool>
	void erase(TMemPool& pool, ConstIterator at)
	{
		erase(pool, at, at + 1);
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
	template<typename TMemPool>
	void resizeStorage(TMemPool& pool, Size newSize);

protected:
	Value* m_data;
	Size m_size;
	Size m_capacity = 0;
};

/// Dynamic array with automatic destruction. It's the same as DynamicArray but it holds the memory pool in order to
/// perform automatic destruction. Use it for temp operations and on transient classes.
template<typename T, typename TSize = U32, typename TMemPool = MemoryPoolPtrWrapper<BaseMemoryPool>>
class DynamicArrayRaii : public DynamicArray<T, TSize>
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
	using MemoryPool = TMemPool;

	DynamicArrayRaii(const MemoryPool& pool)
		: Base()
		, m_pool(pool)
	{
	}

	/// And resize
	DynamicArrayRaii(const MemoryPool& pool, Size size)
		: Base()
		, m_pool(pool)
	{
		resize(size);
	}

	/// With default value
	DynamicArrayRaii(const MemoryPool& pool, Size size, const T& v)
		: Base()
		, m_pool(pool)
	{
		create(size, v);
	}

	/// Move.
	DynamicArrayRaii(DynamicArrayRaii&& b)
		: Base()
	{
		*this = std::move(b);
	}

	~DynamicArrayRaii()
	{
		Base::destroy(m_pool);
	}

	/// Copy.
	DynamicArrayRaii(const DynamicArrayRaii& b)
		: Base()
		, m_pool(b.m_pool)
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
	DynamicArrayRaii& operator=(DynamicArrayRaii&& b)
	{
		Base::destroy(m_pool);

		m_data = b.m_data;
		b.m_data = nullptr;
		m_size = b.m_size;
		b.m_size = 0;
		m_capacity = b.m_capacity;
		b.m_capacity = 0;
		m_pool = std::move(b.m_pool);
		b.m_pool = {};
		return *this;
	}

	/// Copy.
	DynamicArrayRaii& operator=(const DynamicArrayRaii& b)
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
		Base::create(m_pool, size);
	}

	/// @copydoc DynamicArray::create
	void create(Size size, const Value& v)
	{
		Base::create(m_pool, size, v);
	}

	/// @copydoc DynamicArray::destroy
	void destroy()
	{
		Base::destroy(m_pool);
	}

	/// @copydoc DynamicArray::resize
	void resize(Size size, const Value& v)
	{
		Base::resize(m_pool, size, v);
	}

	/// @copydoc DynamicArray::resize
	void resize(Size size)
	{
		Base::resize(m_pool, size);
	}

	/// @copydoc DynamicArray::emplaceBack
	template<typename... TArgs>
	Iterator emplaceBack(TArgs&&... args)
	{
		return Base::emplaceBack(m_pool, std::forward<TArgs>(args)...);
	}

	/// @copydoc DynamicArray::popBack
	void popBack()
	{
		Base::popBack(m_pool);
	}

	/// @copydoc DynamicArray::emplaceAt
	template<typename... TArgs>
	Iterator emplaceAt(ConstIterator where, TArgs&&... args)
	{
		return Base::emplaceAt(m_pool, where, std::forward<TArgs>(args)...);
	}

	/// @copydoc DynamicArray::erase
	void erase(ConstIterator first, ConstIterator last)
	{
		return Base::erase(m_pool, first, last);
	}

	/// @copydoc DynamicArray::erase
	void erase(ConstIterator at)
	{
		return Base::erase(m_pool, at);
	}

	/// @copydoc DynamicArray::moveAndReset
	void moveAndReset(Value*& data, Size& size, Size& storageSize)
	{
		Base::moveAndReset(data, size, storageSize);
		// Don't touch the m_pool
	}

	/// @copydoc DynamicArray::moveAndReset
	void moveAndReset(WeakArray<Value, Size>& array)
	{
		Base::moveAndReset(array);
		// Don't touch the m_pool
	}

	/// @copydoc DynamicArray::resizeStorage
	void resizeStorage(Size newSize)
	{
		Base::resizeStorage(m_pool, newSize);
	}

	/// Get the pool.
	const MemoryPool& getMemoryPool() const
	{
		return m_pool;
	}

	/// Get the pool.
	MemoryPool& getMemoryPool()
	{
		return m_pool;
	}

private:
	MemoryPool m_pool;
};
/// @}

} // end namespace anki

#include <AnKi/Util/DynamicArray.inl.h>

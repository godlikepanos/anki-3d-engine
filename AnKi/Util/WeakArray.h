// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Array.h>
#include <AnKi/Util/Forward.h>

namespace anki {

/// @addtogroup util_containers
/// @{

/// Array that doesn't own the memory.
template<typename T, typename TSize = U32>
class WeakArray
{
public:
	using Value = T;
	using Iterator = Value*;
	using ConstIterator = const Value*;
	using Reference = Value&;
	using ConstReference = const Value&;
	using Size = TSize;

	WeakArray(T* mem, Size size)
		: m_data(mem)
		, m_size(size)
	{
		if(size)
		{
			ANKI_ASSERT(mem);
		}
	}

	WeakArray(T* begin, T* end)
		: m_data(begin)
		, m_size(end - begin)
	{
		ANKI_ASSERT(begin && end);
		ANKI_ASSERT(end >= begin);
	}

	WeakArray()
		: WeakArray(nullptr, Size(0))
	{
	}

	template<PtrSize kSize>
	WeakArray(Array<T, kSize>& arr)
		: WeakArray(&arr[0], arr.getSize())
	{
	}

	template<typename TMemoryPool>
	explicit WeakArray(DynamicArray<T, TMemoryPool, TSize>& arr)
		: WeakArray()
	{
		if(arr.getSize())
		{
			m_data = &arr[0];
			m_size = arr.getSize();
		}
	}

	/// Copy.
	WeakArray(const WeakArray& b)
		: WeakArray(b.m_data, b.m_size)
	{
	}

	/// Move.
	WeakArray(WeakArray&& b)
		: WeakArray()
	{
		*this = std::move(b);
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

	template<PtrSize kSize>
	WeakArray& operator=(Array<T, kSize>& arr)
	{
		m_data = &arr[0];
		m_size = arr.getSize();
		return *this;
	}

	template<typename TMemoryPool>
	WeakArray& operator=(DynamicArray<T, TMemoryPool, TSize>& arr)
	{
		m_data = (arr.getSize()) ? &arr[0] : nullptr;
		m_size = arr.getSize();
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

	/// Set the array pointer and its size.
	void setArray(Value* array, Size size)
	{
		ANKI_ASSERT((array && size > 0) || (array == nullptr && size == 0));
		m_data = array;
		m_size = size;
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

	/// Fill the array.
	static void fill(Iterator begin, Iterator end, const T& val)
	{
		while(begin != end)
		{
			*begin = val;
			++begin;
		}
	}

	void fill(const T& val)
	{
		auto begin = getBegin();
		auto end = getEnd();
		while(begin != end)
		{
			*begin = val;
			++begin;
		}
	}

	WeakArray subrange(Size offset, Size range) const
	{
		ANKI_ASSERT(offset < m_size);
		ANKI_ASSERT(offset + range <= m_size);
		WeakArray out(m_data + offset, range);
		return out;
	}

private:
	Value* m_data;
	Size m_size;
};

/// Array that doesn't own the memory.
template<typename T, typename TSize = U32>
class ConstWeakArray
{
public:
	using Value = T;
	using ConstIterator = const Value*;
	using ConstReference = const Value&;
	using Size = TSize;

	ConstWeakArray(const T* mem, Size size)
		: m_data(mem)
		, m_size(size)
	{
		if(size)
		{
			ANKI_ASSERT(mem);
		}
	}

	ConstWeakArray(T* begin, T* end)
		: m_data(begin)
		, m_size(end - begin)
	{
		ANKI_ASSERT(begin && end);
		ANKI_ASSERT(end >= begin);
	}

	ConstWeakArray()
		: ConstWeakArray(nullptr, Size(0))
	{
	}

	/// Construct from WeakArray.
	ConstWeakArray(const WeakArray<T, TSize>& arr)
		: ConstWeakArray((arr.getSize()) ? &arr[0] : nullptr, arr.getSize())
	{
	}

	/// Construct from Array.
	template<PtrSize kSize>
	ConstWeakArray(const Array<T, kSize>& arr)
		: ConstWeakArray(&arr[0], arr.getSize())
	{
	}

	/// Construct from DynamicArray.
	template<typename TMemoryPool>
	ConstWeakArray(const DynamicArray<T, TMemoryPool, TSize>& arr)
		: ConstWeakArray()
	{
		if(arr.getSize())
		{
			m_data = &arr[0];
			m_size = arr.getSize();
		}
	}

	/// Copy.
	ConstWeakArray(const ConstWeakArray& b)
		: ConstWeakArray(b.m_data, b.m_size)
	{
	}

	/// Move.
	ConstWeakArray(ConstWeakArray&& b)
		: ConstWeakArray()
	{
		*this = std::move(b);
	}

	/// Copy.
	ConstWeakArray& operator=(const ConstWeakArray& b)
	{
		m_data = b.m_data;
		m_size = b.m_size;
		return *this;
	}

	/// Copy from a WeakArray.
	template<typename Y>
	ConstWeakArray& operator=(const WeakArray<Y, TSize>& b)
	{
		m_data = (b.getSize()) ? b.getBegin() : nullptr;
		m_size = b.getSize();
		return *this;
	}

	/// Move.
	ConstWeakArray& operator=(ConstWeakArray&& b)
	{
		m_data = b.m_data;
		b.m_data = nullptr;
		m_size = b.m_size;
		b.m_size = 0;
		return *this;
	}

	template<PtrSize kSize>
	ConstWeakArray& operator=(const Array<T, kSize>& arr)
	{
		m_data = &arr[0];
		m_size = arr.getSize();
		return *this;
	}

	template<typename TMemoryPool>
	ConstWeakArray& operator=(const DynamicArray<T, TMemoryPool, TSize>& arr)
	{
		m_data = (arr.getSize()) ? &arr[0] : nullptr;
		m_size = arr.getSize();
		return *this;
	}

	ConstReference operator[](const Size n) const
	{
		ANKI_ASSERT(n < m_size);
		return m_data[n];
	}

	ConstIterator getBegin() const
	{
		return m_data;
	}

	ConstIterator getEnd() const
	{
		return m_data + m_size;
	}

	/// Make it compatible with the C++11 range based for loop.
	ConstIterator begin() const
	{
		return getBegin();
	}

	/// Make it compatible with the C++11 range based for loop.
	ConstIterator end() const
	{
		return getEnd();
	}

	/// Get first element.
	ConstReference getFront() const
	{
		ANKI_ASSERT(!isEmpty());
		return m_data[0];
	}

	/// Get last element.
	ConstReference getBack() const
	{
		ANKI_ASSERT(!isEmpty());
		return m_data[m_size - 1];
	}

	/// Set the array pointer and its size.
	void setArray(Value* array, Size size)
	{
		ANKI_ASSERT((array && size > 0) || (array == nullptr && size == 0));
		m_data = array;
		m_size = size;
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

	ConstWeakArray subrange(Size offset, Size range) const
	{
		ANKI_ASSERT(offset < m_size);
		ANKI_ASSERT(offset + range <= m_size);
		ConstWeakArray out(m_data + offset, range);
		return out;
	}

private:
	const Value* m_data;
	Size m_size;
};
/// @}

} // end namespace anki

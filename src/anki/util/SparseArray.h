// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/Assert.h>
#include <anki/util/Array.h>
#include <anki/util/Allocator.h>
#include <utility>

namespace anki
{

/// @addtogroup util_containers
/// @{

/// Sparse array iterator.
template<typename TValuePointer, typename TValueReference, typename TSparseArrayPtr>
class SparseArrayIterator
{
	template<typename, typename>
	friend class SparseArray;

public:
	/// Default constructor.
	SparseArrayIterator()
		: m_array(nullptr)
		, m_elementIdx(MAX_U32)
	{
	}

	/// Copy.
	SparseArrayIterator(const SparseArrayIterator& b)
		: m_array(b.m_array)
		, m_elementIdx(b.m_elementIdx)
	{
	}

	/// Allow conversion from iterator to const iterator.
	template<typename YValuePointer, typename YValueReference, typename YSparseArrayPtr>
	SparseArrayIterator(const SparseArrayIterator<YValuePointer, YValueReference, YSparseArrayPtr>& b)
		: m_array(b.m_array)
		, m_elementIdx(b.m_elementIdx)
	{
	}

	SparseArrayIterator(TSparseArrayPtr arr, U32 modIdx)
		: m_array(arr)
		, m_elementIdx(modIdx)
	{
		ANKI_ASSERT(arr);
	}

	TValueReference operator*() const
	{
		check();
		return m_array->m_elements[m_elementIdx].m_value;
	}

	TValuePointer operator->() const
	{
		check();
		return &m_array->m_elements[m_elementIdx].m_value;
	}

	SparseArrayIterator& operator++()
	{
		check();
		++m_elementIdx;
		if(m_elementIdx >= m_array->m_capacity)
		{
			m_elementIdx = MAX_U32;
		}
		return *this;
	}

	SparseArrayIterator operator++(int)
	{
		check();
		SparseArrayIterator out = *this;
		++(*this);
		return out;
	}

	SparseArrayIterator operator+(U n) const
	{
		check();
		U32 pos = m_elementIdx + n;
		if(pos >= m_array->m_capacity)
		{
			pos = MAX_U32;
		}
		return SparseArrayIterator(m_array, pos);
	}

	SparseArrayIterator& operator+=(U n)
	{
		check();
		m_elementIdx += n;
		if(m_elementIdx >= m_array->m_capacity)
		{
			m_elementIdx = MAX_U32;
		}
		return *this;
	}

	Bool operator==(const SparseArrayIterator& b) const
	{
		ANKI_ASSERT(m_array == b.m_array);
		return m_elementIdx == b.m_elementIdx;
	}

	Bool operator!=(const SparseArrayIterator& b) const
	{
		return !(*this == b);
	}

	operator Bool() const
	{
		return m_elementIdx != MAX_U32;
	}

private:
	TSparseArrayPtr m_array;
	U32 m_elementIdx;

	void check() const
	{
		ANKI_ASSERT(m_elementIdx != MAX_U32 && m_array);
		ANKI_ASSERT(m_array->m_elements[m_elementIdx].m_alive);
	}
};

/// Sparse array.
/// @tparam T The type of the valut it will hold.
template<typename T, typename TIndex = U32>
class SparseArray
{
	template<typename, typename, typename>
	friend class SparseArrayIterator;

public:
	// Typedefs
	using Value = T;
	using Iterator = SparseArrayIterator<T*, T&, SparseArray*>;
	using ConstIterator = SparseArrayIterator<const T*, const T&, const SparseArray*>;
	using Index = TIndex;

	/// Constructor #1.
	SparseArray(U32 initialStorageSize, U32 probeCount, F32 maxLoadFactor)
		: m_initialStorageSize(initialStorageSize)
		, m_probeCount(probeCount)
		, m_maxLoadFactor(maxLoadFactor)
	{
		ANKI_ASSERT(initialStorageSize > 0 && isPowerOfTwo(initialStorageSize));
		ANKI_ASSERT(probeCount > 0 && probeCount < initialStorageSize);
		ANKI_ASSERT(maxLoadFactor > 0.0f && maxLoadFactor <= 1.0f);
	}

	/// Constructor #2.
	SparseArray(U32 initialStorageSize, U32 probeCount)
		: SparseArray(initialStorageSize, probeCount, 0.8f)
	{
	}

	/// Constructor #3.
	SparseArray(U32 initialStorageSize)
		: SparseArray(initialStorageSize, log2(initialStorageSize))
	{
	}

	/// Constructor #4.
	SparseArray()
		: SparseArray(64)
	{
	}

	/// Non-copyable.
	SparseArray(const SparseArray&) = delete;

	/// Move constructor.
	SparseArray(SparseArray&& b)
	{
		*this = std::move(b);
	}

	/// Destroy.
	~SparseArray()
	{
		ANKI_ASSERT(m_elements == nullptr && "Forgot to call destroy");
	}

	/// Non-copyable.
	SparseArray& operator=(const SparseArray&) = delete;

	/// Move operator.
	SparseArray& operator=(SparseArray&& b)
	{
		ANKI_ASSERT(m_elements == nullptr && "Forgot to call destroy");

		m_elements = b.m_elements;
		m_elementCount = b.m_elementCount;
		m_capacity = b.m_capacity;
		m_initialStorageSize = b.m_initialStorageSize;
		m_probeCount = b.m_probeCount;
		m_maxLoadFactor = b.m_maxLoadFactor;

		b.resetMembers();

		return *this;
	}

	/// Get begin.
	Iterator getBegin()
	{
		return Iterator(this, findFirstAlive());
	}

	/// Get begin.
	ConstIterator getBegin() const
	{
		return ConstIterator(this, findFirstAlive());
	}

	/// Get end.
	Iterator getEnd()
	{
		return Iterator();
	}

	/// Get end.
	ConstIterator getEnd() const
	{
		return ConstIterator();
	}

	/// Get begin.
	Iterator begin()
	{
		return getBegin();
	}

	/// Get begin.
	ConstIterator begin() const
	{
		return getBegin();
	}

	/// Get end.
	Iterator end()
	{
		return getEnd();
	}

	/// Get end.
	ConstIterator end() const
	{
		return getEnd();
	}

	/// Get the number of elements in the array.
	PtrSize getSize() const
	{
		return m_elementCount;
	}

	/// Return true if it's empty and false otherwise.
	Bool isEmpty() const
	{
		return m_elementCount != 0;
	}

	/// Destroy the array and free its elements.
	template<typename TAlloc>
	void destroy(TAlloc& alloc);

	/// Set a value to an index.
	template<typename TAlloc, typename... TArgs>
	void emplace(TAlloc& alloc, Index idx, TArgs&&... args);

	/// Get an iterator.
	Iterator find(Index idx)
	{
		return Iterator(this, findInternal(idx));
	}

	/// Get an iterator.
	ConstIterator find(Index idx) const
	{
		return ConstIterator(this, findInternal(idx));
	}

	/// Remove an element.
	template<typename TAlloc>
	void erase(TAlloc& alloc, Iterator it);

	/// Check the validity of the array.
	void validate() const;

protected:
	/// Element.
	class Element
	{
	public:
		Value m_value;
		Index m_idx;
		Bool8 m_alive;

		Element() = delete;
		Element(const Element&) = delete;
		Element(Element&&) = delete;
		~Element() = delete;
	};

	Element* m_elements = nullptr;
	U32 m_elementCount = 0;
	U32 m_capacity = 0;

	U32 m_initialStorageSize = 0;
	U32 m_probeCount = 0;
	F32 m_maxLoadFactor = 0.0;

	Index mod(const Index idx) const
	{
		ANKI_ASSERT(m_capacity > 0);
		ANKI_ASSERT(isPowerOfTwo(m_capacity));
		return idx & (m_capacity - 1);
	}

	static Index mod(const Index idx, U32 capacity)
	{
		ANKI_ASSERT(capacity > 0);
		ANKI_ASSERT(isPowerOfTwo(capacity));
		return idx & (capacity - 1);
	}

	F32 calcLoadFactor() const
	{
		ANKI_ASSERT(m_elementCount <= m_capacity);
		ANKI_ASSERT(m_capacity > 0);
		return F32(m_elementCount) / m_capacity;
	}

	/// Insert a value.
	/// @return One if the idx was a new element or zero if the idx was there already.
	template<typename TAlloc>
	U32 insert(TAlloc& alloc, Index idx, Value& val);

	/// Grow the storage and re-insert.
	template<typename TAlloc>
	void grow(TAlloc& alloc);

	/// Compute the distance between a desired position and the current one. This method does a trick with capacity to
	/// account for wrapped positions.
	Index distanceFromDesired(const Index crntPos, const Index desiredPos) const
	{
		return mod(crntPos + m_capacity - desiredPos);
	}

	Index findFirstAlive() const
	{
		if(m_elementCount == 0)
		{
			return MAX_U32;
		}

		for(Index i = 0; i < m_capacity; ++i)
		{
			if(m_elements[i].m_alive)
			{
				return i;
			}
		}

		ANKI_ASSERT(0);
		return MAX_U32;
	}

	/// Find an element and return its position inside m_elements.
	Index findInternal(Index idx) const;

	void resetMembers()
	{
		m_elements = nullptr;
		m_elementCount = 0;
		m_capacity = 0;
		m_initialStorageSize = 0;
		m_probeCount = 0;
		m_maxLoadFactor = 0;
	}
};
/// @}

} // end namespace anki

#include <anki/util/SparseArray.inl.h>

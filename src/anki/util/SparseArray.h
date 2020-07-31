// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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

private:
	using Index = typename RemovePointer<TSparseArrayPtr>::Type::Index;

public:
	/// Default constructor.
	SparseArrayIterator()
		: m_array(nullptr)
		, m_elementIdx(getMaxNumericLimit<Index>())
#if ANKI_EXTRA_CHECKS
		, m_iteratorVer(MAX_U32)
#endif
	{
	}

	/// Copy.
	SparseArrayIterator(const SparseArrayIterator& b)
		: m_array(b.m_array)
		, m_elementIdx(b.m_elementIdx)
#if ANKI_EXTRA_CHECKS
		, m_iteratorVer(b.m_iteratorVer)
#endif
	{
	}

	/// Allow conversion from iterator to const iterator.
	template<typename YValuePointer, typename YValueReference, typename YSparseArrayPtr>
	SparseArrayIterator(const SparseArrayIterator<YValuePointer, YValueReference, YSparseArrayPtr>& b)
		: m_array(b.m_array)
		, m_elementIdx(b.m_elementIdx)
#if ANKI_EXTRA_CHECKS
		, m_iteratorVer(b.m_iteratorVer)
#endif
	{
	}

	SparseArrayIterator(TSparseArrayPtr arr, Index modIdx
#if ANKI_EXTRA_CHECKS
						,
						U32 ver
#endif
						)
		: m_array(arr)
		, m_elementIdx(modIdx)
#if ANKI_EXTRA_CHECKS
		, m_iteratorVer(ver)
#endif
	{
		ANKI_ASSERT(arr);
	}

	SparseArrayIterator& operator=(const SparseArrayIterator& b)
	{
		m_array = b.m_array;
		m_elementIdx = b.m_elementIdx;
#if ANKI_EXTRA_CHECKS
		m_iteratorVer = b.m_iteratorVer;
#endif
		return *this;
	}

	TValueReference operator*() const
	{
		check();
		return m_array->m_elements[m_elementIdx];
	}

	TValuePointer operator->() const
	{
		check();
		return &m_array->m_elements[m_elementIdx];
	}

	SparseArrayIterator& operator++()
	{
		check();
		m_elementIdx = m_array->iterate(m_elementIdx, 1);
		return *this;
	}

	SparseArrayIterator operator++(int)
	{
		check();
		SparseArrayIterator out = *this;
		++(*this);
		return out;
	}

	SparseArrayIterator operator+(Index n) const
	{
		check();
		Index pos = m_array->iterate(m_elementIdx, n);
		return SparseArrayIterator(m_array, pos);
	}

	SparseArrayIterator& operator+=(Index n)
	{
		check();
		m_elementIdx = m_array->iterate(m_elementIdx, n);
		return *this;
	}

	Bool operator==(const SparseArrayIterator& b) const
	{
		ANKI_ASSERT(m_array == b.m_array);
		ANKI_ASSERT(m_iteratorVer == b.m_iteratorVer);
		return m_elementIdx == b.m_elementIdx;
	}

	Bool operator!=(const SparseArrayIterator& b) const
	{
		return !(*this == b);
	}

	U32 getKey() const
	{
		check();
		return m_elementIdx;
	}

private:
	TSparseArrayPtr m_array;
	Index m_elementIdx;
#if ANKI_EXTRA_CHECKS
	U32 m_iteratorVer; ///< See SparseArray::m_iteratorVer.
#endif

	void check() const
	{
		ANKI_ASSERT(m_array);
		ANKI_ASSERT(m_elementIdx != getMaxNumericLimit<Index>());
		ANKI_ASSERT(m_array->m_metadata[m_elementIdx].m_alive);
		ANKI_ASSERT(m_array->m_iteratorVer == m_iteratorVer);
	}
};

/// Sparse array.
/// @tparam T The type of the valut it will hold.
/// @tparam TIndex Indicates the max size of the sparse indices it can accept. Can be U32 or U64.
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

	// Consts
	static constexpr Index INITIAL_STORAGE_SIZE = 64; ///< The initial storage size of the array.
	static constexpr U32 LINEAR_PROBING_COUNT = 8; ///< The number of linear probes.
	static constexpr F32 MAX_LOAD_FACTOR = 0.8f; ///< Load factor.

	/// Constructor.
	/// @param initialStorageSize The initial size of the array.
	/// @param probeCount         The number of probe queries. It's the linear probe count the sparse array is using.
	/// @param maxLoadFactor      If storage is loaded more than maxLoadFactor then increase it.
	SparseArray(Index initialStorageSize = INITIAL_STORAGE_SIZE, U32 probeCount = LINEAR_PROBING_COUNT,
				F32 maxLoadFactor = MAX_LOAD_FACTOR)
		: m_initialStorageSize(initialStorageSize)
		, m_probeCount(probeCount)
		, m_maxLoadFactor(maxLoadFactor)
	{
		ANKI_ASSERT(initialStorageSize > 0 && isPowerOfTwo(initialStorageSize));
		ANKI_ASSERT(probeCount > 0 && probeCount < initialStorageSize);
		ANKI_ASSERT(maxLoadFactor > 0.5f && maxLoadFactor < 1.0f);
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
		ANKI_ASSERT(m_elements == nullptr && m_metadata == nullptr && "Forgot to call destroy");
	}

	/// Non-copyable.
	SparseArray& operator=(const SparseArray&) = delete;

	/// Move operator.
	SparseArray& operator=(SparseArray&& b)
	{
		ANKI_ASSERT(m_elements == nullptr && m_metadata == nullptr && "Forgot to call destroy");

		m_elements = b.m_elements;
		m_metadata = b.m_metadata;
		m_elementCount = b.m_elementCount;
		m_capacity = b.m_capacity;
		m_initialStorageSize = b.m_initialStorageSize;
		m_probeCount = b.m_probeCount;
		m_maxLoadFactor = b.m_maxLoadFactor;
#if ANKI_EXTRA_CHECKS
		++m_iteratorVer;
#endif

		b.resetMembers();

		return *this;
	}

	/// Get begin.
	Iterator getBegin()
	{
		return Iterator(this, findFirstAlive()
#if ANKI_EXTRA_CHECKS
								  ,
						m_iteratorVer
#endif
		);
	}

	/// Get begin.
	ConstIterator getBegin() const
	{
		return ConstIterator(this, findFirstAlive()
#if ANKI_EXTRA_CHECKS
									   ,
							 m_iteratorVer
#endif
		);
	}

	/// Get end.
	Iterator getEnd()
	{
		return Iterator(this, getMaxNumericLimit<Index>()
#if ANKI_EXTRA_CHECKS
								  ,
						m_iteratorVer
#endif
		);
	}

	/// Get end.
	ConstIterator getEnd() const
	{
		return ConstIterator(this, getMaxNumericLimit<Index>()
#if ANKI_EXTRA_CHECKS
									   ,
							 m_iteratorVer
#endif
		);
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
		return m_elementCount == 0;
	}

	/// Destroy the array and free its elements.
	template<typename TAlloc>
	void destroy(TAlloc& alloc);

	/// Set a value to an index.
	template<typename TAlloc, typename... TArgs>
	Iterator emplace(TAlloc& alloc, Index idx, TArgs&&... args);

	/// Get an iterator.
	Iterator find(Index idx)
	{
		return Iterator(this, findInternal(idx)
#if ANKI_EXTRA_CHECKS
								  ,
						m_iteratorVer
#endif
		);
	}

	/// Get an iterator.
	ConstIterator find(Index idx) const
	{
		return ConstIterator(this, findInternal(idx)
#if ANKI_EXTRA_CHECKS
									   ,
							 m_iteratorVer
#endif
		);
	}

	/// Remove an element.
	template<typename TAlloc>
	void erase(TAlloc& alloc, Iterator it);

	/// Check the validity of the array.
	void validate() const;

	/// Create a copy of this.
	template<typename TAlloc>
	void clone(TAlloc& alloc, SparseArray& b) const;

protected:
	/// Element metadata.
	class Metadata
	{
	public:
		Index m_idx;
		Bool m_alive;
	};

	Value* m_elements = nullptr;
	Metadata* m_metadata = nullptr;
	Index m_elementCount = 0;
	Index m_capacity = 0;

	Index m_initialStorageSize = 0;
	U32 m_probeCount = 0;
	F32 m_maxLoadFactor = 0.0f;
#if ANKI_EXTRA_CHECKS
	/// Iterators version. Used to check if iterators point to the newest storage. Needs to be changed whenever we need
	/// to invalidate iterators.
	U32 m_iteratorVer = 0;
#endif

	/// Wrap an index.
	Index mod(const Index idx) const
	{
		ANKI_ASSERT(m_capacity > 0);
		ANKI_ASSERT(isPowerOfTwo(m_capacity));
		return idx & (m_capacity - 1);
	}

	/// Wrap an index.
	static Index mod(const Index idx, Index capacity)
	{
		ANKI_ASSERT(capacity > 0);
		ANKI_ASSERT(isPowerOfTwo(capacity));
		return idx & (capacity - 1);
	}

	F32 calcLoadFactor() const
	{
		ANKI_ASSERT(m_elementCount <= m_capacity);
		ANKI_ASSERT(m_capacity > 0);
		return F32(m_elementCount) / F32(m_capacity);
	}

	/// Insert a value. This method will move the val to a new place.
	/// @return One if the idx was a new element or zero if the idx was there already.
	template<typename TAlloc>
	Index insert(TAlloc& alloc, Index idx, Value& val);

	/// Grow the storage and re-insert.
	template<typename TAlloc>
	void grow(TAlloc& alloc);

	/// Compute the distance between a desired position and the current one. This method does a trick with capacity to
	/// account for wrapped positions.
	Index distanceFromDesired(const Index crntPos, const Index desiredPos) const
	{
		return mod(crntPos + m_capacity - desiredPos);
	}

	/// Find the first alive element.
	Index findFirstAlive() const
	{
		if(m_elementCount == 0)
		{
			return getMaxNumericLimit<Index>();
		}

		for(Index i = 0; i < m_capacity; ++i)
		{
			if(m_metadata[i].m_alive)
			{
				return i;
			}
		}

		ANKI_ASSERT(0);
		return getMaxNumericLimit<Index>();
	}

	/// Find an element and return its position inside m_elements.
	Index findInternal(Index idx) const;

	/// Reset the class.
	void resetMembers()
	{
		m_elements = nullptr;
		m_metadata = nullptr;
		m_elementCount = 0;
		m_capacity = 0;
		invalidateIterators();
	}

	/// Iterate a number of elements.
	Index iterate(Index pos, Index n) const
	{
		ANKI_ASSERT(pos < m_capacity);
		ANKI_ASSERT(n > 0);
		ANKI_ASSERT(m_metadata[pos].m_alive);

		while(n > 0 && ++pos < m_capacity)
		{
			ANKI_ASSERT(m_metadata[pos].m_alive == 1 || m_metadata[pos].m_alive == 0);
			n -= Index(m_metadata[pos].m_alive);
		}

		return (pos >= m_capacity) ? getMaxNumericLimit<Index>() : pos;
	}

	template<typename TAlloc, typename... TArgs>
	void emplaceInternal(TAlloc& alloc, Index idx, TArgs&&... args);

	void destroyElement(Value& v)
	{
		v.~Value();
#if ANKI_EXTRA_CHECKS
		memset(&v, 0xC, sizeof(v));
#endif
	}

	void invalidateIterators()
	{
#if ANKI_EXTRA_CHECKS
		++m_iteratorVer;
#endif
	}
};
/// @}

} // end namespace anki

#include <anki/util/SparseArray.inl.h>

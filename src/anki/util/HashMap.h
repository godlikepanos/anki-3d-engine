// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Allocator.h>
#include <anki/util/Functions.h>
#include <anki/util/NonCopyable.h>
#include <anki/util/SparseArray.h>

namespace anki
{

/// @addtogroup util_containers
/// @{

/// Default hasher.
template<typename TKey>
class DefaultHasher
{
public:
	U64 operator()(const TKey& a) const
	{
		return a.computeHash();
	}
};

/// Specialization for U64 keys.
template<>
class DefaultHasher<U64>
{
public:
	U64 operator()(const U64 a) const
	{
		return a;
	}
};

/// Specialization for I64 keys.
template<>
class DefaultHasher<I64>
{
public:
	U64 operator()(const I64 a) const
	{
		union
		{
			U64 u64;
			I64 i64;
		};
		i64 = a;
		return u64;
	}
};

/// Hash map template.
template<typename TKey, typename TValue, typename THasher = DefaultHasher<TKey>>
class HashMap
{
public:
	// Typedefs
	using SparseArrayType = SparseArray<TValue, U64>;
	using Value = TValue;
	using Key = TKey;
	using Hasher = THasher;
	using Iterator = typename SparseArrayType::Iterator;
	using ConstIterator = typename SparseArrayType::ConstIterator;

	// Consts
	/// @see SparseArray::INITIAL_STORAGE_SIZE
	static constexpr U32 INITIAL_STORAGE_SIZE = SparseArrayType::INITIAL_STORAGE_SIZE;
	/// @see SparseArray::LINEAR_PROBING_COUNT
	static constexpr U32 LINEAR_PROBING_COUNT = SparseArrayType::LINEAR_PROBING_COUNT;
	/// @see SparseArray::MAX_LOAD_FACTOR
	static constexpr F32 MAX_LOAD_FACTOR = SparseArrayType::MAX_LOAD_FACTOR;

	/// Default constructor.
	/// @copy doc SparseArray::SparseArray
	HashMap(U32 initialStorageSize = INITIAL_STORAGE_SIZE,
		U32 probeCount = LINEAR_PROBING_COUNT,
		F32 maxLoadFactor = MAX_LOAD_FACTOR)
		: m_sparseArr(initialStorageSize, probeCount, maxLoadFactor)
	{
	}

	/// Move.
	HashMap(HashMap&& b)
	{
		*this = std::move(b);
	}

	/// You need to manually destroy the map.
	/// @see HashMap::destroy
	~HashMap()
	{
	}

	/// Move.
	HashMap& operator=(HashMap&& b)
	{
		m_sparseArr = std::move(b.m_sparseArr);
		return *this;
	}

	/// Get begin.
	Iterator getBegin()
	{
		return m_sparseArr.getBegin();
	}

	/// Get begin.
	ConstIterator getBegin() const
	{
		return m_sparseArr.getBegin();
	}

	/// Get end.
	Iterator getEnd()
	{
		return m_sparseArr.getEnd();
	}

	/// Get end.
	ConstIterator getEnd() const
	{
		return m_sparseArr.getEnd();
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

	/// Return true if map is empty.
	Bool isEmpty() const
	{
		return m_sparseArr.isEmpty();
	}

	/// Destroy the list.
	template<typename TAllocator>
	void destroy(TAllocator alloc)
	{
		m_sparseArr.destroy(alloc);
	}

	/// Construct an element inside the map.
	template<typename TAllocator, typename... TArgs>
	Iterator emplace(TAllocator alloc, const TKey& key, TArgs&&... args)
	{
		const U64 hash = THasher()(key);
		return m_sparseArr.emplace(alloc, hash, std::forward<TArgs>(args)...);
	}

	/// Erase element.
	template<typename TAllocator>
	void erase(TAllocator alloc, Iterator it)
	{
		m_sparseArr.erase(alloc, it);
	}

	/// Find a value using a key.
	Iterator find(const Key& key)
	{
		const U64 hash = THasher()(key);
		return m_sparseArr.find(hash);
	}

	/// Find a value using a key.
	ConstIterator find(const Key& key) const
	{
		const U64 hash = THasher()(key);
		return m_sparseArr.find(hash);
	}

private:
	SparseArrayType m_sparseArr;
};

/// Hash map template with automatic cleanup.
template<typename TKey, typename TValue, typename THasher = DefaultHasher<TKey>>
class HashMapAuto : public HashMap<TKey, TValue, THasher>
{
public:
	using Base = HashMap<TKey, TValue, THasher>;

	/// Default constructor.
	/// @copy doc SparseArray::SparseArray
	HashMapAuto(const GenericMemoryPoolAllocator<U8>& alloc,
		U32 initialStorageSize = Base::INITIAL_STORAGE_SIZE,
		U32 probeCount = Base::LINEAR_PROBING_COUNT,
		F32 maxLoadFactor = Base::MAX_LOAD_FACTOR)
		: Base(initialStorageSize, probeCount, maxLoadFactor)
		, m_alloc(alloc)
	{
	}

	/// Move.
	HashMapAuto(HashMapAuto&& b)
	{
		*this = std::move(b);
	}

	/// Destructor.
	~HashMapAuto()
	{
		Base::destroy(m_alloc);
	}

	/// Move.
	HashMapAuto& operator=(HashMapAuto&& b)
	{
		std::move(*static_cast<HashMapAuto>(this));
		m_alloc = std::move(b.m_alloc);
		return *this;
	}

	/// Construct an element inside the map.
	template<typename... TArgs>
	typename Base::Iterator emplace(const TKey& key, TArgs&&... args)
	{
		return Base::emplace(m_alloc, std::forward(args)...);
	}

	/// Erase element.
	void erase(typename Base::Iterator it)
	{
		Base::erase(m_alloc, it);
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
};
/// @}

} // end namespace anki

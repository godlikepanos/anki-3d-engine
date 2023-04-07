// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/MemoryPool.h>
#include <AnKi/Util/Hash.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/SparseArray.h>

namespace anki {

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

/// Specialization for some fundamental types.
template<typename T>
concept IsFundamentalOrPointer = std::is_fundamental_v<T> || std::is_pointer_v<T>;

template<IsFundamentalOrPointer TKey>
class DefaultHasher<TKey>
{
public:
	U64 operator()(const TKey a) const
	{
		return computeHash(&a, sizeof(a));
	}
};

/// SparseArray configuration. See SparseArray docs for details.
/// @memberof HashMap
class HashMapSparseArrayConfig
{
public:
	using Index = U64;

	static constexpr Index getInitialStorageSize()
	{
		return 64;
	}

	static constexpr U32 getLinearProbingCount()
	{
		return 8;
	}

	static constexpr F32 getMaxLoadFactor()
	{
		return 0.8f;
	}
};

/// Hash map template.
template<typename TKey, typename TValue, typename THasher = DefaultHasher<TKey>,
		 typename TMemoryPool = SingletonMemoryPoolWrapper<DefaultMemoryPool>,
		 typename TSparseArrayConfig = HashMapSparseArrayConfig>
class HashMap
{
public:
	// Typedefs
	using SparseArrayType = SparseArray<TValue, TMemoryPool, TSparseArrayConfig>;
	using Value = TValue;
	using Key = TKey;
	using Hasher = THasher;
	using Iterator = typename SparseArrayType::Iterator;
	using ConstIterator = typename SparseArrayType::ConstIterator;

	/// Default constructor.
	HashMap(const TMemoryPool& pool = TMemoryPool())
		: m_sparseArr(pool)
	{
	}

	/// Move.
	HashMap(HashMap&& b)
	{
		*this = std::move(b);
	}

	/// Copy.
	HashMap(const HashMap& b)
		: m_sparseArr(b.m_sparseArr)
	{
	}

	~HashMap() = default;

	/// Move.
	HashMap& operator=(HashMap&& b)
	{
		m_sparseArr = std::move(b.m_sparseArr);
		return *this;
	}

	/// Move.
	HashMap& operator=(const HashMap& b)
	{
		m_sparseArr = b.m_sparseArr;
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

	PtrSize getSize() const
	{
		return m_sparseArr.getSize();
	}

	/// Destroy the list.
	void destroy()
	{
		m_sparseArr.destroy();
	}

	/// Construct an element inside the map.
	template<typename... TArgs>
	Iterator emplace(const TKey& key, TArgs&&... args)
	{
		const U64 hash = THasher()(key);
		return m_sparseArr.emplace(hash, std::forward<TArgs>(args)...);
	}

	/// Erase element.
	void erase(Iterator it)
	{
		m_sparseArr.erase(it);
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

protected:
	SparseArrayType m_sparseArr;
};
/// @}

} // end namespace anki

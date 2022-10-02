// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/MemoryPool.h>
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

template<>
class DefaultHasher<U32>
{
public:
	U64 operator()(const U32 a) const
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
		 typename TSparseArrayConfig = HashMapSparseArrayConfig>
class HashMap
{
public:
	// Typedefs
	using SparseArrayType = SparseArray<TValue, TSparseArrayConfig>;
	using Value = TValue;
	using Key = TKey;
	using Hasher = THasher;
	using Iterator = typename SparseArrayType::Iterator;
	using ConstIterator = typename SparseArrayType::ConstIterator;

	/// Default constructor.
	HashMap() = default;

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

	PtrSize getSize() const
	{
		return m_sparseArr.getSize();
	}

	/// Destroy the list.
	template<typename TMemPool>
	void destroy(TMemPool& pool)
	{
		m_sparseArr.destroy(pool);
	}

	/// Construct an element inside the map.
	template<typename TMemPool, typename... TArgs>
	Iterator emplace(TMemPool& pool, const TKey& key, TArgs&&... args)
	{
		const U64 hash = THasher()(key);
		return m_sparseArr.emplace(pool, hash, std::forward<TArgs>(args)...);
	}

	/// Erase element.
	template<typename TMemPool>
	void erase(TMemPool& pool, Iterator it)
	{
		m_sparseArr.erase(pool, it);
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

/// Hash map template with automatic cleanup.
template<typename TKey, typename TValue, typename THasher = DefaultHasher<TKey>,
		 typename TSparseArrayConfig = HashMapSparseArrayConfig,
		 typename TMemPool = MemoryPoolPtrWrapper<BaseMemoryPool>>
class HashMapRaii : public HashMap<TKey, TValue, THasher, TSparseArrayConfig>
{
public:
	using Base = HashMap<TKey, TValue, THasher, TSparseArrayConfig>;
	using MemoryPool = TMemPool;

	/// Default constructor.
	HashMapRaii(const TMemPool& pool)
		: m_pool(pool)
	{
	}

	/// Move.
	HashMapRaii(HashMapRaii&& b)
	{
		*this = std::move(b);
	}

	/// Copy.
	HashMapRaii(const HashMapRaii& b)
		: Base()
	{
		copy(b);
	}

	/// Destructor.
	~HashMapRaii()
	{
		destroy();
	}

	/// Move.
	HashMapRaii& operator=(HashMapRaii&& b)
	{
		std::move(*static_cast<HashMapRaii>(this));
		m_pool = std::move(b.m_pool);
		return *this;
	}

	/// Copy.
	HashMapRaii& operator=(const HashMapRaii& b)
	{
		copy(b);
		return *this;
	}

	/// Construct an element inside the map.
	template<typename... TArgs>
	typename Base::Iterator emplace(const TKey& key, TArgs&&... args)
	{
		return Base::emplace(m_pool, key, std::forward<TArgs>(args)...);
	}

	/// Erase element.
	void erase(typename Base::Iterator it)
	{
		Base::erase(m_pool, it);
	}

	/// Clean up the map.
	void destroy()
	{
		Base::destroy(m_pool);
	}

private:
	MemoryPool m_pool;

	void copy(const HashMapRaii& b)
	{
		destroy();
		m_pool = b.m_pool;
		b.m_sparseArr.clone(m_pool, Base::m_sparseArr);
	}
};
/// @}

} // end namespace anki

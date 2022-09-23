// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Allocator.h>
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

protected:
	SparseArrayType m_sparseArr;
};

/// Hash map template with automatic cleanup.
template<typename TKey, typename TValue, typename THasher = DefaultHasher<TKey>,
		 typename TSparseArrayConfig = HashMapSparseArrayConfig>
class HashMapAuto : public HashMap<TKey, TValue, THasher, TSparseArrayConfig>
{
public:
	using Base = HashMap<TKey, TValue, THasher, TSparseArrayConfig>;

	/// Default constructor.
	HashMapAuto(const GenericMemoryPoolAllocator<U8>& alloc)
		: m_alloc(alloc)
	{
	}

	/// Move.
	HashMapAuto(HashMapAuto&& b)
	{
		*this = std::move(b);
	}

	/// Copy.
	HashMapAuto(const HashMapAuto& b)
		: Base()
	{
		copy(b);
	}

	/// Destructor.
	~HashMapAuto()
	{
		destroy();
	}

	/// Move.
	HashMapAuto& operator=(HashMapAuto&& b)
	{
		std::move(*static_cast<HashMapAuto>(this));
		m_alloc = std::move(b.m_alloc);
		return *this;
	}

	/// Copy.
	HashMapAuto& operator=(const HashMapAuto& b)
	{
		copy(b);
		return *this;
	}

	/// Construct an element inside the map.
	template<typename... TArgs>
	typename Base::Iterator emplace(const TKey& key, TArgs&&... args)
	{
		return Base::emplace(m_alloc, key, std::forward<TArgs>(args)...);
	}

	/// Erase element.
	void erase(typename Base::Iterator it)
	{
		Base::erase(m_alloc, it);
	}

	/// Clean up the map.
	void destroy()
	{
		Base::destroy(m_alloc);
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;

	void copy(const HashMapAuto& b)
	{
		destroy();
		m_alloc = b.m_alloc;
		b.m_sparseArr.clone(m_alloc, Base::m_sparseArr);
	}
};
/// @}

} // end namespace anki

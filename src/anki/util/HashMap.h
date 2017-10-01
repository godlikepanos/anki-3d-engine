// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

/// Hash map template.
template<typename TKey, typename TValue, typename THasher = DefaultHasher<TKey>>
class HashMap
{
public:
	using SparseArrayType = SparseArray<TValue, U64>;
	using Value = TValue;
	using Key = TKey;
	using Hasher = THasher;
	using Iterator = typename SparseArrayType::Iterator;
	using ConstIterator = typename SparseArrayType::ConstIterator;

	/// Default constructor.
	HashMap()
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
/// @}

} // end namespace anki

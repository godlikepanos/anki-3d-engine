// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/gr/GrObject.h>
#include <anki/gr/GrManager.h>
#include <anki/util/HashMap.h>
#include <anki/util/NonCopyable.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// A cache that can be used for specific GrObject types.
class GrObjectCache : public NonCopyable
{
	friend class GrObject;

public:
	GrObjectCache(GrManager* gr)
		: m_gr(gr)
	{
	}

	~GrObjectCache();

	/// Create a new graphics object and use the cache to avoid duplication. It's thread safe.
	template<typename T, typename TArg>
	GrObjectPtr<T> newInstance(const TArg& arg, U64 overrideHash = 0);

private:
	using Key = U64;

	/// Hash the hash.
	class Hasher
	{
	public:
		U64 operator()(const Key& b) const
		{
			return b;
		}
	};

	/// Hash compare.
	class Compare
	{
	public:
		Bool operator()(const Key& a, const Key& b) const
		{
			return a == b;
		}
	};

	GrManager* m_gr;
	HashMap<U64, GrObject*, Hasher, Compare> m_map;
	Mutex m_mtx;

	/// Try to find an object in the cache.
	GrObject* tryFind(U64 hash)
	{
		auto it = m_map.find(hash);
		return (it != m_map.getEnd()) ? (*it) : nullptr;
	}

	/// Unregister an object from the cache.
	void unregisterObject(GrObject* obj);
};

template<typename T, typename TArg>
inline GrObjectPtr<T> GrObjectCache::newInstance(const TArg& arg, U64 overrideHash)
{
	U64 hash = (overrideHash != 0) ? overrideHash : arg.computeHash();
	ANKI_ASSERT(hash != 0);

	LockGuard<Mutex> lock(m_mtx);
	GrObject* ptr = tryFind(hash);
	if(ptr == nullptr)
	{
		auto tptr = m_gr->template newInstanceCached<T>(hash, this, arg);
		m_map.pushBack(m_gr->getAllocator(), hash, tptr.get());
		return tptr;
	}
	else
	{
		ANKI_ASSERT(ptr->getType() == T::CLASS_TYPE);
		return GrObjectPtr<T>(static_cast<T*>(ptr));
	}
}
/// @}

} // end namespace anki

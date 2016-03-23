// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/gr/GrObject.h>
#include <anki/util/HashMap.h>
#include <anki/util/NonCopyable.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// A cache that can be used for specific GrObject types.
class GrObjectCache : public NonCopyable
{
public:
	void init(const GrAllocator<U8>& alloc)
	{
		m_alloc = alloc;
	}

	Mutex& getMutex()
	{
		return m_mtx;
	}

	/// Try to find an object in the cache.
	GrObject* tryFind(U64 hash)
	{
		auto it = m_map.find(hash);
		return (it != m_map.getEnd()) ? *it : nullptr;
	}

	/// Register object in the cache.
	void registerObject(GrObject* obj)
	{
		ANKI_ASSERT(obj->getHash() != 0);
		ANKI_ASSERT(tryFind(obj->getHash()) == nullptr);
		m_map.pushBack(m_alloc, obj->getHash(), obj);
	}

	/// Unregister an object from the cache.
	void unregisterObject(GrObject* obj)
	{
		ANKI_ASSERT(tryFind(obj->getHash()) != nullptr);
		auto it = m_map.find(obj->getHash());
		m_map.erase(m_alloc, it);
	}

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

	GrAllocator<U8> m_alloc;
	HashMap<U64, GrObject*, Hasher, Compare> m_map;
	Mutex m_mtx;
};
/// @}

} // end namespace anki

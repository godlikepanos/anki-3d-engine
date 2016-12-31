// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrObjectCache.h>

namespace anki
{

GrObjectCache::~GrObjectCache()
{
	// Some GrObjects may be in flight but someone destroys the cache. Do a manual destruction of the map
	for(auto it : m_map)
	{
		GrObject* ptr = it;
		unregisterObject(ptr);
	}
}

void GrObjectCache::unregisterObject(GrObject* obj)
{
	ANKI_ASSERT(obj);
	ANKI_ASSERT(obj->getHash() != 0);
	ANKI_ASSERT(obj->m_cache == this);

	LockGuard<Mutex> lock(m_mtx);
	ANKI_ASSERT(tryFind(obj->getHash()) != nullptr);
	auto it = m_map.find(obj->getHash());
	m_map.erase(m_gr->getAllocator(), it);
	obj->m_cache = nullptr;
}

} // end namespace anki

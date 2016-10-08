// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrObjectCache.h>

namespace anki
{

void GrObjectCache::unregisterObject(GrObject* obj)
{
	ANKI_ASSERT(obj);
	ANKI_ASSERT(obj->getHash() != 0);

	LockGuard<Mutex> lock(m_mtx);
	ANKI_ASSERT(tryFind(obj->getHash()) != nullptr);
	auto it = m_map.find(obj->getHash());
	m_map.erase(m_gr->getAllocator(), it);
}

} // end namespace anki

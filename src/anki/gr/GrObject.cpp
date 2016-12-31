// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrObject.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/GrObjectCache.h>

namespace anki
{

GrObject::GrObject(GrManager* manager, GrObjectType type, U64 hash, GrObjectCache* cache)
	: m_refcount(0)
	, m_manager(manager)
	, m_uuid(m_manager->getUuidIndex()++)
	, m_hash(hash)
	, m_type(type)
	, m_cache(cache)
{
}

GrObject::~GrObject()
{
	if(m_cache)
	{
		m_cache->unregisterObject(this);
	}
}

GrAllocator<U8> GrObject::getAllocator() const
{
	return m_manager->getAllocator();
}

} // end namespace anki

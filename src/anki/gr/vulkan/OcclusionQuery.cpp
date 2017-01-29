// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/OcclusionQuery.h>
#include <anki/gr/vulkan/OcclusionQueryImpl.h>

namespace anki
{

OcclusionQuery::OcclusionQuery(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

OcclusionQuery::~OcclusionQuery()
{
}

void OcclusionQuery::init()
{
	m_impl.reset(getAllocator().newInstance<OcclusionQueryImpl>(&getManager()));

	if(m_impl->init())
	{
		ANKI_VK_LOGF("Cannot recover");
	}
}

} // end namespace anki

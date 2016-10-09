// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/ResourceGroup.h>
#include <anki/gr/vulkan/ResourceGroupImpl.h>

namespace anki
{

ResourceGroup::ResourceGroup(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

ResourceGroup::~ResourceGroup()
{
}

void ResourceGroup::init(const ResourceGroupInitInfo& init)
{
	m_impl.reset(getAllocator().newInstance<ResourceGroupImpl>(&getManager()));

	if(m_impl->init(init))
	{
		ANKI_LOGF("Cannot recover");
	}
}

} // end namespace anki

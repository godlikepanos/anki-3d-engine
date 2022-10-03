// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/OcclusionQuery.h>
#include <AnKi/Gr/Vulkan/OcclusionQueryImpl.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

OcclusionQuery* OcclusionQuery::newInstance(GrManager* manager)
{
	OcclusionQueryImpl* impl = anki::newInstance<OcclusionQueryImpl>(manager->getMemoryPool(), manager, "N/A");
	const Error err = impl->init();
	if(err)
	{
		deleteInstance(manager->getMemoryPool(), impl);
		impl = nullptr;
	}
	return impl;
}

OcclusionQueryResult OcclusionQuery::getResult() const
{
	return static_cast<const OcclusionQueryImpl*>(this)->getResultInternal();
}

} // end namespace anki

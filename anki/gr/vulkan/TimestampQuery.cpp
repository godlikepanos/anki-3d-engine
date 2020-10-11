// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/TimestampQuery.h>
#include <anki/gr/vulkan/TimestampQueryImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

TimestampQuery* TimestampQuery::newInstance(GrManager* manager)
{
	TimestampQueryImpl* impl = manager->getAllocator().newInstance<TimestampQueryImpl>(manager, "N/A");
	const Error err = impl->init();
	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
		impl = nullptr;
	}
	return impl;
}

TimestampQueryResult TimestampQuery::getResult(Second& timestamp) const
{
	return static_cast<const TimestampQueryImpl*>(this)->getResultInternal(timestamp);
}

} // end namespace anki

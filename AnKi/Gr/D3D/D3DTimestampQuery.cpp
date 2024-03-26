// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DTimestampQuery.h>

namespace anki {

TimestampQuery* TimestampQuery::newInstance()
{
	TimestampQueryImpl* impl = anki::newInstance<TimestampQueryImpl>(GrMemoryPool::getSingleton(), "N/A");
	const Error err = impl->init();
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

TimestampQueryResult TimestampQuery::getResult(Second& timestamp) const
{
	ANKI_ASSERT(!"TODO");
	return TimestampQueryResult::kNotAvailable;
}

TimestampQueryImpl::~TimestampQueryImpl()
{
}

Error TimestampQueryImpl::init()
{
	ANKI_ASSERT(!"TODO");
	return Error::kNone;
}

} // end namespace anki

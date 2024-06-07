// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DTimestampQuery.h>
#include <AnKi/Gr/D3D/D3DGrManager.h>

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
	ANKI_D3D_SELF_CONST(TimestampQueryImpl);

	U64 result;
	const Bool resultAvailable = TimestampQueryFactory::getSingleton().getResult(self.m_handle, result);

	if(resultAvailable)
	{
		timestamp = F64(result) / F64(getGrManagerImpl().getTimestampFrequency());
	}

	return (resultAvailable) ? TimestampQueryResult::kAvailable : TimestampQueryResult::kNotAvailable;
}

TimestampQueryImpl::~TimestampQueryImpl()
{
	TimestampQueryFactory::getSingleton().deleteQuery(m_handle);
}

Error TimestampQueryImpl::init()
{
	ANKI_CHECK(TimestampQueryFactory::getSingleton().newQuery(m_handle));

	return Error::kNone;
}

} // end namespace anki

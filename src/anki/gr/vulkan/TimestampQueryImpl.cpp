// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/TimestampQueryImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

TimestampQueryImpl::~TimestampQueryImpl()
{
	if(m_handle)
	{
		getGrManagerImpl().getTimestampQueryFactory().deleteQuery(m_handle);
	}
}

Error TimestampQueryImpl::init()
{
	ANKI_CHECK(getGrManagerImpl().getTimestampQueryFactory().newQuery(m_handle));
	return Error::NONE;
}

TimestampQueryResult TimestampQueryImpl::getResultInternal(U64& timestamp) const
{
	ANKI_ASSERT(m_handle);

	VkResult res;
	ANKI_VK_CHECKF(res = vkGetQueryPoolResults(getDevice(),
					   m_handle.getQueryPool(),
					   m_handle.getQueryIndex(),
					   1,
					   sizeof(timestamp),
					   &timestamp,
					   sizeof(timestamp),
					   VK_QUERY_RESULT_64_BIT));

	TimestampQueryResult qout = TimestampQueryResult::NOT_AVAILABLE;
	if(res == VK_SUCCESS)
	{
		qout = TimestampQueryResult::AVAILABLE;
	}
	else if(res == VK_NOT_READY)
	{
		qout = TimestampQueryResult::NOT_AVAILABLE;
	}
	else
	{
		ANKI_ASSERT(0);
	}

	return qout;
}

} // end namespace anki

// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/TimestampQueryImpl.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>

namespace anki {

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

	m_timestampPeriod = U64(getGrManagerImpl().getPhysicalDeviceProperties().limits.timestampPeriod);

	return Error::kNone;
}

TimestampQueryResult TimestampQueryImpl::getResultInternal(Second& timestamp) const
{
	ANKI_ASSERT(m_handle);
	timestamp = -1.0;

	VkResult res;
	U64 value;
	ANKI_VK_CHECKF(res = vkGetQueryPoolResults(getDevice(), m_handle.getQueryPool(), m_handle.getQueryIndex(), 1,
											   sizeof(value), &value, sizeof(value), VK_QUERY_RESULT_64_BIT));

	TimestampQueryResult qout = TimestampQueryResult::kNotAvailable;
	if(res == VK_SUCCESS)
	{
		value *= m_timestampPeriod;
		timestamp = Second(value) / Second(1000000000);
		qout = TimestampQueryResult::kAvailable;
	}
	else if(res == VK_NOT_READY)
	{
		qout = TimestampQueryResult::kNotAvailable;
	}
	else
	{
		ANKI_ASSERT(0);
	}

	return qout;
}

} // end namespace anki

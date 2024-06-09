// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkTimestampQuery.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>

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
	ANKI_VK_SELF_CONST(TimestampQueryImpl);

	ANKI_ASSERT(self.m_handle);
	timestamp = -1.0;

	VkResult res;
	U64 value;
	ANKI_VK_CHECKF(res = vkGetQueryPoolResults(getVkDevice(), self.m_handle.getQueryPool(), self.m_handle.getQueryIndex(), 1, sizeof(value), &value,
											   sizeof(value), VK_QUERY_RESULT_64_BIT));

	TimestampQueryResult qout = TimestampQueryResult::kNotAvailable;
	if(res == VK_SUCCESS)
	{
		value *= self.m_timestampPeriod;
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

TimestampQueryImpl::~TimestampQueryImpl()
{
	if(m_handle)
	{
		TimestampQueryFactory::getSingleton().deleteQuery(m_handle);
	}
}

Error TimestampQueryImpl::init()
{
	ANKI_CHECK(TimestampQueryFactory::getSingleton().newQuery(m_handle));

	m_timestampPeriod = U64(getGrManagerImpl().getPhysicalDeviceProperties().limits.timestampPeriod);

	return Error::kNone;
}

} // end namespace anki

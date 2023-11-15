// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/PipelineQueryImpl.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>

namespace anki {

PipelineQueryImpl::~PipelineQueryImpl()
{
	if(m_handle)
	{
		getGrManagerImpl().getPipelineQueryFactory(m_type).deleteQuery(m_handle);
	}
}

Error PipelineQueryImpl::init(PipelineQueryType type)
{
	ANKI_ASSERT(type < PipelineQueryType::kCount);
	m_type = type;
	ANKI_CHECK(getGrManagerImpl().getPipelineQueryFactory(type).newQuery(m_handle));
	return Error::kNone;
}

PipelineQueryResult PipelineQueryImpl::getResultInternal(U64& value) const
{
	ANKI_ASSERT(m_handle);
	value = kMaxU64;

	VkResult res;
	ANKI_VK_CHECKF(res = vkGetQueryPoolResults(getVkDevice(), m_handle.getQueryPool(), m_handle.getQueryIndex(), 1, sizeof(value), &value,
											   sizeof(value), VK_QUERY_RESULT_64_BIT));

	PipelineQueryResult qout = PipelineQueryResult::kNotAvailable;
	if(res == VK_SUCCESS)
	{
		qout = PipelineQueryResult::kAvailable;
	}
	else if(res == VK_NOT_READY)
	{
		value = kMaxU64;
		qout = PipelineQueryResult::kNotAvailable;
	}
	else
	{
		ANKI_ASSERT(0);
	}

	return qout;
}

} // end namespace anki

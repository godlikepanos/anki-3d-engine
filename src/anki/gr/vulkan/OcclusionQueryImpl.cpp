// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/OcclusionQueryImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

OcclusionQueryImpl::~OcclusionQueryImpl()
{
	if(m_handle)
	{
		getGrManagerImpl().getQueryAllocator().deleteQuery(m_handle);
	}
}

Error OcclusionQueryImpl::init()
{
	ANKI_CHECK(getGrManagerImpl().getQueryAllocator().newQuery(m_handle));
	return ErrorCode::NONE;
}

OcclusionQueryResult OcclusionQueryImpl::getResult() const
{
	ANKI_ASSERT(m_handle);
	U64 out = 0;

	VkResult res;
	ANKI_VK_CHECKF(res = vkGetQueryPoolResults(getDevice(),
					   m_handle.m_pool,
					   m_handle.m_queryIndex,
					   1,
					   sizeof(out),
					   &out,
					   sizeof(out),
					   VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | VK_QUERY_RESULT_PARTIAL_BIT));

	OcclusionQueryResult qout = OcclusionQueryResult::NOT_AVAILABLE;
	if(res == VK_SUCCESS)
	{
		qout = (out) ? OcclusionQueryResult::VISIBLE : OcclusionQueryResult::NOT_VISIBLE;
	}
	else if(res == VK_NOT_READY)
	{
		qout = OcclusionQueryResult::NOT_AVAILABLE;
	}
	else
	{
		ANKI_ASSERT(0);
	}

	return qout;
}

} // end namespace anki

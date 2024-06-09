// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkOcclusionQuery.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>

namespace anki {

OcclusionQuery* OcclusionQuery::newInstance()
{
	OcclusionQueryImpl* impl = anki::newInstance<OcclusionQueryImpl>(GrMemoryPool::getSingleton(), "N/A");
	const Error err = impl->init();
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

OcclusionQueryResult OcclusionQuery::getResult() const
{
	return static_cast<const OcclusionQueryImpl*>(this)->getResultInternal();
}

OcclusionQueryImpl::~OcclusionQueryImpl()
{
	if(m_handle)
	{
		OcclusionQueryFactory::getSingleton().deleteQuery(m_handle);
	}
}

Error OcclusionQueryImpl::init()
{
	ANKI_CHECK(OcclusionQueryFactory::getSingleton().newQuery(m_handle));
	return Error::kNone;
}

OcclusionQueryResult OcclusionQueryImpl::getResultInternal() const
{
	ANKI_ASSERT(m_handle);
	U64 out = 0;

	VkResult res;
	ANKI_VK_CHECKF(res = vkGetQueryPoolResults(getVkDevice(), m_handle.getQueryPool(), m_handle.getQueryIndex(), 1, sizeof(out), &out, sizeof(out),
											   VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | VK_QUERY_RESULT_PARTIAL_BIT));

	OcclusionQueryResult qout = OcclusionQueryResult::kNotAvailable;
	if(res == VK_SUCCESS)
	{
		qout = (out) ? OcclusionQueryResult::kVisible : OcclusionQueryResult::kNotVisible;
	}
	else if(res == VK_NOT_READY)
	{
		qout = OcclusionQueryResult::kNotAvailable;
	}
	else
	{
		ANKI_ASSERT(0);
	}

	return qout;
}

} // end namespace anki

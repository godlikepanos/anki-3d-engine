// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/OcclusionQueryImpl.h>

namespace anki
{

//==============================================================================
OcclusionQueryImpl::~OcclusionQueryImpl()
{
	if(m_handle)
	{
		vkDestroyQueryPool(getDevice(), m_handle, nullptr);
	}
}

//==============================================================================
Error OcclusionQueryImpl::init(OcclusionQueryResultBit condRenderingBit)
{
	m_condRenderingBit = condRenderingBit;

	VkQueryPoolCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	ci.queryType = VK_QUERY_TYPE_OCCLUSION;
	ci.queryCount = 1;

	ANKI_VK_CHECK(vkCreateQueryPool(getDevice(), &ci, nullptr, &m_handle));

	return ErrorCode::NONE;
}

//==============================================================================
OcclusionQueryResult OcclusionQueryImpl::getResult() const
{
	ANKI_ASSERT(m_handle);
	U64 out = 0;

	VkResult res;
	ANKI_VK_CHECKF(
		res = vkGetQueryPoolResults(getDevice(),
			m_handle,
			0,
			1,
			sizeof(out),
			&out,
			sizeof(out),
			VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT
				| VK_QUERY_RESULT_PARTIAL_BIT));

	OcclusionQueryResult qout = OcclusionQueryResult::NOT_AVAILABLE;
	if(res == VK_SUCCESS)
	{
		qout = (out) ? OcclusionQueryResult::VISIBLE
					 : OcclusionQueryResult::NOT_VISIBLE;
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

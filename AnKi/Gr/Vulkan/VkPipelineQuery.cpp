// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkPipelineQuery.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>

namespace anki {

PipelineQuery* PipelineQuery::newInstance(const PipelineQueryInitInfo& inf)
{
	PipelineQueryImpl* impl = anki::newInstance<PipelineQueryImpl>(GrMemoryPool::getSingleton(), inf.getName());
	const Error err = impl->init(inf.m_type);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

PipelineQueryResult PipelineQuery::getResult(U64& value) const
{
	ANKI_VK_SELF_CONST(PipelineQueryImpl);

	ANKI_ASSERT(self.m_handle);
	value = kMaxU64;

	VkResult res;
	ANKI_VK_CHECKF(res = vkGetQueryPoolResults(getVkDevice(), self.m_handle.getQueryPool(), self.m_handle.getQueryIndex(), 1, sizeof(value), &value,
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

PipelineQueryImpl::~PipelineQueryImpl()
{
	if(m_handle)
	{
		PrimitivesPassedClippingFactory::getSingleton().deleteQuery(m_handle);
	}
}

Error PipelineQueryImpl::init(PipelineQueryType type)
{
	ANKI_ASSERT(type < PipelineQueryType::kCount);
	m_type = type;
	ANKI_CHECK(PrimitivesPassedClippingFactory::getSingleton().newQuery(m_handle));
	return Error::kNone;
}

} // end namespace anki

// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/PipelineQuery.h>
#include <AnKi/Gr/Vulkan/PipelineQueryImpl.h>
#include <AnKi/Gr/GrManager.h>

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
	return static_cast<const PipelineQueryImpl*>(this)->getResultInternal(value);
}

} // end namespace anki

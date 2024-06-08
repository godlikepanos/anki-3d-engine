// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DPipelineQuery.h>

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
	ANKI_D3D_SELF_CONST(PipelineQueryImpl);

	const Bool resultAvailable = PrimitivesPassedClippingFactory::getSingleton().getResult(self.m_handle, value);

	return (resultAvailable) ? PipelineQueryResult::kAvailable : PipelineQueryResult::kNotAvailable;
}

PipelineQueryImpl::~PipelineQueryImpl()
{
	PrimitivesPassedClippingFactory::getSingleton().deleteQuery(m_handle);
}

Error PipelineQueryImpl::init([[maybe_unused]] PipelineQueryType type)
{
	ANKI_ASSERT(type == PipelineQueryType::kPrimitivesPassedClipping);
	ANKI_CHECK(PrimitivesPassedClippingFactory::getSingleton().newQuery(m_handle));

	return Error::kNone;
}

} // end namespace anki

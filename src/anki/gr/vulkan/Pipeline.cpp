// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Pipeline.h>
#include <anki/gr/vulkan/PipelineImpl.h>
#include <anki/core/Trace.h>

namespace anki
{

Pipeline::Pipeline(GrManager* manager, U64 hash)
	: GrObject(manager, CLASS_TYPE, hash)
{
	ANKI_TRACE_INC_COUNTER(GR_PIPELINES_CREATED, 1);
}

Pipeline::~Pipeline()
{
}

void Pipeline::init(const PipelineInitInfo& init)
{
	m_impl.reset(getAllocator().newInstance<PipelineImpl>(&getManager()));
	if(m_impl->init(init))
	{
		ANKI_LOGF("Cannot recover");
	}
}

} // end namespace anki

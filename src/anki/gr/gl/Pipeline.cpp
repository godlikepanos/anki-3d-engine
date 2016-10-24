// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Pipeline.h>
#include <anki/gr/gl/PipelineImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/core/Trace.h>

namespace anki
{

Pipeline::Pipeline(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
	ANKI_TRACE_INC_COUNTER(GR_PIPELINES_CREATED, 1);
}

Pipeline::~Pipeline()
{
}

class CreatePipelineCommand final : public GlCommand
{
public:
	PipelinePtr m_ppline;
	PipelineInitInfo m_init;

	CreatePipelineCommand(Pipeline* ppline, const PipelineInitInfo& init)
		: m_ppline(ppline)
		, m_init(init)
	{
	}

	Error operator()(GlState&)
	{
		PipelineImpl& impl = *m_ppline->m_impl;

		Error err = impl.init(m_init);

		GlObject::State oldState = impl.setStateAtomically(err ? GlObject::State::ERROR : GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		return err;
	}
};

void Pipeline::init(const PipelineInitInfo& init)
{
	m_impl.reset(getAllocator().newInstance<PipelineImpl>(&getManager()));

	CommandBufferInitInfo inf;
	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>(inf);

	cmdb->m_impl->pushBackNewCommand<CreatePipelineCommand>(this, init);
	cmdb->flush();
}

} // end namespace anki

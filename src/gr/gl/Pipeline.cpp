// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Pipeline.h>
#include <anki/gr/gl/PipelineImpl.h>
#include <anki/gr/gl/CommandBufferImpl.h>

namespace anki {

//==============================================================================
Pipeline::Pipeline(GrManager* manager)
	: GrObject(manager)
{}

//==============================================================================
Pipeline::~Pipeline()
{}

//==============================================================================
class CreatePipelineCommand final: public GlCommand
{
public:
	PipelinePtr m_ppline;
	PipelineInitializer m_init;

	CreatePipelineCommand(Pipeline* ppline, const PipelineInitializer& init)
		: m_ppline(ppline)
		, m_init(init)
	{}

	Error operator()(GlState&)
	{
		PipelineImpl& impl = m_ppline->getImplementation();

		Error err = impl.create(m_init);

		GlObject::State oldState = impl.setStateAtomically(
			err ? GlObject::State::ERROR : GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		return err;
	}
};

void Pipeline::create(const PipelineInitializer& init)
{
	m_impl.reset(getAllocator().newInstance<PipelineImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>();

	cmdb->getImplementation().pushBackNewCommand<CreatePipelineCommand>(
		this, init);
	cmdb->flush();
}

} // end namespace anki


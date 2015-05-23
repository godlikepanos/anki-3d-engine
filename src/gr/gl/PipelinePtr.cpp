// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/PipelinePtr.h"
#include "anki/gr/PipelineCommon.h"
#include "anki/gr/gl/PipelineImpl.h"
#include "anki/gr/gl/ShaderImpl.h"
#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/CommandBufferPtr.h"
#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/gl/RenderingThread.h"

namespace anki {

//==============================================================================
// Commands                                                                    =
//==============================================================================

class CreatePipelineCommand final: public GlCommand
{
public:
	PipelinePtr m_ppline;
	PipelineInitializer m_init;

	CreatePipelineCommand(
		const PipelinePtr& ppline,
		const PipelineInitializer& init)
	:	m_ppline(ppline),
		m_init(init)
	{}

	Error operator()(CommandBufferImpl* cmdb)
	{
		Error err = m_ppline.get().create(m_init);

		GlObject::State oldState = m_ppline.get().setStateAtomically(
			err ? GlObject::State::ERROR : GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		return err;
	}
};

class BindPipelineCommand final: public GlCommand
{
public:
	PipelinePtr m_ppline;

	BindPipelineCommand(PipelinePtr& ppline)
	:	m_ppline(ppline)
	{}

	Error operator()(CommandBufferImpl* commands)
	{
		GlState& state = commands->getManager().getImplementation().
			getRenderingThread().getState();

		auto name = m_ppline.get().getGlName();
		if(state.m_crntPpline != name)
		{
			m_ppline.get().bind();
			state.m_crntPpline = name;
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
// PipelinePtr                                                              =
//==============================================================================

//==============================================================================
PipelinePtr::PipelinePtr()
{}

//==============================================================================
PipelinePtr::~PipelinePtr()
{}

//==============================================================================
void PipelinePtr::create(GrManager* manager, const Initializer& init)
{
	CommandBufferPtr cmdb;
	cmdb.create(manager);

	Base::create(cmdb.get().getManager());
	get().setStateAtomically(GlObject::State::TO_BE_CREATED);
	cmdb.get().pushBackNewCommand<CreatePipelineCommand>(*this, init);

	cmdb.flush();
}

//==============================================================================
void PipelinePtr::bind(CommandBufferPtr& commands)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<BindPipelineCommand>(*this);
}

} // end namespace anki


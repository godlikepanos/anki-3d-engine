// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/PipelineHandle.h"
#include "anki/gr/PipelineCommon.h"
#include "anki/gr/gl/PipelineImpl.h"
#include "anki/gr/gl/DeferredDeleter.h"

namespace anki {

//==============================================================================
// Commands                                                                    =
//==============================================================================

class CreatePipelineCommand final: public GlCommand
{
public:
	PipelineHandle m_ppline;
	PipelineInitializer m_init;

	CreatePipelineCommand(
		const PipelineHandle& ppline, 
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
	PipelineHandle m_ppline;

	BindPipelineCommand(PipelineHandle& ppline)
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
// PipelineHandle                                                              =
//==============================================================================

//==============================================================================
PipelineHandle::PipelineHandle()
{}

//==============================================================================
PipelineHandle::~PipelineHandle()
{}

//==============================================================================
Error PipelineHandle::create(GrManager* manager, const Initializer& init)
{
	using DeleteCommand = DeleteObjectCommand<PipelineImpl>;
	using Deleter = DeferredDeleter<PipelineImpl, DeleteCommand>;

	CommandBufferHandle cmdb;
	ANKI_CHECK(cmdb.create(manager));

	Base::create(cmdb.get().getManager(), Deleter());
	get().setStateAtomically(GlObject::State::TO_BE_CREATED);
	cmdb.get().pushBackNewCommand<CreatePipelineCommand>(*this, init);

	cmdb.flush();

	return ErrorCode::NONE;
}

//==============================================================================
void PipelineHandle::bind(CommandBufferHandle& commands)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<BindPipelineCommand>(*this);
}

} // end namespace anki


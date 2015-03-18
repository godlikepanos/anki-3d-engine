// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/PipelineHandle.h"
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
	Array<ShaderHandle, 6> m_progs;
	U8 m_progsCount;

	CreatePipelineCommand(PipelineHandle& ppline, 
		const ShaderHandle* progsBegin, const ShaderHandle* progsEnd)
	:	m_ppline(ppline)
	{
		m_progsCount = 0;
		const ShaderHandle* prog = progsBegin;
		do
		{
			m_progs[m_progsCount++] = *prog;
		} while(++prog != progsEnd);
	}

	Error operator()(CommandBufferImpl* cmdb)
	{
		Error err = m_ppline.get().create(
			&m_progs[0], &m_progs[0] + m_progsCount);

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
		State& state = commands->getManager().getImplementation().
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
Error PipelineHandle::create(
	CommandBufferHandle& commands,
	std::initializer_list<ShaderHandle> iprogs)
{
	Array<ShaderHandle, 6> progs;

	U count = 0;
	for(ShaderHandle prog : iprogs)
	{
		progs[count++] = prog;
	}

	return commonConstructor(commands, &progs[0], &progs[0] + count);
}

//==============================================================================
Error PipelineHandle::commonConstructor(
	CommandBufferHandle& commands,
	const ShaderHandle* progsBegin, const ShaderHandle* progsEnd)
{
	using DeleteCommand = DeleteObjectCommand<PipelineImpl>;
	using Deleter = DeferredDeleter<PipelineImpl, DeleteCommand>;

	Error err = Base::create(commands.get().getManager(), Deleter());
	if(!err)
	{
		get().setStateAtomically(GlObject::State::TO_BE_CREATED);

		commands.get().pushBackNewCommand<CreatePipelineCommand>(
			*this, progsBegin, progsEnd);
	}

	return err;
}

//==============================================================================
void PipelineHandle::bind(CommandBufferHandle& commands)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<BindPipelineCommand>(*this);
}

//==============================================================================
ShaderHandle PipelineHandle::getAttachedProgram(GLenum type) const
{
	ANKI_ASSERT(isCreated());
	Error err = get().serializeOnGetter();
	if(!err)
	{
		return get().getAttachedProgram(type);
	}
	else
	{
		return ShaderHandle();
	}
}

} // end namespace anki


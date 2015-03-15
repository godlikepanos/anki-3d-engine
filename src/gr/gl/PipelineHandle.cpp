// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/PipelineHandle.h"
#include "anki/gr/gl/PipelineImpl.h"
#include "anki/gr/GlHandleDeferredDeleter.h"

namespace anki {

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
	class Command: public GlCommand
	{
	public:
		PipelineHandle m_ppline;
		Array<ShaderHandle, 6> m_progs;
		U8 m_progsCount;

		Command(PipelineHandle& ppline, 
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
			Error err = m_ppline._get().create(
				&m_progs[0], &m_progs[0] + m_progsCount,
				cmdb->getGlobalAllocator());

			GlHandleState oldState = m_ppline._setState(
				err ? GlHandleState::ERROR : GlHandleState::CREATED);
			ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
			(void)oldState;

			return err;
		}
	};

	using Alloc = GlAllocator<PipelineImpl>;
	using DeleteCommand = GlDeleteObjectCommand<PipelineImpl, Alloc>;
	using Deleter = 
		GlHandleDeferredDeleter<PipelineImpl, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._get().getRenderingThread().getDevice(),
		commands._get().getGlobalAllocator(), 
		Deleter());

	if(!err)
	{
		_setState(GlHandleState::TO_BE_CREATED);

		commands._pushBackNewCommand<Command>(*this, progsBegin, progsEnd);
	}

	return err;
}

//==============================================================================
void PipelineHandle::bind(CommandBufferHandle& commands)
{
	ANKI_ASSERT(isCreated());

	class Command: public GlCommand
	{
	public:
		PipelineHandle m_ppline;

		Command(PipelineHandle& ppline)
		:	m_ppline(ppline)
		{}

		Error operator()(CommandBufferImpl* commands)
		{
			State& state = commands->getRenderingThread().getState();

			if(state.m_crntPpline != m_ppline._get().getGlName())
			{
				m_ppline._get().bind();

				state.m_crntPpline = m_ppline._get().getGlName();
			}

			return ErrorCode::NONE;
		}
	};

	commands._pushBackNewCommand<Command>(*this);
}

//==============================================================================
ShaderHandle PipelineHandle::getAttachedProgram(GLenum type) const
{
	ANKI_ASSERT(isCreated());
	Error err = serializeOnGetter();
	if(!err)
	{
		return _get().getAttachedProgram(type);
	}
	else
	{
		return ShaderHandle();
	}
}

} // end namespace anki


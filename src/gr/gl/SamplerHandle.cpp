// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/SamplerHandle.h"
#include "anki/gr/gl/SamplerImpl.h"
#include "anki/gr/GrManager.h"
#include "anki/gr/gl/DeferredDeleter.h"	

namespace anki {

//==============================================================================
// SamplerCommands                                                             =
//==============================================================================

//==============================================================================
class CreateSamplerCommand: public GlCommand
{
public:
	SamplerHandle m_sampler;
	SamplerInitializer m_init;

	CreateSamplerCommand(const SamplerHandle& sampler, 
		const SamplerInitializer& init)
	:	m_sampler(sampler),
		m_init(init)
	{}

	Error operator()(CommandBufferImpl* commands)
	{
		ANKI_ASSERT(commands);

		Error err = m_sampler.get().create(m_init);

		GlObject::State oldState = m_sampler.get().setStateAtomically(
			(err) ? GlObject::State::ERROR : GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		return err;
	}
};

//==============================================================================
class BindSamplerCommand: public GlCommand
{
public:
	SamplerHandle m_sampler;
	U8 m_unit;

	BindSamplerCommand(SamplerHandle& sampler, U8 unit)
	:	m_sampler(sampler), 
		m_unit(unit)
	{}

	Error operator()(CommandBufferImpl*)
	{
		m_sampler.get().bind(m_unit);
		return ErrorCode::NONE;
	}
};

//==============================================================================
class BindDefaultSamplerCommand: public GlCommand
{
public:
	U32 m_unit;

	BindDefaultSamplerCommand(U32 unit)
	:	m_unit(unit)
	{}

	Error operator()(CommandBufferImpl*)
	{
		SamplerImpl::unbind(m_unit);
		return ErrorCode::NONE;
	}
};

//==============================================================================
// SamplerHandle                                                               =
//==============================================================================

//==============================================================================
SamplerHandle::SamplerHandle()
{}

//==============================================================================
SamplerHandle::~SamplerHandle()
{}

//==============================================================================
Error SamplerHandle::create(CommandBufferHandle& commands, 
	const SamplerInitializer& init)
{
	using DeleteCommand = DeleteObjectCommand<SamplerImpl>;
	using Deleter = DeferredDeleter<SamplerImpl, DeleteCommand>;

	Base::create(commands.get().getManager(), Deleter());
	get().setStateAtomically(GlObject::State::TO_BE_CREATED);
	commands.get().pushBackNewCommand<CreateSamplerCommand>(*this, init);

	return ErrorCode::NONE;
}

//==============================================================================
void SamplerHandle::bind(CommandBufferHandle& commands, U32 unit)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<BindSamplerCommand>(*this, unit);
}

//==============================================================================
void SamplerHandle::bindDefault(CommandBufferHandle& commands, U32 unit)
{
	commands.get().pushBackNewCommand<BindDefaultSamplerCommand>(unit);
}

} // end namespace anki

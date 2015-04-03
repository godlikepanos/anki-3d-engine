// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/OcclusionQueryHandle.h"
#include "anki/gr/gl/OcclusionQueryImpl.h"
#include "anki/gr/gl/DeferredDeleter.h"

namespace anki {

//==============================================================================
// Commands                                                                    =
//==============================================================================

/// Create command.
class OqCreateCommand final: public GlCommand
{
public:	
	OcclusionQueryHandle m_handle;
	OcclusionQueryResultBit m_condRenderingBit;

	OqCreateCommand(OcclusionQueryHandle& handle, 
		OcclusionQueryResultBit condRenderingBit)
	:	m_handle(handle),
		m_condRenderingBit(condRenderingBit)
	{}

	Error operator()(CommandBufferImpl*)
	{
		m_handle.get().create(m_condRenderingBit);

		GlObject::State oldState = m_handle.get().setStateAtomically(
			GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		return ErrorCode::NONE;
	}
};

/// Begin query.
class OqBeginCommand final: public GlCommand
{
public:
	OcclusionQueryHandle m_handle;

	OqBeginCommand(OcclusionQueryHandle& handle)
	:	m_handle(handle)
	{}

	Error operator()(CommandBufferImpl*)
	{
		m_handle.get().begin();
		return ErrorCode::NONE;
	}
};

/// End query.
class OqEndCommand final: public GlCommand
{
public:
	OcclusionQueryHandle m_handle;

	OqEndCommand(OcclusionQueryHandle& handle)
	:	m_handle(handle)
	{}

	Error operator()(CommandBufferImpl*)
	{
		m_handle.get().end();
		return ErrorCode::NONE;
	}
};

//==============================================================================
// OcclusionQueryHandle                                                        =
//==============================================================================

//==============================================================================
OcclusionQueryHandle::OcclusionQueryHandle()
{}

//==============================================================================
OcclusionQueryHandle::~OcclusionQueryHandle()
{}

//==============================================================================
Error OcclusionQueryHandle::create(
	GrManager* manager, ResultBit condRenderingBit)
{
	using DeleteCommand = DeleteObjectCommand<OcclusionQueryImpl>;
	using Deleter = DeferredDeleter<OcclusionQueryImpl, DeleteCommand>;

	CommandBufferHandle cmd;
	ANKI_CHECK(cmd.create(manager));

	Base::create(*manager, Deleter());
	get().setStateAtomically(GlObject::State::TO_BE_CREATED);

	cmd.get().pushBackNewCommand<OqCreateCommand>(*this, condRenderingBit);
	cmd.flush();

	return ErrorCode::NONE;
}

//==============================================================================
void OcclusionQueryHandle::begin(CommandBufferHandle& commands)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<OqBeginCommand>(*this);
}

//==============================================================================
void OcclusionQueryHandle::end(CommandBufferHandle& commands)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<OqEndCommand>(*this);
}

} // end namespace anki


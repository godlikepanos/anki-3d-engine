// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/OcclusionQueryPtr.h"
#include "anki/gr/gl/OcclusionQueryImpl.h"
#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/CommandBufferPtr.h"

namespace anki {

//==============================================================================
// Commands                                                                    =
//==============================================================================

/// Create command.
class OqCreateCommand final: public GlCommand
{
public:
	OcclusionQueryPtr m_handle;
	OcclusionQueryResultBit m_condRenderingBit;

	OqCreateCommand(OcclusionQueryPtr& handle,
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
	OcclusionQueryPtr m_handle;

	OqBeginCommand(OcclusionQueryPtr& handle)
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
	OcclusionQueryPtr m_handle;

	OqEndCommand(OcclusionQueryPtr& handle)
	:	m_handle(handle)
	{}

	Error operator()(CommandBufferImpl*)
	{
		m_handle.get().end();
		return ErrorCode::NONE;
	}
};

//==============================================================================
// OcclusionQueryPtr                                                        =
//==============================================================================

//==============================================================================
OcclusionQueryPtr::OcclusionQueryPtr()
{}

//==============================================================================
OcclusionQueryPtr::~OcclusionQueryPtr()
{}

//==============================================================================
Error OcclusionQueryPtr::create(
	GrManager* manager, ResultBit condRenderingBit)
{
	CommandBufferPtr cmd;
	ANKI_CHECK(cmd.create(manager));

	Base::create(*manager);
	get().setStateAtomically(GlObject::State::TO_BE_CREATED);

	cmd.get().pushBackNewCommand<OqCreateCommand>(*this, condRenderingBit);
	cmd.flush();

	return ErrorCode::NONE;
}

//==============================================================================
void OcclusionQueryPtr::begin(CommandBufferPtr& commands)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<OqBeginCommand>(*this);
}

//==============================================================================
void OcclusionQueryPtr::end(CommandBufferPtr& commands)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<OqEndCommand>(*this);
}

} // end namespace anki


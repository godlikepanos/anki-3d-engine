// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/GlObject.h"
#include "anki/gr/GrManager.h"
#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/gl/RenderingThread.h"
#include "anki/gr/CommandBufferPtr.h"
#include "anki/gr/gl/CommandBufferImpl.h"

namespace anki {

//==============================================================================
Error GlObject::serializeOnGetter() const
{
	Error err = ErrorCode::NONE;
	State state = State(m_state.load());
	ANKI_ASSERT(state != State::NEW);

	if(state == State::TO_BE_CREATED)
	{
		RenderingThread& thread = const_cast<RenderingThread&>(
			getManager().getImplementation().getRenderingThread());
		thread.syncClientServer();

		state = State(m_state.load());

		if(state == State::ERROR)
		{
			err = ErrorCode::UNKNOWN;
		}
	}

	ANKI_ASSERT(state > State::TO_BE_CREATED);

	return err;
}

//==============================================================================
class DeleteGlObjectCommand final: public GlCommand
{
public:
	GlObject::GlDeleteFunction m_callback;
	GLuint m_glName;

	DeleteGlObjectCommand(GlObject::GlDeleteFunction callback, GLuint name)
	:	m_callback(callback),
		m_glName(name)
	{}

	Error operator()(CommandBufferImpl*)
	{
		m_callback(1, &m_glName);
		return ErrorCode::NONE;
	}
};

void GlObject::destroyDeferred(GlDeleteFunction deleteCallback)
{
	GrManager& manager = getManager();
	RenderingThread& thread = manager.getImplementation().getRenderingThread();

	if(!thread.isServerThread())
	{
		CommandBufferPtr commands;

		commands.create(&manager);
		commands.get().template pushBackNewCommand<DeleteGlObjectCommand>(
			deleteCallback, m_glName);
		commands.flush();
	}
	else
	{
		deleteCallback(1, &m_glName);
	}

	m_glName = 0;
}

} // end namespace anki



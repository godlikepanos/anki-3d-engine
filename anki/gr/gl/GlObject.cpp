// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/GlObject.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/gl/CommandBufferImpl.h>

namespace anki
{

GlObject::GlObject()
	: m_glName(0)
	, m_state(I32(State::TO_BE_CREATED))
{
}

Error GlObject::serializeRenderingThread(GrManager& manager)
{
	Error err = Error::NONE;
	State state = State(m_state.load());
	ANKI_ASSERT(state != State::NEW);

	if(state == State::TO_BE_CREATED)
	{
		RenderingThread& thread = static_cast<GrManagerImpl&>(manager).getRenderingThread();
		thread.syncClientServer();

		state = State(m_state.load());

		if(state == State::ERROR)
		{
			err = Error::UNKNOWN;
		}
	}

	ANKI_ASSERT(state > State::TO_BE_CREATED);

	return err;
}

void GlObject::destroyDeferred(GrManager& manager, GlDeleteFunction deleteCallback)
{
	class DeleteGlObjectCommand final : public GlCommand
	{
	public:
		GlObject::GlDeleteFunction m_callback;
		GLuint m_glName;

		DeleteGlObjectCommand(GlObject::GlDeleteFunction callback, GLuint name)
			: m_callback(callback)
			, m_glName(name)
		{
		}

		Error operator()(GlState&)
		{
			m_callback(1, &m_glName);
			return Error::NONE;
		}
	};

	if(m_glName == 0)
	{
		return;
	}

	RenderingThread& thread = static_cast<GrManagerImpl&>(manager).getRenderingThread();

	if(!thread.isServerThread())
	{
		CommandBufferPtr commands;

		commands = manager.newCommandBuffer(CommandBufferInitInfo());
		static_cast<CommandBufferImpl&>(*commands).pushBackNewCommand<DeleteGlObjectCommand>(deleteCallback, m_glName);
		static_cast<CommandBufferImpl&>(*commands).flush();
	}
	else
	{
		deleteCallback(1, &m_glName);
	}

	m_glName = 0;
}

} // end namespace anki

// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

GlObject::GlObject(GrManager* manager)
	: m_manager(manager)
	, m_glName(0)
	, m_state(I32(State::TO_BE_CREATED))
{
}

Error GlObject::serializeRenderingThread()
{
	Error err = ErrorCode::NONE;
	State state = State(m_state.load());
	ANKI_ASSERT(state != State::NEW);

	if(state == State::TO_BE_CREATED)
	{
		RenderingThread& thread = m_manager->getImplementation().getRenderingThread();
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
		return ErrorCode::NONE;
	}
};

void GlObject::destroyDeferred(GlDeleteFunction deleteCallback)
{
	if(m_glName == 0)
	{
		return;
	}

	GrManager& manager = getManager();
	RenderingThread& thread = manager.getImplementation().getRenderingThread();

	if(!thread.isServerThread())
	{
		CommandBufferPtr commands;

		commands = manager.newInstance<CommandBuffer>(CommandBufferInitInfo());
		commands->m_impl->pushBackNewCommand<DeleteGlObjectCommand>(deleteCallback, m_glName);
		commands->flush();
	}
	else
	{
		deleteCallback(1, &m_glName);
	}

	m_glName = 0;
}

GrAllocator<U8> GlObject::getAllocator() const
{
	return m_manager->getAllocator();
}

} // end namespace anki

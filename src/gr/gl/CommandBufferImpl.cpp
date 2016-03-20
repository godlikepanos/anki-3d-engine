// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/gl/Error.h>
#include <anki/util/Logger.h>
#include <anki/core/Trace.h>
#include <cstring>

namespace anki
{

//==============================================================================
void CommandBufferImpl::init(const InitHints& hints)
{
	auto& pool = m_manager->getAllocator().getMemoryPool();

	m_alloc = CommandBufferAllocator<GlCommand*>(pool.getAllocationCallback(),
		pool.getAllocationCallbackUserData(),
		hints.m_chunkSize,
		1.0,
		0,
		false);
}

//==============================================================================
void CommandBufferImpl::destroy()
{
	ANKI_TRACE_START_EVENT(GL_CMD_BUFFER_DESTROY);

#if ANKI_DEBUG
	if(!m_executed && m_firstCommand)
	{
		ANKI_LOGW("Chain contains commands but never executed. "
				  "This should only happen on exceptions");
	}
#endif

	GlCommand* command = m_firstCommand;
	while(command != nullptr)
	{
		GlCommand* next = command->m_nextCommand; // Get next before deleting
		m_alloc.deleteInstance(command);
		command = next;
	}

	ANKI_ASSERT(m_alloc.getMemoryPool().getUsersCount() == 1
		&& "Someone is holding a reference to the command buffer's allocator");

	m_alloc = CommandBufferAllocator<U8>();

	ANKI_TRACE_STOP_EVENT(GL_CMD_BUFFER_DESTROY);
}

//==============================================================================
Error CommandBufferImpl::executeAllCommands()
{
	ANKI_ASSERT(m_firstCommand != nullptr && "Empty command buffer");
	ANKI_ASSERT(m_lastCommand != nullptr && "Empty command buffer");
#if ANKI_DEBUG
	m_executed = true;
#endif

	Error err = ErrorCode::NONE;
	GlState& state =
		m_manager->getImplementation().getRenderingThread().getState();

	GlCommand* command = m_firstCommand;

	while(command != nullptr && !err)
	{
		err = (*command)(state);
		ANKI_CHECK_GL_ERROR();

		command = command->m_nextCommand;
	}

	return err;
}

//==============================================================================
CommandBufferImpl::InitHints CommandBufferImpl::computeInitHints() const
{
	InitHints out;
	out.m_chunkSize = m_alloc.getMemoryPool().getMemoryCapacity();

	return out;
}

//==============================================================================
GrAllocator<U8> CommandBufferImpl::getAllocator() const
{
	return m_manager->getAllocator();
}

} // end namespace anki

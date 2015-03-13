// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/gl/Queue.h"
#include "anki/gr/GlDevice.h"
#include "anki/gr/GlError.h"
#include "anki/util/Logger.h"
#include "anki/core/Counters.h"
#include <cstring>

namespace anki {

//==============================================================================
Error CommandBufferImpl::create(Queue* server, 
	const CommandBufferImplInitHints& hints)
{
	ANKI_ASSERT(server);

	m_server = server;

	m_alloc = GlCommandBufferAllocator<GlCommand*>(
		m_server->getAllocationCallback(),
		m_server->getAllocationCallbackUserData(),
		hints.m_chunkSize, 
		CommandBufferImplInitHints::MAX_CHUNK_SIZE, 
		ChainMemoryPool::ChunkGrowMethod::ADD,
		hints.m_chunkSize);

	return ErrorCode::NONE;
}

//==============================================================================
void CommandBufferImpl::destroy()
{
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

	m_alloc = GlCommandBufferAllocator<U8>();
}

//==============================================================================
GlAllocator<U8> CommandBufferImpl::getGlobalAllocator() const
{
	return m_server->getDevice()._getAllocator();
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

	GlCommand* command = m_firstCommand;

	while(command != nullptr && !err)
	{
		err = (*command)(this);
		ANKI_CHECK_GL_ERROR();

		command = command->m_nextCommand;
	}

	return err;
}

//==============================================================================
CommandBufferImplInitHints CommandBufferImpl::computeInitHints() const
{
	CommandBufferImplInitHints out;
	out.m_chunkSize = m_alloc.getMemoryPool().getAllocatedSize() + 16;

	ANKI_COUNTER_INC(GL_QUEUES_SIZE, 
		U64(m_alloc.getMemoryPool().getAllocatedSize()));

	return out;
}

} // end namespace anki

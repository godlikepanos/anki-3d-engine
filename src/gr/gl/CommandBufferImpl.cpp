// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/GrManager.h"
#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/gl/RenderingThread.h"
#include "anki/gr/gl/Error.h"
#include "anki/util/Logger.h"
#include "anki/core/Counters.h"
#include <cstring>

namespace anki {

//==============================================================================
void CommandBufferImpl::create(const InitHints& hints)
{
	auto& pool = m_manager->getAllocator().getMemoryPool();

	m_alloc = CommandBufferAllocator<GlCommand*>(
		pool.getAllocationCallback(),
		pool.getAllocationCallbackUserData(),
		hints.m_chunkSize,
		1.0,
		hints.m_chunkSize);
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

	m_alloc = CommandBufferAllocator<U8>();
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
	out.m_chunkSize = m_alloc.getMemoryPool().getAllocatedSize() + 16;

	ANKI_COUNTER_INC(GL_QUEUES_SIZE,
		U64(m_alloc.getMemoryPool().getAllocatedSize()));

	return out;
}

} // end namespace anki

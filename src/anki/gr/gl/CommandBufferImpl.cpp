// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/GlState.h>
#include <anki/gr/gl/Error.h>

#include <anki/gr/OcclusionQuery.h>
#include <anki/gr/gl/OcclusionQueryImpl.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/gl/BufferImpl.h>

#include <anki/util/Logger.h>
#include <anki/core/Trace.h>
#include <cstring>

namespace anki
{

void CommandBufferImpl::init(const CommandBufferInitInfo& init)
{
	auto& pool = m_manager->getAllocator().getMemoryPool();

	m_alloc = CommandBufferAllocator<GlCommand*>(
		pool.getAllocationCallback(), pool.getAllocationCallbackUserData(), init.m_hints.m_chunkSize, 1.0, 0, false);

	m_flags = init.m_flags;
}

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

Error CommandBufferImpl::executeAllCommands()
{
	ANKI_ASSERT(m_firstCommand != nullptr && "Empty command buffer");
	ANKI_ASSERT(m_lastCommand != nullptr && "Empty command buffer");
#if ANKI_DEBUG
	m_executed = true;
#endif

	Error err = ErrorCode::NONE;
	GlState& state = m_manager->getImplementation().getState();

	GlCommand* command = m_firstCommand;

	while(command != nullptr && !err)
	{
		err = (*command)(state);
		ANKI_CHECK_GL_ERROR();

		command = command->m_nextCommand;
	}

	return err;
}

CommandBufferImpl::InitHints CommandBufferImpl::computeInitHints() const
{
	InitHints out;
	out.m_chunkSize = m_alloc.getMemoryPool().getMemoryCapacity();

	return out;
}

GrAllocator<U8> CommandBufferImpl::getAllocator() const
{
	return m_manager->getAllocator();
}

void CommandBufferImpl::flushDrawcall()
{
	class Cmd final : public GlCommand
	{
	public:
		GLenum m_face;
		GLenum m_func;
		GLint m_ref;
		GLuint m_compareMask;

		Cmd(GLenum face, GLenum func, GLint ref, GLuint mask)
			: m_face(face)
			, m_func(func)
			, m_ref(ref)
			, m_compareMask(mask)
		{
		}

		Error operator()(GlState&)
		{
			glStencilFuncSeparate(m_face, m_func, m_ref, m_compareMask);
			return ErrorCode::NONE;
		}
	};

	const Array<GLenum, 2> FACE = {{GL_FRONT, GL_BACK}};
	for(U i = 0; i < 2; ++i)
	{
		if(m_state.m_glStencilFuncSeparateDirty[i])
		{
			pushBackNewCommand<Cmd>(FACE[i],
				convertCompareOperation(m_state.m_stencilCompare[i]),
				m_state.m_stencilRef[i],
				m_state.m_stencilCompareMask[i]);

			m_state.m_glStencilFuncSeparateDirty[i] = false;
		}
	}
}

} // end namespace anki

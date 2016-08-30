// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/GlState.h>
#include <anki/gr/gl/Error.h>

#include <anki/gr/gl/ResourceGroupImpl.h>
#include <anki/gr/OcclusionQuery.h>
#include <anki/gr/gl/OcclusionQueryImpl.h>

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

class BindResourcesCommand final : public GlCommand
{
public:
	ResourceGroupPtr m_rc;
	TransientMemoryInfo m_info;
	U8 m_slot;

	BindResourcesCommand(ResourceGroupPtr rc, U8 slot, const TransientMemoryInfo* info)
		: m_rc(rc)
		, m_slot(slot)
	{
		if(info)
		{
			m_info = *info;
		}
	}

	Error operator()(GlState& state)
	{
		ANKI_TRACE_START_EVENT(GL_BIND_RESOURCES);
		m_rc->getImplementation().bind(m_slot, m_info, state);
		ANKI_TRACE_STOP_EVENT(GL_BIND_RESOURCES);
		return ErrorCode::NONE;
	}
};

void CommandBufferImpl::bindResourceGroup(ResourceGroupPtr rc, U slot, const TransientMemoryInfo* info)
{
	ANKI_ASSERT(rc.isCreated());

	pushBackNewCommand<BindResourcesCommand>(rc, slot, info);
}

class DrawElementsCondCommand : public GlCommand
{
public:
	DrawElementsIndirectInfo m_info;
	OcclusionQueryPtr m_query;

	DrawElementsCondCommand(const DrawElementsIndirectInfo& info, OcclusionQueryPtr query = OcclusionQueryPtr())
		: m_info(info)
		, m_query(query)
	{
	}

	Error operator()(GlState& state)
	{
		GLenum indicesType = 0;
		switch(state.m_indexSize)
		{
		case 2:
			indicesType = GL_UNSIGNED_SHORT;
			break;
		case 4:
			indicesType = GL_UNSIGNED_INT;
			break;
		default:
			ANKI_ASSERT(0);
			break;
		};

		if(!m_query.isCreated() || !m_query->getImplementation().skipDrawcall())
		{
			state.flushVertexState();

			glDrawElementsInstancedBaseVertexBaseInstance(state.m_topology,
				m_info.m_count,
				indicesType,
				(const void*)(PtrSize)(m_info.m_firstIndex * state.m_indexSize),
				m_info.m_instanceCount,
				m_info.m_baseVertex,
				m_info.m_baseInstance);

			ANKI_TRACE_INC_COUNTER(GR_DRAWCALLS, 1);
			ANKI_TRACE_INC_COUNTER(GR_VERTICES, m_info.m_instanceCount * m_info.m_count);
		}

		return ErrorCode::NONE;
	}
};

void CommandBufferImpl::drawElements(U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	ANKI_ASSERT(m_dbg.m_insideRenderPass);
	DrawElementsIndirectInfo info(count, instanceCount, firstIndex, baseVertex, baseInstance);

	checkDrawcall();
	pushBackNewCommand<DrawElementsCondCommand>(info);
}

class DrawArraysCondCommand final : public GlCommand
{
public:
	DrawArraysIndirectInfo m_info;
	OcclusionQueryPtr m_query;

	DrawArraysCondCommand(const DrawArraysIndirectInfo& info, OcclusionQueryPtr query = OcclusionQueryPtr())
		: m_info(info)
		, m_query(query)
	{
	}

	Error operator()(GlState& state)
	{
		if(!m_query.isCreated() || !m_query->getImplementation().skipDrawcall())
		{
			state.flushVertexState();

			glDrawArraysInstancedBaseInstance(
				state.m_topology, m_info.m_first, m_info.m_count, m_info.m_instanceCount, m_info.m_baseInstance);

			ANKI_TRACE_INC_COUNTER(GR_DRAWCALLS, 1);
		}

		return ErrorCode::NONE;
	}
};

void CommandBufferImpl::drawArrays(U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	ANKI_ASSERT(m_dbg.m_insideRenderPass);
	DrawArraysIndirectInfo info(count, instanceCount, first, baseInstance);

	checkDrawcall();
	pushBackNewCommand<DrawArraysCondCommand>(info);
}

void CommandBufferImpl::drawElementsConditional(
	OcclusionQueryPtr query, U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	ANKI_ASSERT(m_dbg.m_insideRenderPass);
	DrawElementsIndirectInfo info(count, instanceCount, firstIndex, baseVertex, baseInstance);

	checkDrawcall();
	pushBackNewCommand<DrawElementsCondCommand>(info, query);
}

void CommandBufferImpl::drawArraysConditional(
	OcclusionQueryPtr query, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	ANKI_ASSERT(m_dbg.m_insideRenderPass);
	DrawArraysIndirectInfo info(count, instanceCount, first, baseInstance);

	checkDrawcall();
	pushBackNewCommand<DrawArraysCondCommand>(info, query);
}

class DispatchCommand final : public GlCommand
{
public:
	Array<U32, 3> m_size;

	DispatchCommand(U32 x, U32 y, U32 z)
		: m_size({{x, y, z}})
	{
	}

	Error operator()(GlState&)
	{
		glDispatchCompute(m_size[0], m_size[1], m_size[2]);
		return ErrorCode::NONE;
	}
};

void CommandBufferImpl::dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_ASSERT(!m_dbg.m_insideRenderPass);
	pushBackNewCommand<DispatchCommand>(groupCountX, groupCountY, groupCountZ);
}

} // end namespace anki

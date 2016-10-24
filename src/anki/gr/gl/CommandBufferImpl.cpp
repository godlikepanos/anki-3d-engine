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
		m_rc->m_impl->bind(m_slot, m_info, state);
		ANKI_TRACE_STOP_EVENT(GL_BIND_RESOURCES);
		return ErrorCode::NONE;
	}
};

void CommandBufferImpl::bindResourceGroup(ResourceGroupPtr rc, U slot, const TransientMemoryInfo* info)
{
	ANKI_ASSERT(rc.isCreated());

	pushBackNewCommand<BindResourcesCommand>(rc, slot, info);
}

void CommandBufferImpl::drawElements(U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	class DrawElementsCommand : public GlCommand
	{
	public:
		DrawElementsIndirectInfo m_info;

		DrawElementsCommand(const DrawElementsIndirectInfo& info)
			: m_info(info)
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

			state.flushVertexState();
			state.flushStencilState();
			glDrawElementsInstancedBaseVertexBaseInstance(state.m_topology,
				m_info.m_count,
				indicesType,
				(const void*)(PtrSize)(m_info.m_firstIndex * state.m_indexSize),
				m_info.m_instanceCount,
				m_info.m_baseVertex,
				m_info.m_baseInstance);

			ANKI_TRACE_INC_COUNTER(GR_DRAWCALLS, 1);
			ANKI_TRACE_INC_COUNTER(GR_VERTICES, m_info.m_instanceCount * m_info.m_count);

			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(m_dbg.m_insideRenderPass);
	DrawElementsIndirectInfo info(count, instanceCount, firstIndex, baseVertex, baseInstance);

	checkDrawcall();
	pushBackNewCommand<DrawElementsCommand>(info);
}

void CommandBufferImpl::drawArrays(U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	class DrawArraysCommand final : public GlCommand
	{
	public:
		DrawArraysIndirectInfo m_info;

		DrawArraysCommand(const DrawArraysIndirectInfo& info)
			: m_info(info)
		{
		}

		Error operator()(GlState& state)
		{
			state.flushVertexState();
			state.flushStencilState();
			glDrawArraysInstancedBaseInstance(
				state.m_topology, m_info.m_first, m_info.m_count, m_info.m_instanceCount, m_info.m_baseInstance);

			ANKI_TRACE_INC_COUNTER(GR_DRAWCALLS, 1);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(m_dbg.m_insideRenderPass);
	DrawArraysIndirectInfo info(count, instanceCount, first, baseInstance);

	checkDrawcall();
	pushBackNewCommand<DrawArraysCommand>(info);
}

void CommandBufferImpl::drawElementsIndirect(U32 drawCount, PtrSize offset, BufferPtr indirectBuff)
{
	class DrawElementsIndirectCommand final : public GlCommand
	{
	public:
		U32 m_drawCount;
		PtrSize m_offset;
		BufferPtr m_buff;

		DrawElementsIndirectCommand(U32 drawCount, PtrSize offset, BufferPtr buff)
			: m_drawCount(drawCount)
			, m_offset(offset)
			, m_buff(buff)
		{
			ANKI_ASSERT(drawCount > 0);
			ANKI_ASSERT((m_offset % 4) == 0);
		}

		Error operator()(GlState& state)
		{
			state.flushVertexState();
			state.flushStencilState();
			const BufferImpl& buff = *m_buff->m_impl;

			ANKI_ASSERT(m_offset + sizeof(DrawElementsIndirectInfo) * m_drawCount <= buff.m_size);

			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buff.getGlName());

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

			glMultiDrawElementsIndirect(state.m_topology,
				indicesType,
				numberToPtr<void*>(m_offset),
				m_drawCount,
				sizeof(DrawElementsIndirectInfo));

			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
			return ErrorCode::NONE;
		}
	};

	checkDrawcall();
	pushBackNewCommand<DrawElementsIndirectCommand>(drawCount, offset, indirectBuff);
}

void CommandBufferImpl::drawArraysIndirect(U32 drawCount, PtrSize offset, BufferPtr indirectBuff)
{
	class DrawArraysIndirectCommand final : public GlCommand
	{
	public:
		U32 m_drawCount;
		PtrSize m_offset;
		BufferPtr m_buff;

		DrawArraysIndirectCommand(U32 drawCount, PtrSize offset, BufferPtr buff)
			: m_drawCount(drawCount)
			, m_offset(offset)
			, m_buff(buff)
		{
			ANKI_ASSERT(drawCount > 0);
			ANKI_ASSERT((m_offset % 4) == 0);
		}

		Error operator()(GlState& state)
		{
			state.flushVertexState();
			state.flushStencilState();
			const BufferImpl& buff = *m_buff->m_impl;

			ANKI_ASSERT(m_offset + sizeof(DrawArraysIndirectInfo) * m_drawCount <= buff.m_size);

			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buff.getGlName());

			glMultiDrawArraysIndirect(
				state.m_topology, numberToPtr<void*>(m_offset), m_drawCount, sizeof(DrawArraysIndirectInfo));

			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
			return ErrorCode::NONE;
		}
	};

	checkDrawcall();
	pushBackNewCommand<DrawArraysIndirectCommand>(drawCount, offset, indirectBuff);
}

void CommandBufferImpl::dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_ASSERT(!m_dbg.m_insideRenderPass);

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

	pushBackNewCommand<DispatchCommand>(groupCountX, groupCountY, groupCountZ);
}

} // end namespace anki

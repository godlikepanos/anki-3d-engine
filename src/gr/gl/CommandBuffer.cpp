// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/CommandBuffer.h"
#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/GrManager.h"
#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/gl/RenderingThread.h"
#include "anki/gr/Pipeline.h"
#include "anki/gr/gl/PipelineImpl.h"
#include "anki/gr/Framebuffer.h"
#include "anki/gr/gl/FramebufferImpl.h"
#include "anki/gr/ResourceGroup.h"
#include "anki/gr/gl/ResourceGroupImpl.h"
#include "anki/gr/OcclusionQuery.h"
#include "anki/gr/gl/OcclusionQueryImpl.h"
#include "anki/gr/Texture.h"
#include "anki/gr/gl/TextureImpl.h"
#include "anki/gr/Buffer.h"
#include "anki/gr/gl/BufferImpl.h"
#include "anki/core/Counters.h"

namespace anki {

//==============================================================================
CommandBuffer::CommandBuffer(GrManager* manager)
	: GrObject(manager)
{}

//==============================================================================
CommandBuffer::~CommandBuffer()
{}

//==============================================================================
void CommandBuffer::create(CommandBufferInitHints hints)
{
	m_impl.reset(getAllocator().newInstance<CommandBufferImpl>(&getManager()));
	m_impl->create(hints);
}

//==============================================================================
void CommandBuffer::flush()
{
	getManager().getImplementation().
		getRenderingThread().flushCommandBuffer(this);
}

//==============================================================================
void CommandBuffer::finish()
{
	getManager().getImplementation().
		getRenderingThread().finishCommandBuffer(this);
}

//==============================================================================
class ViewportCommand final: public GlCommand
{
public:
	Array<U16, 4> m_value;

	ViewportCommand(U16 a, U16 b, U16 c, U16 d)
	{
		m_value = {{a, b, c, d}};
	}

	Error operator()(GlState& state)
	{
		if(state.m_viewport[0] != m_value[0]
			|| state.m_viewport[1] != m_value[1]
			|| state.m_viewport[2] != m_value[2]
			|| state.m_viewport[3] != m_value[3])
		{
			glViewport(m_value[0], m_value[1], m_value[2], m_value[3]);

			state.m_viewport = m_value;
		}

		return ErrorCode::NONE;
	}
};

void CommandBuffer::setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy)
{
	m_impl->pushBackNewCommand<ViewportCommand>(minx, miny, maxx, maxy);
}

//==============================================================================
class BindPipelineCommand final: public GlCommand
{
public:
	PipelinePtr m_ppline;

	BindPipelineCommand(PipelinePtr& ppline)
		: m_ppline(ppline)
	{}

	Error operator()(GlState& state)
	{
		PipelineImpl& impl = m_ppline->getImplementation();

		auto name = impl.getGlName();
		if(state.m_crntPpline != name)
		{
			impl.bind(state);
			state.m_crntPpline = name;
		}

		return ErrorCode::NONE;
	}
};

void CommandBuffer::bindPipeline(PipelinePtr ppline)
{
	m_impl->pushBackNewCommand<BindPipelineCommand>(ppline);
}

//==============================================================================
class BindFramebufferCommand final: public GlCommand
{
public:
	FramebufferPtr m_fb;

	BindFramebufferCommand(FramebufferPtr fb)
		: m_fb(fb)
	{}

	Error operator()(GlState& state)
	{
		m_fb->getImplementation().bind(state);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::bindFramebuffer(FramebufferPtr fb)
{
	m_impl->pushBackNewCommand<BindFramebufferCommand>(fb);
}

//==============================================================================
class BindResourcesCommand final: public GlCommand
{
public:
	ResourceGroupPtr m_rc;
	U8 m_slot;

	BindResourcesCommand(ResourceGroupPtr rc, U8 slot)
		: m_rc(rc)
		, m_slot(slot)
	{}

	Error operator()(GlState& state)
	{
		m_rc->getImplementation().bind(m_slot, state);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::bindResourceGroup(ResourceGroupPtr rc, U slot)
{
	ANKI_ASSERT(rc.isCreated());
	m_impl->pushBackNewCommand<BindResourcesCommand>(rc, slot);
}

//==============================================================================
class DrawElementsCondCommand: public GlCommand
{
public:
	DrawElementsIndirectInfo m_info;
	OcclusionQueryPtr m_query;

	DrawElementsCondCommand(
		const DrawElementsIndirectInfo& info,
		OcclusionQueryPtr query = OcclusionQueryPtr())
		: m_info(info)
		, m_query(query)
	{}

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
			glDrawElementsInstancedBaseVertexBaseInstance(
				state.m_topology,
				m_info.m_count,
				indicesType,
				(const void*)(PtrSize)(m_info.m_firstIndex * state.m_indexSize),
				m_info.m_instanceCount,
				m_info.m_baseVertex,
				m_info.m_baseInstance);

			ANKI_COUNTER_INC(GL_DRAWCALLS_COUNT, U64(1));
			ANKI_COUNTER_INC(GL_VERTICES_COUNT,
				U64(m_info.m_instanceCount * m_info.m_count));
		}

		return ErrorCode::NONE;
	}
};

void CommandBuffer::drawElements(U32 count, U32 instanceCount, U32 firstIndex,
	U32 baseVertex, U32 baseInstance)
{
	DrawElementsIndirectInfo info(count, instanceCount, firstIndex,
		baseVertex, baseInstance);

	m_impl->pushBackNewCommand<DrawElementsCondCommand>(info);
}

//==============================================================================
class DrawArraysCondCommand final: public GlCommand
{
public:
	DrawArraysIndirectInfo m_info;
	OcclusionQueryPtr m_query;

	DrawArraysCondCommand(
		const DrawArraysIndirectInfo& info,
		OcclusionQueryPtr query = OcclusionQueryPtr())
		: m_info(info)
		, m_query(query)
	{}

	Error operator()(GlState& state)
	{
		if(!m_query.isCreated() || !m_query->getImplementation().skipDrawcall())
		{
			glDrawArraysInstancedBaseInstance(
				state.m_topology,
				m_info.m_first,
				m_info.m_count,
				m_info.m_instanceCount,
				m_info.m_baseInstance);

			ANKI_COUNTER_INC(GL_DRAWCALLS_COUNT, U64(1));
		}

		return ErrorCode::NONE;
	}
};

void CommandBuffer::drawArrays(U32 count, U32 instanceCount, U32 first,
	U32 baseInstance)
{
	DrawArraysIndirectInfo info(count, instanceCount, first, baseInstance);

	m_impl->pushBackNewCommand<DrawArraysCondCommand>(info);
}

//==============================================================================
void CommandBuffer::drawElementsConditional(OcclusionQueryPtr query, U32 count,
	U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	DrawElementsIndirectInfo info(count, instanceCount, firstIndex,
		baseVertex, baseInstance);

	m_impl->pushBackNewCommand<DrawElementsCondCommand>(info, query);
}

//==============================================================================
void CommandBuffer::drawArraysConditional(OcclusionQueryPtr query, U32 count,
	U32 instanceCount, U32 first, U32 baseInstance)
{
	DrawArraysIndirectInfo info(count, instanceCount, first, baseInstance);

	m_impl->pushBackNewCommand<DrawArraysCondCommand>(info, query);
}

//==============================================================================
class DispatchCommand final: public GlCommand
{
public:
	Array<U32, 3> m_size;

	DispatchCommand(U32 x, U32 y, U32 z)
		: m_size({x, y, z})
	{}

	Error operator()(GlState&)
	{
		glDispatchCompute(m_size[0], m_size[1], m_size[2]);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::dispatchCompute(
	U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	m_impl->pushBackNewCommand<DispatchCommand>(
		groupCountX, groupCountY, groupCountZ);
}

//==============================================================================
class UpdateUniformsCommand final: public GlCommand
{
public:
	GLuint m_uboName;
	U32 m_offset;
	U16 m_range;

	UpdateUniformsCommand(GLuint ubo, U32 offset, U16 range)
		: m_uboName(ubo)
		, m_offset(offset)
		, m_range(range)
	{}

	Error operator()(GlState&)
	{
		const U binding = 0;
		glBindBufferRange(
			GL_UNIFORM_BUFFER, binding, m_uboName, m_offset, m_range);

		return ErrorCode::NONE;
	}
};

void CommandBuffer::updateDynamicUniformsInternal(U32 originalSize, void*& data)
{
	ANKI_ASSERT(originalSize > 0);
	ANKI_ASSERT(originalSize <= 1024 * 8 && "Too high?");

	// Will be used in a thread safe way
	GlState& state =
		getManager().getImplementation().getRenderingThread().getState();

	const U uboSize = state.m_globalUboSize;
	const U subUboSize = GlState::MAX_UBO_SIZE;

	// Get offset in the contiguous buffer
	U size = getAlignedRoundUp(state.m_uniBuffOffsetAlignment, originalSize);
	U offset = state.m_globalUboCurrentOffset.fetchAdd(size);
	offset = offset % uboSize;

	while((offset % subUboSize) + size > subUboSize)
	{
		// Update area will fall between UBOs, need to start over
		offset = state.m_globalUboCurrentOffset.fetchAdd(size);
		offset = offset % uboSize;
	}

	ANKI_ASSERT(isAligned(state.m_uniBuffOffsetAlignment, offset));
	ANKI_ASSERT(offset + size <= uboSize);

	// Get actual UBO address to write
	U uboIdx = offset / subUboSize;
	U subUboOffset = offset % subUboSize;
	ANKI_ASSERT(isAligned(state.m_uniBuffOffsetAlignment, subUboOffset));

	U8* addressToWrite = state.m_globalUboAddresses[uboIdx] + subUboOffset;

	data = static_cast<void*>(addressToWrite);

	// Push bind command
	m_impl->pushBackNewCommand<UpdateUniformsCommand>(
		state.m_globalUbos[uboIdx], subUboOffset, originalSize);
}

//==============================================================================
class OqBeginCommand final: public GlCommand
{
public:
	OcclusionQueryPtr m_handle;

	OqBeginCommand(const OcclusionQueryPtr& handle)
		: m_handle(handle)
	{}

	Error operator()(GlState&)
	{
		m_handle->getImplementation().begin();
		return ErrorCode::NONE;
	}
};

void CommandBuffer::beginOcclusionQuery(OcclusionQueryPtr query)
{
	m_impl->pushBackNewCommand<OqBeginCommand>(query);
}

//==============================================================================
class OqEndCommand final: public GlCommand
{
public:
	OcclusionQueryPtr m_handle;

	OqEndCommand(const OcclusionQueryPtr& handle)
		: m_handle(handle)
	{}

	Error operator()(GlState&)
	{
		m_handle->getImplementation().end();
		return ErrorCode::NONE;
	}
};

void CommandBuffer::endOcclusionQuery(OcclusionQueryPtr query)
{
	m_impl->pushBackNewCommand<OqEndCommand>(query);
}

//==============================================================================
class TexUploadCommand final: public GlCommand
{
public:
	TexturePtr m_handle;
	U32 m_mipmap;
	U32 m_slice;
	PtrSize m_dataSize;
	void* m_data;
	CommandBufferAllocator<U8> m_alloc; ///< Alloc that was used for m_data.

	TexUploadCommand(const TexturePtr& handle, U32 mipmap, U32 slice,
		PtrSize dataSize, void* data, const CommandBufferAllocator<U8>& alloc)
		: m_handle(handle)
		, m_mipmap(mipmap)
		, m_slice(slice)
		, m_dataSize(dataSize)
		, m_data(data)
		, m_alloc(alloc)
	{}

	Error operator()(GlState&)
	{
		m_handle->getImplementation().write(
			m_mipmap, m_slice, m_data, m_dataSize);

		// Cleanup to avoid warnings
		m_alloc.deallocate(m_data, 1);

		return ErrorCode::NONE;
	}
};

void CommandBuffer::textureUploadInternal(TexturePtr tex, U32 mipmap, U32 slice,
	PtrSize dataSize, void*& data)
{
	ANKI_ASSERT(dataSize > 0);

	// Allocate memory to write
	data = m_impl->getInternalAllocator().allocate(dataSize);

	m_impl->pushBackNewCommand<TexUploadCommand>(
		tex, mipmap, slice, dataSize, data, m_impl->getInternalAllocator());
}

//==============================================================================
class BuffWriteCommand final: public GlCommand
{
public:
	BufferPtr m_handle;
	PtrSize m_offset;
	PtrSize m_range;
	void* m_data;
	CommandBufferAllocator<U8> m_alloc;

	BuffWriteCommand(const BufferPtr& handle, PtrSize offset, PtrSize range,
		void* data, const CommandBufferAllocator<U8>& alloc)
		: m_handle(handle)
		, m_offset(offset)
		, m_range(range)
		, m_data(data)
		, m_alloc(alloc)
	{}

	Error operator()(GlState&)
	{
		m_handle->getImplementation().write(m_data, m_offset, m_range);

		// Cleanup to avoid warnings
		m_alloc.deallocate(m_data, 1);

		return ErrorCode::NONE;
	}
};

void CommandBuffer::writeBufferInternal(BufferPtr buff, PtrSize offset,
	PtrSize range, void*& data)
{
	ANKI_ASSERT(range > 0);

	// Allocate memory to write
	data = m_impl->getInternalAllocator().allocate(range);

	m_impl->pushBackNewCommand<BuffWriteCommand>(
		buff, offset, range, data, m_impl->getInternalAllocator());
}

//==============================================================================
class GenMipsCommand final: public GlCommand
{
public:
	TexturePtr m_tex;

	GenMipsCommand(const TexturePtr& tex)
		: m_tex(tex)
	{}

	Error operator()(GlState&)
	{
		m_tex->getImplementation().generateMipmaps();
		return ErrorCode::NONE;
	}
};

void CommandBuffer::generateMipmaps(TexturePtr tex)
{
	m_impl->pushBackNewCommand<GenMipsCommand>(tex);
}

//==============================================================================
CommandBufferInitHints CommandBuffer::computeInitHints() const
{
	return m_impl->computeInitHints();
}

//==============================================================================
class ExecCmdbCommand final: public GlCommand
{
public:
	CommandBufferPtr m_cmdb;

	ExecCmdbCommand(const CommandBufferPtr& cmdb)
		: m_cmdb(cmdb)
	{}

	Error operator()(GlState&)
	{
		return m_cmdb->getImplementation().executeAllCommands();
	}
};

void CommandBuffer::pushSecondLevelCommandBuffer(CommandBufferPtr cmdb)
{
	m_impl->pushBackNewCommand<ExecCmdbCommand>(cmdb);
}

//==============================================================================
Bool CommandBuffer::isEmpty() const
{
	return m_impl->isEmpty();
}

} // end namespace anki


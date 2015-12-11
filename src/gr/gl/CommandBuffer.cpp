// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/CommandBuffer.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/Pipeline.h>
#include <anki/gr/gl/PipelineImpl.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/gl/FramebufferImpl.h>
#include <anki/gr/ResourceGroup.h>
#include <anki/gr/gl/ResourceGroupImpl.h>
#include <anki/gr/OcclusionQuery.h>
#include <anki/gr/gl/OcclusionQueryImpl.h>
#include <anki/gr/Texture.h>
#include <anki/gr/gl/TextureImpl.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/gl/BufferImpl.h>
#include <anki/core/Trace.h>

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
	DynamicBufferInfo m_dynInfo;

	BindResourcesCommand(ResourceGroupPtr rc, U8 slot,
		const DynamicBufferInfo* dynInfo)
		: m_rc(rc)
		, m_slot(slot)
	{
		if(dynInfo)
		{
			m_dynInfo = *dynInfo;
		}
	}

	Error operator()(GlState& state)
	{
		m_rc->getImplementation().bind(m_slot, m_dynInfo, state);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::bindResourceGroup(ResourceGroupPtr rc, U slot,
	const DynamicBufferInfo* dynInfo)
{
	ANKI_ASSERT(rc.isCreated());
	m_impl->pushBackNewCommand<BindResourcesCommand>(rc, slot, dynInfo);
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

			ANKI_TRACE_INC_COUNTER(GR_DRAWCALLS, 1);
			ANKI_TRACE_INC_COUNTER(GR_VERTICES,
				m_info.m_instanceCount * m_info.m_count);
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

			ANKI_TRACE_INC_COUNTER(GR_DRAWCALLS, 1);
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
		: m_size({{x, y, z}})
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
	DynamicBufferToken m_token;

	TexUploadCommand(const TexturePtr& handle, U32 mipmap, U32 slice,
		const DynamicBufferToken& token)
		: m_handle(handle)
		, m_mipmap(mipmap)
		, m_slice(slice)
		, m_token(token)
	{}

	Error operator()(GlState& state)
	{
		U8* data =
			state.m_dynamicBuffers[BufferUsage::TRANSFER].m_address
			+ m_token.m_offset;

		m_handle->getImplementation().write(
			m_mipmap, m_slice, data, m_token.m_range);

		return ErrorCode::NONE;
	}
};

void CommandBuffer::textureUpload(TexturePtr tex, U32 mipmap, U32 slice,
	const DynamicBufferToken& token)
{
	ANKI_ASSERT(token.m_range > 0);

	m_impl->pushBackNewCommand<TexUploadCommand>(tex, mipmap, slice, token);
}

//==============================================================================
class BuffWriteCommand final: public GlCommand
{
public:
	BufferPtr m_handle;
	PtrSize m_offset;
	DynamicBufferToken m_token;

	BuffWriteCommand(const BufferPtr& handle, PtrSize offset,
		const DynamicBufferToken& token)
		: m_handle(handle)
		, m_offset(offset)
		, m_token(token)
	{}

	Error operator()(GlState& state)
	{
		U8* data =
			state.m_dynamicBuffers[BufferUsage::TRANSFER].m_address
			+ m_token.m_offset;

		m_handle->getImplementation().write(data, m_offset, m_token.m_range);

		return ErrorCode::NONE;
	}
};

void CommandBuffer::writeBuffer(BufferPtr buff, PtrSize offset,
	const DynamicBufferToken& token)
{
	m_impl->pushBackNewCommand<BuffWriteCommand>(buff, offset, token);
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
		m_tex->getImplementation().generateMipmaps(0);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::generateMipmaps(TexturePtr tex)
{
	m_impl->pushBackNewCommand<GenMipsCommand>(tex);
}

//==============================================================================
class GenMipsCommand1 final: public GlCommand
{
public:
	TexturePtr m_tex;
	U32 m_surface;

	GenMipsCommand1(const TexturePtr& tex, U surface)
		: m_tex(tex)
		, m_surface(surface)
	{}

	Error operator()(GlState&)
	{
		m_tex->getImplementation().generateMipmaps(m_surface);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::generateMipmaps(TexturePtr tex, U surface)
{
	m_impl->pushBackNewCommand<GenMipsCommand1>(tex, surface);
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

//==============================================================================
class CopyTexCommand final: public GlCommand
{
public:
	TexturePtr m_src;
	U16 m_srcSlice;
	U16 m_srcLevel;
	TexturePtr m_dest;
	U16 m_destSlice;
	U16 m_destLevel;

	CopyTexCommand(TexturePtr src, U srcSlice, U srcLevel, TexturePtr dest,
		U destSlice, U destLevel)
		: m_src(src)
		, m_srcSlice(srcSlice)
		, m_srcLevel(srcLevel)
		, m_dest(dest)
		, m_destSlice(destSlice)
		, m_destLevel(destLevel)
	{}

	Error operator()(GlState&)
	{
		TextureImpl::copy(m_src->getImplementation(), m_srcSlice, m_srcLevel,
			m_dest->getImplementation(), m_destSlice, m_destLevel);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::copyTextureToTexture(TexturePtr src, U srcSlice, U srcLevel,
	TexturePtr dest, U destSlice, U destLevel)
{
	m_impl->pushBackNewCommand<CopyTexCommand>(src, srcSlice, srcLevel, dest,
		destSlice, destLevel);
}

} // end namespace anki


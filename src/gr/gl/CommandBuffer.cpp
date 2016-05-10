// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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

namespace anki
{

//==============================================================================
CommandBuffer::CommandBuffer(GrManager* manager, U64 hash)
	: GrObject(manager, CLASS_TYPE, hash)
{
}

//==============================================================================
CommandBuffer::~CommandBuffer()
{
}

//==============================================================================
void CommandBuffer::init(CommandBufferInitInfo& inf)
{
	m_impl.reset(getAllocator().newInstance<CommandBufferImpl>(&getManager()));
	m_impl->init(inf.m_hints);

#if ANKI_ASSERTS_ENABLED
	if(inf.m_secondLevel)
	{
		ANKI_ASSERT(inf.m_framebuffer.isCreated());
		m_impl->m_dbg.m_insideRenderPass = true;
		m_impl->m_dbg.m_secondLevel = true;
	}
#endif
}

//==============================================================================
void CommandBuffer::flush()
{
#if ANKI_ASSERTS_ENABLED
	if(!m_impl->m_dbg.m_secondLevel)
	{
		ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
	}
#endif
	getManager().getImplementation().getRenderingThread().flushCommandBuffer(
		CommandBufferPtr(this));
}

//==============================================================================
void CommandBuffer::finish()
{
#if ANKI_ASSERTS_ENABLED
	if(!m_impl->m_dbg.m_secondLevel)
	{
		ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
	}
#endif
	getManager().getImplementation().getRenderingThread().finishCommandBuffer(
		CommandBufferPtr(this));
}

//==============================================================================
class ViewportCommand final : public GlCommand
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
#if ANKI_ASSERTS_ENABLED
	m_impl->m_dbg.m_viewport = true;
#endif
	m_impl->pushBackNewCommand<ViewportCommand>(minx, miny, maxx, maxy);
}

//==============================================================================
class SetPolygonOffsetCommand final : public GlCommand
{
public:
	F32 m_offset;
	F32 m_units;

	SetPolygonOffsetCommand(F32 offset, F32 units)
		: m_offset(offset)
		, m_units(units)
	{
	}

	Error operator()(GlState& state)
	{
		if(m_offset == 0.0 && m_units == 0.0)
		{
			glDisable(GL_POLYGON_OFFSET_FILL);
		}
		else
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(m_offset, m_units);
		}

		return ErrorCode::NONE;
	}
};

void CommandBuffer::setPolygonOffset(F32 offset, F32 units)
{
#if ANKI_ASSERTS_ENABLED
	m_impl->m_dbg.m_polygonOffset = true;
#endif
	m_impl->pushBackNewCommand<SetPolygonOffsetCommand>(offset, units);
}

//==============================================================================
class BindPipelineCommand final : public GlCommand
{
public:
	PipelinePtr m_ppline;

	BindPipelineCommand(PipelinePtr& ppline)
		: m_ppline(ppline)
	{
	}

	Error operator()(GlState& state)
	{
		if(state.m_lastPplineBoundUuid != m_ppline->getUuid())
		{
			ANKI_TRACE_START_EVENT(GL_BIND_PPLINE);

			PipelineImpl& impl = m_ppline->getImplementation();
			impl.bind(state);
			state.m_lastPplineBoundUuid = m_ppline->getUuid();
			ANKI_TRACE_INC_COUNTER(GR_PIPELINE_BINDS_HAPPENED, 1);

			ANKI_TRACE_STOP_EVENT(GL_BIND_PPLINE);
		}
		else
		{
			ANKI_TRACE_INC_COUNTER(GR_PIPELINE_BINDS_SKIPPED, 1);
		}

		return ErrorCode::NONE;
	}
};

void CommandBuffer::bindPipeline(PipelinePtr ppline)
{
	m_impl->pushBackNewCommand<BindPipelineCommand>(ppline);
}

//==============================================================================
class BindFramebufferCommand final : public GlCommand
{
public:
	FramebufferPtr m_fb;

	BindFramebufferCommand(FramebufferPtr fb)
		: m_fb(fb)
	{
	}

	Error operator()(GlState& state)
	{
		m_fb->getImplementation().bind(state);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::beginRenderPass(FramebufferPtr fb)
{
	ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
#if ANKI_ASSERTS_ENABLED
	m_impl->m_dbg.m_insideRenderPass = true;
#endif
	m_impl->pushBackNewCommand<BindFramebufferCommand>(fb);
}

//==============================================================================
void CommandBuffer::endRenderPass()
{
	// Nothing for GL
	ANKI_ASSERT(m_impl->m_dbg.m_insideRenderPass);
#if ANKI_ASSERTS_ENABLED
	m_impl->m_dbg.m_insideRenderPass = false;
#endif
}

//==============================================================================
class BindResourcesCommand final : public GlCommand
{
public:
	ResourceGroupPtr m_rc;
	U8 m_slot;
	DynamicBufferInfo m_dynInfo;

	BindResourcesCommand(
		ResourceGroupPtr rc, U8 slot, const DynamicBufferInfo* dynInfo)
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
		ANKI_TRACE_START_EVENT(GL_BIND_RESOURCES);
		m_rc->getImplementation().bind(m_slot, m_dynInfo, state);
		ANKI_TRACE_STOP_EVENT(GL_BIND_RESOURCES);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::bindResourceGroup(
	ResourceGroupPtr rc, U slot, const DynamicBufferInfo* dynInfo)
{
	ANKI_ASSERT(rc.isCreated());
	m_impl->pushBackNewCommand<BindResourcesCommand>(rc, slot, dynInfo);
}

//==============================================================================
class DrawElementsCondCommand : public GlCommand
{
public:
	DrawElementsIndirectInfo m_info;
	OcclusionQueryPtr m_query;

	DrawElementsCondCommand(const DrawElementsIndirectInfo& info,
		OcclusionQueryPtr query = OcclusionQueryPtr())
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
			ANKI_TRACE_INC_COUNTER(
				GR_VERTICES, m_info.m_instanceCount * m_info.m_count);
		}

		return ErrorCode::NONE;
	}
};

void CommandBuffer::drawElements(U32 count,
	U32 instanceCount,
	U32 firstIndex,
	U32 baseVertex,
	U32 baseInstance)
{
	ANKI_ASSERT(m_impl->m_dbg.m_insideRenderPass);
	DrawElementsIndirectInfo info(
		count, instanceCount, firstIndex, baseVertex, baseInstance);

	m_impl->checkDrawcall();
	m_impl->pushBackNewCommand<DrawElementsCondCommand>(info);
}

//==============================================================================
class DrawArraysCondCommand final : public GlCommand
{
public:
	DrawArraysIndirectInfo m_info;
	OcclusionQueryPtr m_query;

	DrawArraysCondCommand(const DrawArraysIndirectInfo& info,
		OcclusionQueryPtr query = OcclusionQueryPtr())
		: m_info(info)
		, m_query(query)
	{
	}

	Error operator()(GlState& state)
	{
		if(!m_query.isCreated() || !m_query->getImplementation().skipDrawcall())
		{
			state.flushVertexState();

			glDrawArraysInstancedBaseInstance(state.m_topology,
				m_info.m_first,
				m_info.m_count,
				m_info.m_instanceCount,
				m_info.m_baseInstance);

			ANKI_TRACE_INC_COUNTER(GR_DRAWCALLS, 1);
		}

		return ErrorCode::NONE;
	}
};

void CommandBuffer::drawArrays(
	U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	ANKI_ASSERT(m_impl->m_dbg.m_insideRenderPass);
	DrawArraysIndirectInfo info(count, instanceCount, first, baseInstance);

	m_impl->checkDrawcall();
	m_impl->pushBackNewCommand<DrawArraysCondCommand>(info);
}

//==============================================================================
void CommandBuffer::drawElementsConditional(OcclusionQueryPtr query,
	U32 count,
	U32 instanceCount,
	U32 firstIndex,
	U32 baseVertex,
	U32 baseInstance)
{
	ANKI_ASSERT(m_impl->m_dbg.m_insideRenderPass);
	DrawElementsIndirectInfo info(
		count, instanceCount, firstIndex, baseVertex, baseInstance);

	m_impl->checkDrawcall();
	m_impl->pushBackNewCommand<DrawElementsCondCommand>(info, query);
}

//==============================================================================
void CommandBuffer::drawArraysConditional(OcclusionQueryPtr query,
	U32 count,
	U32 instanceCount,
	U32 first,
	U32 baseInstance)
{
	ANKI_ASSERT(m_impl->m_dbg.m_insideRenderPass);
	DrawArraysIndirectInfo info(count, instanceCount, first, baseInstance);

	m_impl->checkDrawcall();
	m_impl->pushBackNewCommand<DrawArraysCondCommand>(info, query);
}

//==============================================================================
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

void CommandBuffer::dispatchCompute(
	U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
	m_impl->pushBackNewCommand<DispatchCommand>(
		groupCountX, groupCountY, groupCountZ);
}

//==============================================================================
class OqBeginCommand final : public GlCommand
{
public:
	OcclusionQueryPtr m_handle;

	OqBeginCommand(const OcclusionQueryPtr& handle)
		: m_handle(handle)
	{
	}

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
class OqEndCommand final : public GlCommand
{
public:
	OcclusionQueryPtr m_handle;

	OqEndCommand(const OcclusionQueryPtr& handle)
		: m_handle(handle)
	{
	}

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
class TexUploadCommand final : public GlCommand
{
public:
	TexturePtr m_handle;
	TextureSurfaceInfo m_surf;
	DynamicBufferToken m_token;

	TexUploadCommand(const TexturePtr& handle,
		TextureSurfaceInfo surf,
		const DynamicBufferToken& token)
		: m_handle(handle)
		, m_surf(surf)
		, m_token(token)
	{
	}

	Error operator()(GlState& state)
	{
		U8* data = static_cast<U8*>(state.m_dynamicMemoryManager.getBaseAddress(
					   BufferUsage::TRANSFER))
			+ m_token.m_offset;

		m_handle->getImplementation().write(m_surf, data, m_token.m_range);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::textureUpload(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	const DynamicBufferToken& token)
{
	ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
	ANKI_ASSERT(token.m_range > 0);

	m_impl->pushBackNewCommand<TexUploadCommand>(tex, surf, token);
}

//==============================================================================
class BuffWriteCommand final : public GlCommand
{
public:
	BufferPtr m_handle;
	PtrSize m_offset;
	DynamicBufferToken m_token;

	BuffWriteCommand(const BufferPtr& handle,
		PtrSize offset,
		const DynamicBufferToken& token)
		: m_handle(handle)
		, m_offset(offset)
		, m_token(token)
	{
	}

	Error operator()(GlState& state)
	{
		U8* data = static_cast<U8*>(state.m_dynamicMemoryManager.getBaseAddress(
					   BufferUsage::TRANSFER))
			+ m_token.m_offset;

		m_handle->getImplementation().write(data, m_offset, m_token.m_range);

		return ErrorCode::NONE;
	}
};

void CommandBuffer::writeBuffer(
	BufferPtr buff, PtrSize offset, const DynamicBufferToken& token)
{
	ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
	m_impl->pushBackNewCommand<BuffWriteCommand>(buff, offset, token);
}

//==============================================================================
class GenMipsCommand final : public GlCommand
{
public:
	TexturePtr m_tex;
	U32 m_depth;
	U8 m_face;

	GenMipsCommand(const TexturePtr& tex, U depth, U face)
		: m_tex(tex)
		, m_depth(depth)
		, m_face(face)
	{
	}

	Error operator()(GlState&)
	{
		m_tex->getImplementation().generateMipmaps(m_depth, m_face);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::generateMipmaps(TexturePtr tex, U depth, U face)
{
	ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
	m_impl->pushBackNewCommand<GenMipsCommand>(tex, depth, face);
}

//==============================================================================
CommandBufferInitHints CommandBuffer::computeInitHints() const
{
	return m_impl->computeInitHints();
}

//==============================================================================
class ExecCmdbCommand final : public GlCommand
{
public:
	CommandBufferPtr m_cmdb;

	ExecCmdbCommand(const CommandBufferPtr& cmdb)
		: m_cmdb(cmdb)
	{
	}

	Error operator()(GlState&)
	{
		ANKI_TRACE_START_EVENT(GL_2ND_LEVEL_CMD_BUFFER);
		Error err = m_cmdb->getImplementation().executeAllCommands();
		ANKI_TRACE_STOP_EVENT(GL_2ND_LEVEL_CMD_BUFFER);
		return err;
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
class CopyTexCommand final : public GlCommand
{
public:
	TexturePtr m_src;
	TextureSurfaceInfo m_srcSurf;
	TexturePtr m_dest;
	TextureSurfaceInfo m_destSurf;

	CopyTexCommand(TexturePtr src,
		const TextureSurfaceInfo& srcSurf,
		TexturePtr dest,
		const TextureSurfaceInfo& destSurf)
		: m_src(src)
		, m_srcSurf(srcSurf)
		, m_dest(dest)
		, m_destSurf(destSurf)
	{
	}

	Error operator()(GlState&)
	{
		TextureImpl::copy(m_src->getImplementation(),
			m_srcSurf,
			m_dest->getImplementation(),
			m_destSurf);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::copyTextureToTexture(TexturePtr src,
	const TextureSurfaceInfo& srcSurf,
	TexturePtr dest,
	const TextureSurfaceInfo& destSurf)
{
	ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
	m_impl->pushBackNewCommand<CopyTexCommand>(src, srcSurf, dest, destSurf);
}

//==============================================================================
class SetBufferMemBarrierCommand final : public GlCommand
{
public:
	GLenum m_barrier;

	SetBufferMemBarrierCommand(GLenum barrier)
		: m_barrier(barrier)
	{
	}

	Error operator()(GlState&)
	{
		glMemoryBarrier(m_barrier);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::setBufferMemoryBarrier(
	BufferPtr buff, ResourceAccessBit src, ResourceAccessBit dst)
{
	const ResourceAccessBit c = dst;
	GLenum d = GL_NONE;

	if((c & ResourceAccessBit::INDIRECT_OR_INDEX_OR_VERTEX_READ)
		!= ResourceAccessBit::NONE)
	{
		d |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT
			| GL_COMMAND_BARRIER_BIT;
	}

	if((c & ResourceAccessBit::UNIFORM_READ) != ResourceAccessBit::NONE)
	{
		d |= GL_UNIFORM_BARRIER_BIT;
	}

	if((c & ResourceAccessBit::ATTACHMENT_READ) != ResourceAccessBit::NONE
		|| (c & ResourceAccessBit::ATTACHMENT_WRITE) != ResourceAccessBit::NONE)
	{
		d |= GL_FRAMEBUFFER_BARRIER_BIT;
	}

	if((c & ResourceAccessBit::SHADER_READ) != ResourceAccessBit::NONE
		|| (c & ResourceAccessBit::SHADER_WRITE) != ResourceAccessBit::NONE)
	{
		d |= GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT
			| GL_TEXTURE_FETCH_BARRIER_BIT;
	}

	if((c & ResourceAccessBit::CLIENT_READ) != ResourceAccessBit::NONE
		|| (c & ResourceAccessBit::CLIENT_WRITE) != ResourceAccessBit::NONE)
	{
		d |= GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT;
	}

	if((c & ResourceAccessBit::TRANSFER_READ) != ResourceAccessBit::NONE
		|| (c & ResourceAccessBit::TRANSFER_WRITE) != ResourceAccessBit::NONE)
	{
		d |= GL_BUFFER_UPDATE_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT;
	}

	ANKI_ASSERT(d != GL_NONE);
	m_impl->pushBackNewCommand<SetBufferMemBarrierCommand>(d);
}

//==============================================================================
class ClearTextCommand final : public GlCommand
{
public:
	TexturePtr m_tex;
	ClearValue m_val;
	TextureSurfaceInfo m_surf;

	ClearTextCommand(
		TexturePtr tex, const TextureSurfaceInfo& surf, const ClearValue& val)
		: m_tex(tex)
		, m_val(val)
		, m_surf(surf)
	{
	}

	Error operator()(GlState&)
	{
		m_tex->getImplementation().clear(m_surf, m_val);
		return ErrorCode::NONE;
	}
};

void CommandBuffer::clearTexture(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	const ClearValue& clearValue)
{
	m_impl->pushBackNewCommand<ClearTextCommand>(tex, surf, clearValue);
}

} // end namespace anki

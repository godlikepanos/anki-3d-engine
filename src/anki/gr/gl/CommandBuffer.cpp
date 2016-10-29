// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/CommandBuffer.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/gl/GlState.h>
#include <anki/gr/gl/TransientMemoryManager.h>

#include <anki/gr/Pipeline.h>
#include <anki/gr/gl/PipelineImpl.h>
#include <anki/gr/ResourceGroup.h>
#include <anki/gr/gl/ResourceGroupImpl.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/gl/FramebufferImpl.h>
#include <anki/gr/OcclusionQuery.h>
#include <anki/gr/gl/OcclusionQueryImpl.h>
#include <anki/gr/Texture.h>
#include <anki/gr/gl/TextureImpl.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/gl/BufferImpl.h>

#include <anki/core/Trace.h>

namespace anki
{

CommandBuffer::CommandBuffer(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

CommandBuffer::~CommandBuffer()
{
}

void CommandBuffer::init(CommandBufferInitInfo& inf)
{
	m_impl.reset(getAllocator().newInstance<CommandBufferImpl>(&getManager()));
	m_impl->init(inf);

#if ANKI_ASSERTS_ENABLED
	if((inf.m_flags & CommandBufferFlag::SECOND_LEVEL) == CommandBufferFlag::SECOND_LEVEL)
	{
		ANKI_ASSERT(inf.m_framebuffer.isCreated());
		m_impl->m_dbg.m_insideRenderPass = true;
		m_impl->m_dbg.m_secondLevel = true;
	}
#endif
}

void CommandBuffer::flush()
{
#if ANKI_ASSERTS_ENABLED
	if(!m_impl->m_dbg.m_secondLevel)
	{
		ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
	}
#endif

	if(!m_impl->isSecondLevel())
	{
		getManager().getImplementation().getRenderingThread().flushCommandBuffer(CommandBufferPtr(this));
	}
}

void CommandBuffer::finish()
{
#if ANKI_ASSERTS_ENABLED
	if(!m_impl->m_dbg.m_secondLevel)
	{
		ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
	}
#endif
	getManager().getImplementation().getRenderingThread().finishCommandBuffer(CommandBufferPtr(this));
}

void CommandBuffer::setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy)
{
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
			if(state.m_viewport[0] != m_value[0] || state.m_viewport[1] != m_value[1]
				|| state.m_viewport[2] != m_value[2]
				|| state.m_viewport[3] != m_value[3])
			{
				glViewport(m_value[0], m_value[1], m_value[2], m_value[3]);

				state.m_viewport = m_value;
			}

			return ErrorCode::NONE;
		}
	};

#if ANKI_ASSERTS_ENABLED
	m_impl->m_dbg.m_viewport = true;
#endif
	m_impl->pushBackNewCommand<ViewportCommand>(minx, miny, maxx, maxy);
}

void CommandBuffer::setPolygonOffset(F32 factor, F32 units)
{
	class SetPolygonOffsetCommand final : public GlCommand
	{
	public:
		F32 m_factor;
		F32 m_units;

		SetPolygonOffsetCommand(F32 factor, F32 units)
			: m_factor(factor)
			, m_units(units)
		{
		}

		Error operator()(GlState& state)
		{
			if(m_factor == 0.0 && m_units == 0.0)
			{
				glDisable(GL_POLYGON_OFFSET_FILL);
			}
			else
			{
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(m_factor, m_units);
			}

			return ErrorCode::NONE;
		}
	};

#if ANKI_ASSERTS_ENABLED
	m_impl->m_dbg.m_polygonOffset = true;
#endif
	m_impl->pushBackNewCommand<SetPolygonOffsetCommand>(factor, units);
}

void CommandBuffer::bindPipeline(PipelinePtr ppline)
{
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

				PipelineImpl& impl = *m_ppline->m_impl;
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

	m_impl->pushBackNewCommand<BindPipelineCommand>(ppline);
}

void CommandBuffer::beginRenderPass(FramebufferPtr fb)
{
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
			m_fb->m_impl->bind(state);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
#if ANKI_ASSERTS_ENABLED
	m_impl->m_dbg.m_insideRenderPass = true;
#endif
	m_impl->pushBackNewCommand<BindFramebufferCommand>(fb);
}

void CommandBuffer::endRenderPass()
{
	// Nothing for GL
	ANKI_ASSERT(m_impl->m_dbg.m_insideRenderPass);
#if ANKI_ASSERTS_ENABLED
	m_impl->m_dbg.m_insideRenderPass = false;
#endif
}

void CommandBuffer::bindResourceGroup(ResourceGroupPtr rc, U slot, const TransientMemoryInfo* info)
{
	m_impl->bindResourceGroup(rc, slot, info);
}

void CommandBuffer::drawElements(U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	m_impl->drawElements(count, instanceCount, firstIndex, baseVertex, baseInstance);
}

void CommandBuffer::drawArrays(U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	m_impl->drawArrays(count, instanceCount, first, baseInstance);
}

void CommandBuffer::drawElementsIndirect(U32 drawCount, PtrSize offset, BufferPtr indirectBuff)
{
	m_impl->drawElementsIndirect(drawCount, offset, indirectBuff);
}

void CommandBuffer::drawArraysIndirect(U32 drawCount, PtrSize offset, BufferPtr indirectBuff)
{
	m_impl->drawArraysIndirect(drawCount, offset, indirectBuff);
}

void CommandBuffer::dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	m_impl->dispatchCompute(groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::resetOcclusionQuery(OcclusionQueryPtr query)
{
	// Nothing for GL
}

void CommandBuffer::beginOcclusionQuery(OcclusionQueryPtr query)
{
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
			m_handle->m_impl->begin();
			return ErrorCode::NONE;
		}
	};

	m_impl->pushBackNewCommand<OqBeginCommand>(query);
}

void CommandBuffer::endOcclusionQuery(OcclusionQueryPtr query)
{
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
			m_handle->m_impl->end();
			return ErrorCode::NONE;
		}
	};

	m_impl->pushBackNewCommand<OqEndCommand>(query);
}

void CommandBuffer::uploadTextureSurface(
	TexturePtr tex, const TextureSurfaceInfo& surf, const TransientMemoryToken& token)
{
	class TexSurfUploadCommand final : public GlCommand
	{
	public:
		TexturePtr m_handle;
		TextureSurfaceInfo m_surf;
		TransientMemoryToken m_token;

		TexSurfUploadCommand(const TexturePtr& handle, TextureSurfaceInfo surf, const TransientMemoryToken& token)
			: m_handle(handle)
			, m_surf(surf)
			, m_token(token)
		{
		}

		Error operator()(GlState& state)
		{
			GLuint pbo = state.m_manager->getImplementation().getTransientMemoryManager().getGlName(m_token);
			m_handle->m_impl->writeSurface(m_surf, pbo, m_token.m_offset, m_token.m_range);

			if(m_token.m_lifetime == TransientMemoryTokenLifetime::PERSISTENT)
			{
				state.m_manager->getImplementation().getTransientMemoryManager().free(m_token);
			}

			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(tex);
	ANKI_ASSERT(token.m_range > 0);
	ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);

	m_impl->pushBackNewCommand<TexSurfUploadCommand>(tex, surf, token);
}

void CommandBuffer::uploadTextureVolume(TexturePtr tex, const TextureVolumeInfo& vol, const TransientMemoryToken& token)
{
	class TexVolUploadCommand final : public GlCommand
	{
	public:
		TexturePtr m_handle;
		TextureVolumeInfo m_vol;
		TransientMemoryToken m_token;

		TexVolUploadCommand(const TexturePtr& handle, TextureVolumeInfo vol, const TransientMemoryToken& token)
			: m_handle(handle)
			, m_vol(vol)
			, m_token(token)
		{
		}

		Error operator()(GlState& state)
		{
			GLuint pbo = state.m_manager->getImplementation().getTransientMemoryManager().getGlName(m_token);
			m_handle->m_impl->writeVolume(m_vol, pbo, m_token.m_offset, m_token.m_range);

			if(m_token.m_lifetime == TransientMemoryTokenLifetime::PERSISTENT)
			{
				state.m_manager->getImplementation().getTransientMemoryManager().free(m_token);
			}

			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(tex);
	ANKI_ASSERT(token.m_range > 0);
	ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);

	m_impl->pushBackNewCommand<TexVolUploadCommand>(tex, vol, token);
}

void CommandBuffer::uploadBuffer(BufferPtr buff, PtrSize offset, const TransientMemoryToken& token)
{
	class BuffWriteCommand final : public GlCommand
	{
	public:
		BufferPtr m_handle;
		PtrSize m_offset;
		TransientMemoryToken m_token;

		BuffWriteCommand(const BufferPtr& handle, PtrSize offset, const TransientMemoryToken& token)
			: m_handle(handle)
			, m_offset(offset)
			, m_token(token)
		{
		}

		Error operator()(GlState& state)
		{
			GLuint pbo = state.m_manager->getImplementation().getTransientMemoryManager().getGlName(m_token);
			m_handle->m_impl->write(pbo, m_token.m_offset, m_offset, m_token.m_range);

			if(m_token.m_lifetime == TransientMemoryTokenLifetime::PERSISTENT)
			{
				state.m_manager->getImplementation().getTransientMemoryManager().free(m_token);
			}

			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(token.m_range > 0);
	ANKI_ASSERT(buff);
	ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);

	m_impl->pushBackNewCommand<BuffWriteCommand>(buff, offset, token);
}

void CommandBuffer::generateMipmaps2d(TexturePtr tex, U face, U layer)
{
	class GenMipsCommand final : public GlCommand
	{
	public:
		TexturePtr m_tex;
		U8 m_face;
		U32 m_layer;

		GenMipsCommand(const TexturePtr& tex, U face, U layer)
			: m_tex(tex)
			, m_face(face)
			, m_layer(layer)
		{
		}

		Error operator()(GlState&)
		{
			m_tex->m_impl->generateMipmaps2d(m_face, m_layer);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
	m_impl->pushBackNewCommand<GenMipsCommand>(tex, face, layer);
}

void CommandBuffer::generateMipmaps3d(TexturePtr tex)
{
	ANKI_ASSERT(!!"TODO");
}

CommandBufferInitHints CommandBuffer::computeInitHints() const
{
	return m_impl->computeInitHints();
}

void CommandBuffer::pushSecondLevelCommandBuffer(CommandBufferPtr cmdb)
{
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
			Error err = m_cmdb->m_impl->executeAllCommands();
			ANKI_TRACE_STOP_EVENT(GL_2ND_LEVEL_CMD_BUFFER);
			return err;
		}
	};

	m_impl->pushBackNewCommand<ExecCmdbCommand>(cmdb);
}

Bool CommandBuffer::isEmpty() const
{
	return m_impl->isEmpty();
}

void CommandBuffer::copyTextureSurfaceToTextureSurface(
	TexturePtr src, const TextureSurfaceInfo& srcSurf, TexturePtr dest, const TextureSurfaceInfo& destSurf)
{
	class CopyTexCommand final : public GlCommand
	{
	public:
		TexturePtr m_src;
		TextureSurfaceInfo m_srcSurf;
		TexturePtr m_dest;
		TextureSurfaceInfo m_destSurf;

		CopyTexCommand(
			TexturePtr src, const TextureSurfaceInfo& srcSurf, TexturePtr dest, const TextureSurfaceInfo& destSurf)
			: m_src(src)
			, m_srcSurf(srcSurf)
			, m_dest(dest)
			, m_destSurf(destSurf)
		{
		}

		Error operator()(GlState&)
		{
			TextureImpl::copy(*m_src->m_impl, m_srcSurf, *m_dest->m_impl, m_destSurf);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(!m_impl->m_dbg.m_insideRenderPass);
	m_impl->pushBackNewCommand<CopyTexCommand>(src, srcSurf, dest, destSurf);
}

void CommandBuffer::setBufferBarrier(
	BufferPtr buff, BufferUsageBit prevUsage, BufferUsageBit nextUsage, PtrSize offset, PtrSize size)
{
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

	GLenum d = GL_NONE;
	BufferUsageBit all = prevUsage | nextUsage;

	if(!!(all & BufferUsageBit::UNIFORM_ALL))
	{
		d |= GL_UNIFORM_BARRIER_BIT;
	}

	if(!!(all & BufferUsageBit::STORAGE_ALL))
	{
		d |= GL_SHADER_STORAGE_BARRIER_BIT;
	}

	if(!!(all & BufferUsageBit::INDEX))
	{
		d |= GL_ELEMENT_ARRAY_BARRIER_BIT;
	}

	if(!!(all & BufferUsageBit::VERTEX))
	{
		d |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
	}

	if(!!(all & BufferUsageBit::INDIRECT))
	{
		d |= GL_COMMAND_BARRIER_BIT;
	}

	if(!!(all
		   & (BufferUsageBit::FILL | BufferUsageBit::BUFFER_UPLOAD_SOURCE | BufferUsageBit::BUFFER_UPLOAD_DESTINATION)))
	{
		d |= GL_BUFFER_UPDATE_BARRIER_BIT;
	}

	if(!!(all & BufferUsageBit::QUERY_RESULT))
	{
		d |= GL_QUERY_BUFFER_BARRIER_BIT;
	}

	ANKI_ASSERT(d);
	m_impl->pushBackNewCommand<SetBufferMemBarrierCommand>(d);
}

void CommandBuffer::setTextureSurfaceBarrier(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureSurfaceInfo& surf)
{
	// Do nothing
}

void CommandBuffer::setTextureVolumeBarrier(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureVolumeInfo& vol)
{
	// Do nothing
}

void CommandBuffer::clearTextureSurface(
	TexturePtr tex, const TextureSurfaceInfo& surf, const ClearValue& clearValue, DepthStencilAspectMask aspect)
{
	class ClearTextCommand final : public GlCommand
	{
	public:
		TexturePtr m_tex;
		ClearValue m_val;
		TextureSurfaceInfo m_surf;
		DepthStencilAspectMask m_aspect;

		ClearTextCommand(
			TexturePtr tex, const TextureSurfaceInfo& surf, const ClearValue& val, DepthStencilAspectMask aspect)
			: m_tex(tex)
			, m_val(val)
			, m_surf(surf)
			, m_aspect(aspect)
		{
		}

		Error operator()(GlState&)
		{
			m_tex->m_impl->clear(m_surf, m_val, m_aspect);
			return ErrorCode::NONE;
		}
	};

	m_impl->pushBackNewCommand<ClearTextCommand>(tex, surf, clearValue, aspect);
}

void CommandBuffer::fillBuffer(BufferPtr buff, PtrSize offset, PtrSize size, U32 value)
{
	class FillBufferCommand final : public GlCommand
	{
	public:
		BufferPtr m_buff;
		PtrSize m_offset;
		PtrSize m_size;
		U32 m_value;

		FillBufferCommand(BufferPtr buff, PtrSize offset, PtrSize size, U32 value)
			: m_buff(buff)
			, m_offset(offset)
			, m_size(size)
			, m_value(value)
		{
		}

		Error operator()(GlState&)
		{
			m_buff->m_impl->fill(m_offset, m_size, m_value);
			return ErrorCode::NONE;
		}
	};

	m_impl->pushBackNewCommand<FillBufferCommand>(buff, offset, size, value);
}

void CommandBuffer::writeOcclusionQueryResultToBuffer(OcclusionQueryPtr query, PtrSize offset, BufferPtr buff)
{
	class WriteOcclResultToBuff final : public GlCommand
	{
	public:
		OcclusionQueryPtr m_query;
		PtrSize m_offset;
		BufferPtr m_buff;

		WriteOcclResultToBuff(OcclusionQueryPtr query, PtrSize offset, BufferPtr buff)
			: m_query(query)
			, m_offset(offset)
			, m_buff(buff)
		{
			ANKI_ASSERT((m_offset % 4) == 0);
		}

		Error operator()(GlState&)
		{
			const BufferImpl& buff = *m_buff->m_impl;
			ANKI_ASSERT(m_offset + 4 <= buff.m_size);

			glBindBuffer(GL_QUERY_BUFFER, buff.getGlName());
			glGetQueryObjectuiv(m_query->m_impl->getGlName(), GL_QUERY_RESULT, numberToPtr<GLuint*>(m_offset));
			glBindBuffer(GL_QUERY_BUFFER, 0);

			return ErrorCode::NONE;
		}
	};

	m_impl->pushBackNewCommand<WriteOcclResultToBuff>(query, offset, buff);
}

void CommandBuffer::setStencilCompareMask(FaceSelectionMask face, U32 mask)
{
	class SetStencilCompareMask final : public GlCommand
	{
	public:
		FaceSelectionMask m_face;
		U32 m_mask;

		SetStencilCompareMask(FaceSelectionMask face, U32 mask)
			: m_face(face)
			, m_mask(mask)
		{
		}

		Error operator()(GlState& state)
		{
			if(!!(m_face & FaceSelectionMask::FRONT) && state.m_stencilCompareMask[0] != m_mask)
			{
				state.m_stencilCompareMask[0] = m_mask;
				state.m_glStencilFuncSeparateDirtyMask |= 1 << 0;
			}

			if(!!(m_face & FaceSelectionMask::BACK) && state.m_stencilCompareMask[1] != m_mask)
			{
				state.m_stencilCompareMask[1] = m_mask;
				state.m_glStencilFuncSeparateDirtyMask |= 1 << 1;
			}

			return ErrorCode::NONE;
		}
	};

	m_impl->pushBackNewCommand<SetStencilCompareMask>(face, mask);
}

void CommandBuffer::setStencilWriteMask(FaceSelectionMask face, U32 mask)
{
	class SetStencilWriteMask final : public GlCommand
	{
	public:
		FaceSelectionMask m_face;
		U32 m_mask;

		SetStencilWriteMask(FaceSelectionMask face, U32 mask)
			: m_face(face)
			, m_mask(mask)
		{
		}

		Error operator()(GlState& state)
		{
			if(!!(m_face & FaceSelectionMask::FRONT) && state.m_stencilWriteMask[0] != m_mask)
			{
				glStencilMaskSeparate(GL_FRONT, m_mask);
				state.m_stencilWriteMask[0] = m_mask;
			}

			if(!!(m_face & FaceSelectionMask::BACK) && state.m_stencilWriteMask[1] != m_mask)
			{
				glStencilMaskSeparate(GL_BACK, m_mask);
				state.m_stencilWriteMask[1] = m_mask;
			}

			return ErrorCode::NONE;
		}
	};

	m_impl->pushBackNewCommand<SetStencilWriteMask>(face, mask);
}

void CommandBuffer::setStencilReference(FaceSelectionMask face, U32 ref)
{
	class SetStencilRefMask final : public GlCommand
	{
	public:
		FaceSelectionMask m_face;
		U32 m_ref;

		SetStencilRefMask(FaceSelectionMask face, U32 ref)
			: m_face(face)
			, m_ref(ref)
		{
		}

		Error operator()(GlState& state)
		{
			if(!!(m_face & FaceSelectionMask::FRONT) && state.m_stencilRef[0] != m_ref)
			{
				state.m_stencilRef[0] = m_ref;
				state.m_glStencilFuncSeparateDirtyMask |= 1 << 0;
			}

			if(!!(m_face & FaceSelectionMask::BACK) && state.m_stencilRef[1] != m_ref)
			{
				state.m_stencilRef[1] = m_ref;
				state.m_glStencilFuncSeparateDirtyMask |= 1 << 1;
			}

			return ErrorCode::NONE;
		}
	};

	m_impl->pushBackNewCommand<SetStencilRefMask>(face, ref);
}

} // end namespace anki

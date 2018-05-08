// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
	auto& pool = getManager().getAllocator().getMemoryPool();

	m_alloc = CommandBufferAllocator<GlCommand*>(
		pool.getAllocationCallback(), pool.getAllocationCallbackUserData(), init.m_hints.m_chunkSize, 1.0, 0, false);

	m_flags = init.m_flags;

#if ANKI_EXTRA_CHECKS
	m_state.m_secondLevel = !!(init.m_flags & CommandBufferFlag::SECOND_LEVEL);
#endif

	if(!!(init.m_flags & CommandBufferFlag::SECOND_LEVEL))
	{
		// TODO Need to hold a ref
		m_state.m_fb = static_cast<const FramebufferImpl*>(init.m_framebuffer.get());
	}
}

void CommandBufferImpl::destroy()
{
	ANKI_TRACE_SCOPED_EVENT(GL_CMD_BUFFER_DESTROY);

#if ANKI_EXTRA_CHECKS
	if(!m_executed && m_firstCommand)
	{
		ANKI_GL_LOGW("Chain contains commands but never executed. This should only happen on exceptions");
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

Error CommandBufferImpl::executeAllCommands()
{
	ANKI_ASSERT(m_firstCommand != nullptr && "Empty command buffer");
	ANKI_ASSERT(m_lastCommand != nullptr && "Empty command buffer");
#if ANKI_EXTRA_CHECKS
	m_executed = true;
#endif

	Error err = Error::NONE;
	GlState& state = static_cast<GrManagerImpl&>(getManager()).getState();

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

void CommandBufferImpl::flushDrawcall(CommandBuffer& cmdb)
{
	ANKI_ASSERT(!!(m_flags & CommandBufferFlag::GRAPHICS_WORK));

	//
	// Set default state
	//
	if(ANKI_UNLIKELY(m_state.m_mayContainUnsetState))
	{
		m_state.m_mayContainUnsetState = false;

		if(m_state.m_primitiveRestart == 2)
		{
			cmdb.setPrimitiveRestart(false);
		}

		if(m_state.m_fillMode == FillMode::COUNT)
		{
			cmdb.setFillMode(FillMode::SOLID);
		}

		if(m_state.m_cullMode == static_cast<FaceSelectionBit>(0))
		{
			cmdb.setCullMode(FaceSelectionBit::BACK);
		}

		if(m_state.m_polyOffsetFactor == -1.0)
		{
			cmdb.setPolygonOffset(0.0, 0.0);
		}

		for(U i = 0; i < 2; ++i)
		{
			FaceSelectionBit face = (i == 0) ? FaceSelectionBit::FRONT : FaceSelectionBit::BACK;

			if(m_state.m_stencilFail[i] == StencilOperation::COUNT)
			{
				cmdb.setStencilOperations(face, StencilOperation::KEEP, StencilOperation::KEEP, StencilOperation::KEEP);
			}

			if(m_state.m_stencilCompare[i] == CompareOperation::COUNT)
			{
				cmdb.setStencilCompareOperation(face, CompareOperation::ALWAYS);
			}

			if(m_state.m_stencilCompareMask[i] == StateTracker::DUMMY_STENCIL_MASK)
			{
				cmdb.setStencilCompareMask(face, MAX_U32);
			}

			if(m_state.m_stencilWriteMask[i] == StateTracker::DUMMY_STENCIL_MASK)
			{
				cmdb.setStencilWriteMask(face, MAX_U32);
			}

			if(m_state.m_stencilRef[i] == StateTracker::DUMMY_STENCIL_MASK)
			{
				cmdb.setStencilReference(face, 0);
			}
		}

		if(m_state.m_depthWrite == 2)
		{
			cmdb.setDepthWrite(true);
		}

		if(m_state.m_depthOp == CompareOperation::COUNT)
		{
			cmdb.setDepthCompareOperation(CompareOperation::LESS);
		}

		for(U i = 0; i < MAX_COLOR_ATTACHMENTS; ++i)
		{
			const auto& att = m_state.m_colorAtt[i];
			if(att.m_writeMask == StateTracker::INVALID_COLOR_MASK)
			{
				cmdb.setColorChannelWriteMask(i, ColorBit::ALL);
			}

			if(att.m_blendSrcFactorRgb == BlendFactor::COUNT)
			{
				cmdb.setBlendFactors(i, BlendFactor::ONE, BlendFactor::ZERO, BlendFactor::ONE, BlendFactor::ZERO);
			}

			if(att.m_blendOpRgb == BlendOperation::COUNT)
			{
				cmdb.setBlendOperation(i, BlendOperation::ADD, BlendOperation::ADD);
			}
		}

		if(!m_state.m_scissorSet)
		{
			cmdb.setScissor(0, 0, MAX_U32, MAX_U32);
		}
	}

	//
	// Fire commands to change some state
	//
	class StencilCmd final : public GlCommand
	{
	public:
		GLenum m_face;
		GLenum m_func;
		GLint m_ref;
		GLuint m_compareMask;

		StencilCmd(GLenum face, GLenum func, GLint ref, GLuint mask)
			: m_face(face)
			, m_func(func)
			, m_ref(ref)
			, m_compareMask(mask)
		{
		}

		Error operator()(GlState&)
		{
			glStencilFuncSeparate(m_face, m_func, m_ref, m_compareMask);
			return Error::NONE;
		}
	};

	for(U i = 0; i < 2; ++i)
	{
		if(m_state.m_glStencilFuncSeparateDirty[i])
		{
			pushBackNewCommand<StencilCmd>(GL_FRONT + i,
				convertCompareOperation(m_state.m_stencilCompare[i]),
				m_state.m_stencilRef[i],
				m_state.m_stencilCompareMask[i]);

			m_state.m_glStencilFuncSeparateDirty[i] = false;
		}
	}

	class DepthTestCmd final : public GlCommand
	{
	public:
		Bool8 m_enable;

		DepthTestCmd(Bool enable)
			: m_enable(enable)
		{
		}

		Error operator()(GlState&)
		{
			if(m_enable)
			{
				glEnable(GL_DEPTH_TEST);
			}
			else
			{
				glDisable(GL_DEPTH_TEST);
			}
			return Error::NONE;
		}
	};

	if(m_state.maybeEnableDepthTest())
	{
		pushBackNewCommand<DepthTestCmd>(m_state.m_depthTestEnabled);
	}

	class StencilTestCmd final : public GlCommand
	{
	public:
		Bool8 m_enable;

		StencilTestCmd(Bool enable)
			: m_enable(enable)
		{
		}

		Error operator()(GlState&)
		{
			if(m_enable)
			{
				glEnable(GL_STENCIL_TEST);
			}
			else
			{
				glDisable(GL_STENCIL_TEST);
			}
			return Error::NONE;
		}
	};

	if(m_state.maybeEnableStencilTest())
	{
		pushBackNewCommand<StencilTestCmd>(m_state.m_stencilTestEnabled);
	}

	class BlendCmd final : public GlCommand
	{
	public:
		U8 m_enableMask;
		U8 m_disableMask;

		BlendCmd(U8 enableMask, U8 disableMask)
			: m_enableMask(enableMask)
			, m_disableMask(disableMask)
		{
		}

		Error operator()(GlState&)
		{
			for(U i = 0; i < MAX_COLOR_ATTACHMENTS; ++i)
			{
				if(m_enableMask & (1 << i))
				{
					glEnablei(GL_BLEND, i);
				}
				else if(m_disableMask & (1 << i))
				{
					glDisablei(GL_BLEND, i);
				}
			}
			return Error::NONE;
		}
	};

	U8 blendEnableMask = 0;
	U8 blendDisableMask = 0;
	for(U i = 0; i < MAX_COLOR_ATTACHMENTS; ++i)
	{
		if(m_state.maybeEnableBlend(i))
		{
			if(m_state.m_colorAtt[i].m_enableBlend)
			{
				blendEnableMask |= 1 << i;
			}
			else
			{
				blendDisableMask |= 1 << i;
			}
		}
	}

	if(blendEnableMask || blendDisableMask)
	{
		pushBackNewCommand<BlendCmd>(blendEnableMask, blendDisableMask);
	}
}

} // end namespace anki

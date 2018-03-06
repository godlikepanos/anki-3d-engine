// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/Common.h>
#include <anki/gr/ShaderProgram.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/gl/FramebufferImpl.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/gl/BufferImpl.h>
#include <anki/gr/Texture.h>
#include <anki/gr/gl/TextureImpl.h>
#include <anki/gr/Sampler.h>
#include <anki/gr/gl/SamplerImpl.h>
#include <anki/gr/ShaderProgram.h>
#include <anki/gr/gl/ShaderProgramImpl.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/gl/FramebufferImpl.h>
#include <anki/gr/common/Misc.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Local state tracker. Used to avoid creating command buffer commands.
class StateTracker
{
public:
	/// If it's false then there might be unset state.
	Bool m_mayContainUnsetState = true;

#if ANKI_EXTRA_CHECKS
	Bool m_secondLevel = false;
#endif

	/// @name vert_state
	/// @{
	class VertexAttribute
	{
	public:
		U32 m_buffBinding = MAX_U32;
		Format m_fmt = Format::NONE;
		PtrSize m_relativeOffset = MAX_PTR_SIZE;
	};

	Array<VertexAttribute, MAX_VERTEX_ATTRIBUTES> m_attribs;

	Bool setVertexAttribute(U32 location, U32 buffBinding, Format fmt, PtrSize relativeOffset)
	{
		VertexAttribute& attrib = m_attribs[location];
		if(attrib.m_buffBinding != buffBinding || attrib.m_fmt != fmt || attrib.m_relativeOffset != relativeOffset)
		{
			attrib.m_buffBinding = buffBinding;
			attrib.m_fmt = fmt;
			attrib.m_relativeOffset = relativeOffset;
			return true;
		}

		return false;
	}

	class VertexBuffer
	{
	public:
		BufferImpl* m_buff = nullptr;
		PtrSize m_offset = 0;
		PtrSize m_stride = 0;
		VertexStepRate m_stepRate = VertexStepRate::COUNT;
	};

	Array<VertexBuffer, MAX_VERTEX_ATTRIBUTES> m_vertBuffs;

	Bool bindVertexBuffer(U32 binding, BufferPtr buff, PtrSize offset, PtrSize stride, VertexStepRate stepRate)
	{
		VertexBuffer& b = m_vertBuffs[binding];
		b.m_buff = static_cast<BufferImpl*>(buff.get());
		b.m_offset = offset;
		b.m_stride = stride;
		b.m_stepRate = stepRate;
		return true;
	}

	class Index
	{
	public:
		BufferImpl* m_buff = nullptr;
		PtrSize m_offset = MAX_PTR_SIZE;
		GLenum m_indexType = 0;
	} m_idx;

	Bool bindIndexBuffer(BufferPtr buff, PtrSize offset, IndexType type)
	{
		m_idx.m_buff = static_cast<BufferImpl*>(buff.get());
		m_idx.m_offset = offset;
		m_idx.m_indexType = convertIndexType(type);
		return true;
	}
	/// @}

	/// @name input_assembly
	/// @{
	Bool m_primitiveRestart = 2;

	Bool setPrimitiveRestart(Bool enable)
	{
		if(enable != m_primitiveRestart)
		{
			m_primitiveRestart = enable;
			return true;
		}
		return false;
	}
	/// @}

	/// @name viewport_state
	/// @{
	Array<U32, 4> m_viewport = {{MAX_U32, MAX_U32, MAX_U32, MAX_U32}};

	Bool setViewport(U32 minx, U32 miny, U32 width, U32 height)
	{
		ANKI_ASSERT(minx != MAX_U32 && miny != MAX_U32 && width != MAX_U32 && height != MAX_U32);
		if(m_viewport[0] != minx || m_viewport[1] != miny || m_viewport[2] != width || m_viewport[3] != height)
		{
			m_viewport = {{minx, miny, width, height}};
			return true;
		}
		return false;
	}
	/// @}

	/// @name scissor_state
	/// @{
	Array<GLsizei, 4> m_scissor = {{0, 0, MAX_I32, MAX_I32}};
	Bool8 m_scissorSet = false;

	Bool setScissor(GLsizei minx, GLsizei miny, GLsizei width, GLsizei height)
	{
		if(!m_scissorSet
			|| (m_scissor[0] != minx || m_scissor[1] != miny || m_scissor[2] != width || m_scissor[3] != height))
		{
			m_scissor = {{minx, miny, width, height}};
			m_scissorSet = true;
			return true;
		}
		return false;
	}
	/// @}

	/// @name rasterizer
	/// @{
	FillMode m_fillMode = FillMode::COUNT;

	Bool setFillMode(FillMode mode)
	{
		if(m_fillMode != mode)
		{
			m_fillMode = mode;
			return true;
		}
		return false;
	}

	FaceSelectionBit m_cullMode = static_cast<FaceSelectionBit>(0);

	Bool setCullMode(FaceSelectionBit mode)
	{
		if(m_cullMode != mode)
		{
			m_cullMode = mode;
			return true;
		}
		return false;
	}

	F32 m_polyOffsetFactor = -1.0;
	F32 m_polyOffsetUnits = -1.0;

	Bool setPolygonOffset(F32 factor, F32 units)
	{
		if(factor != m_polyOffsetFactor || units != m_polyOffsetUnits)
		{
			m_polyOffsetFactor = factor;
			m_polyOffsetUnits = units;
			return true;
		}
		return false;
	}
	/// @}

	/// @name depth_stencil
	/// @{
	Bool m_stencilTestEnabled = 2;

	Bool maybeEnableStencilTest()
	{
		Bool enable = !stencilTestDisabled(
			m_stencilFail[0], m_stencilPassDepthFail[0], m_stencilPassDepthPass[0], m_stencilCompare[0]);
		enable = enable
				 || !stencilTestDisabled(
						m_stencilFail[1], m_stencilPassDepthFail[1], m_stencilPassDepthPass[1], m_stencilCompare[1]);

		if(enable != m_stencilTestEnabled)
		{
			m_stencilTestEnabled = enable;
			return true;
		}
		return false;
	}

	Array<StencilOperation, 2> m_stencilFail = {{StencilOperation::COUNT, StencilOperation::COUNT}};
	Array<StencilOperation, 2> m_stencilPassDepthFail = {{StencilOperation::COUNT, StencilOperation::COUNT}};
	Array<StencilOperation, 2> m_stencilPassDepthPass = {{StencilOperation::COUNT, StencilOperation::COUNT}};

	Bool setStencilOperations(FaceSelectionBit face,
		StencilOperation stencilFail,
		StencilOperation stencilPassDepthFail,
		StencilOperation stencilPassDepthPass)
	{
		Bool changed = false;
		if(!!(face & FaceSelectionBit::FRONT)
			&& (m_stencilFail[0] != stencilFail || m_stencilPassDepthFail[0] != stencilPassDepthFail
				   || m_stencilPassDepthPass[0] != stencilPassDepthPass))
		{
			m_stencilFail[0] = stencilFail;
			m_stencilPassDepthFail[0] = stencilPassDepthFail;
			m_stencilPassDepthPass[0] = stencilPassDepthPass;
			changed = true;
		}

		if(!!(face & FaceSelectionBit::BACK)
			&& (m_stencilFail[1] != stencilFail || m_stencilPassDepthFail[1] != stencilPassDepthFail
				   || m_stencilPassDepthPass[1] != stencilPassDepthPass))
		{
			m_stencilFail[1] = stencilFail;
			m_stencilPassDepthFail[1] = stencilPassDepthFail;
			m_stencilPassDepthPass[1] = stencilPassDepthPass;
			changed = true;
		}

		return changed;
	}

	Array<Bool8, 2> m_glStencilFuncSeparateDirty = {{false, false}};
	Array<CompareOperation, 2> m_stencilCompare = {{CompareOperation::COUNT, CompareOperation::COUNT}};

	void setStencilCompareOperation(FaceSelectionBit face, CompareOperation comp)
	{
		if(!!(face & FaceSelectionBit::FRONT) && m_stencilCompare[0] != comp)
		{
			m_stencilCompare[0] = comp;
			m_glStencilFuncSeparateDirty[0] = true;
		}

		if(!!(face & FaceSelectionBit::BACK) && m_stencilCompare[1] != comp)
		{
			m_stencilCompare[1] = comp;
			m_glStencilFuncSeparateDirty[1] = true;
		}
	}

	static const U32 DUMMY_STENCIL_MASK = 0x969696;

	Array<U32, 2> m_stencilCompareMask = {{DUMMY_STENCIL_MASK, DUMMY_STENCIL_MASK}};

	void setStencilCompareMask(FaceSelectionBit face, U32 mask)
	{
		ANKI_ASSERT(mask != DUMMY_STENCIL_MASK && "Oops");

		if(!!(face & FaceSelectionBit::FRONT) && m_stencilCompareMask[0] != mask)
		{
			m_stencilCompareMask[0] = mask;
			m_glStencilFuncSeparateDirty[0] = true;
		}

		if(!!(face & FaceSelectionBit::BACK) && m_stencilCompareMask[1] != mask)
		{
			m_stencilCompareMask[1] = mask;
			m_glStencilFuncSeparateDirty[1] = true;
		}
	}

	Array<U32, 2> m_stencilWriteMask = {{DUMMY_STENCIL_MASK, DUMMY_STENCIL_MASK}};

	Bool setStencilWriteMask(FaceSelectionBit face, U32 mask)
	{
		ANKI_ASSERT(mask != DUMMY_STENCIL_MASK && "Oops");

		Bool changed = false;
		if(!!(face & FaceSelectionBit::FRONT) && m_stencilWriteMask[0] != mask)
		{
			m_stencilWriteMask[0] = mask;
			changed = true;
		}

		if(!!(face & FaceSelectionBit::BACK) && m_stencilWriteMask[1] != mask)
		{
			m_stencilWriteMask[1] = mask;
			changed = true;
		}

		return changed;
	}

	Array<U32, 2> m_stencilRef = {{DUMMY_STENCIL_MASK, DUMMY_STENCIL_MASK}};

	void setStencilReference(FaceSelectionBit face, U32 mask)
	{
		ANKI_ASSERT(mask != DUMMY_STENCIL_MASK && "Oops");

		if(!!(face & FaceSelectionBit::FRONT) && m_stencilRef[0] != mask)
		{
			m_stencilRef[0] = mask;
			m_glStencilFuncSeparateDirty[0] = true;
		}

		if(!!(face & FaceSelectionBit::BACK) && m_stencilRef[1] != mask)
		{
			m_stencilRef[1] = mask;
			m_glStencilFuncSeparateDirty[1] = true;
		}
	}

	Bool m_depthTestEnabled = 2; ///< 2 means don't know

	Bool maybeEnableDepthTest()
	{
		ANKI_ASSERT(m_depthWrite <= 1 && m_depthOp != CompareOperation::COUNT);
		Bool enable = m_depthWrite || m_depthOp != CompareOperation::ALWAYS;

		if(enable != m_depthTestEnabled)
		{
			m_depthTestEnabled = enable;
			return true;
		}

		return false;
	}

	Bool m_depthWrite = 2;

	Bool setDepthWrite(Bool enable)
	{
		if(m_depthWrite != enable)
		{
			m_depthWrite = enable;
			return true;
		}
		return false;
	}

	CompareOperation m_depthOp = CompareOperation::COUNT;

	Bool setDepthCompareOperation(CompareOperation op)
	{
		if(op != m_depthOp)
		{
			m_depthOp = op;
			return true;
		}
		return false;
	}
	/// @}

	/// @name color
	/// @{
	static const ColorBit INVALID_COLOR_MASK = static_cast<ColorBit>(MAX_U8);

	class ColorAttachment
	{
	public:
		ColorBit m_writeMask = INVALID_COLOR_MASK;
		Bool8 m_enableBlend = 2;
		BlendFactor m_blendSrcFactorRgb = BlendFactor::COUNT;
		BlendFactor m_blendDstFactorRgb = BlendFactor::COUNT;
		BlendFactor m_blendSrcFactorA = BlendFactor::COUNT;
		BlendFactor m_blendDstFactorA = BlendFactor::COUNT;
		BlendOperation m_blendOpRgb = BlendOperation::COUNT;
		BlendOperation m_blendOpA = BlendOperation::COUNT;
	};

	Array<ColorAttachment, MAX_COLOR_ATTACHMENTS> m_colorAtt;

	Bool setColorChannelWriteMask(U32 attachment, ColorBit mask)
	{
		if(m_colorAtt[attachment].m_writeMask != mask)
		{
			m_colorAtt[attachment].m_writeMask = mask;
			return true;
		}
		return false;
	}

	Bool maybeEnableBlend(U attidx)
	{
		ColorAttachment& att = m_colorAtt[attidx];
		Bool wantBlend = !blendingDisabled(att.m_blendSrcFactorRgb,
			att.m_blendDstFactorRgb,
			att.m_blendSrcFactorA,
			att.m_blendDstFactorA,
			att.m_blendOpRgb,
			att.m_blendOpA);

		if(wantBlend != att.m_enableBlend)
		{
			att.m_enableBlend = wantBlend;
			return true;
		}
		return false;
	}

	Bool setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA)
	{
		auto& att = m_colorAtt[attachment];
		if(att.m_blendSrcFactorRgb != srcRgb || att.m_blendDstFactorRgb != dstRgb || att.m_blendSrcFactorA != srcA
			|| att.m_blendDstFactorA != dstA)
		{
			att.m_blendSrcFactorRgb = srcRgb;
			att.m_blendDstFactorRgb = dstRgb;
			att.m_blendSrcFactorA = srcA;
			att.m_blendDstFactorA = dstA;
			return true;
		}
		return false;
	}

	Bool setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA)
	{
		auto& att = m_colorAtt[attachment];

		if(att.m_blendOpRgb != funcRgb || att.m_blendOpA != funcA)
		{
			att.m_blendOpRgb = funcRgb;
			att.m_blendOpA = funcA;
			return true;
		}
		return false;
	}
	/// @}

	/// @name resources
	/// @{
	class TextureBinding
	{
	public:
		U64 m_texViewUuid = 0;
		U64 m_samplerUuid = 0;
	};

	Array2d<TextureBinding, MAX_DESCRIPTOR_SETS, MAX_TEXTURE_BINDINGS> m_textures;

	Bool bindTextureViewAndSampler(U32 set, U32 binding, const TextureViewPtr& texView, const SamplerPtr& sampler)
	{
		TextureBinding& b = m_textures[set][binding];

		Bool dirty;
		if(b.m_texViewUuid != texView->getUuid() || b.m_samplerUuid != sampler->getUuid())
		{
			b.m_texViewUuid = texView->getUuid();
			b.m_samplerUuid = sampler->getUuid();
			dirty = true;
		}
		else
		{
			dirty = false;
		}

		return dirty;
	}

	class ShaderBufferBinding
	{
	public:
		BufferImpl* m_buff = nullptr;
		PtrSize m_offset;
		PtrSize m_range;
	};

	Array2d<ShaderBufferBinding, MAX_DESCRIPTOR_SETS, MAX_UNIFORM_BUFFER_BINDINGS> m_ubos;

	Bool bindUniformBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range)
	{
		ShaderBufferBinding& b = m_ubos[set][binding];
		b.m_buff = static_cast<BufferImpl*>(buff.get());
		b.m_offset = offset;
		b.m_range = range;
		return true;
	}

	Array2d<ShaderBufferBinding, MAX_DESCRIPTOR_SETS, MAX_STORAGE_BUFFER_BINDINGS> m_ssbos;

	Bool bindStorageBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range)
	{
		ShaderBufferBinding& b = m_ssbos[set][binding];
		b.m_buff = static_cast<BufferImpl*>(buff.get());
		b.m_offset = offset;
		b.m_range = range;
		return true;
	}

	class ImageBinding
	{
	public:
		U64 m_texViewUuid = 0;
	};

	Array2d<ImageBinding, MAX_DESCRIPTOR_SETS, MAX_IMAGE_BINDINGS> m_images;

	Bool bindImage(U32 set, U32 binding, const TextureViewPtr& img)
	{
		ImageBinding& b = m_images[set][binding];
		b.m_texViewUuid = img->getUuid();
		return true;
	}

	ShaderProgramImpl* m_prog = nullptr;

	Bool bindShaderProgram(ShaderProgramPtr prog)
	{
		ShaderProgramImpl* const progimpl = static_cast<ShaderProgramImpl*>(prog.get());
		if(progimpl != m_prog)
		{
			m_prog = progimpl;
			return true;
		}
		return false;
	}
	/// @}

	/// @name other
	/// @{
	const FramebufferImpl* m_fb = nullptr;

	Bool beginRenderPass(const FramebufferPtr& fb)
	{
		ANKI_ASSERT(!insideRenderPass() && "Already inside a renderpass");
		m_fb = static_cast<const FramebufferImpl*>(fb.get());
		m_lastSecondLevelCmdb = nullptr;
		return true;
	}

	void endRenderPass()
	{
		ANKI_ASSERT(insideRenderPass() && "Not inside a renderpass");
		if(m_lastSecondLevelCmdb)
		{
			// Renderpass had 2nd level cmdbs, need to restore the state back to default
			::new(this) StateTracker();
		}
		else
		{
			m_fb = nullptr;
		}
	}

	Bool insideRenderPass() const
	{
		return m_fb != nullptr;
	}

	CommandBufferImpl* m_lastSecondLevelCmdb = nullptr;
	/// @}

	/// @name drawcalls
	/// @{
	void checkIndexedDracall() const
	{
		ANKI_ASSERT(m_idx.m_indexType != 0 && "Forgot to bind index buffer");
		checkDrawcall();
	}

	void checkNonIndexedDrawcall() const
	{
		checkDrawcall();
	}

	void checkDrawcall() const
	{
		ANKI_ASSERT(m_viewport[1] != MAX_U16 && "Forgot to set the viewport");
		ANKI_ASSERT(m_prog && "Forgot to bound a program");
		ANKI_ASSERT((insideRenderPass() || m_secondLevel) && "Forgot to begin a render pass");
	}

	void checkDispatch() const
	{
		ANKI_ASSERT(m_prog && "Forgot to bound a program");
		ANKI_ASSERT(!insideRenderPass() && "Forgot to end the render pass");
	}
	/// @}
};
/// @}

} // end namespace anki

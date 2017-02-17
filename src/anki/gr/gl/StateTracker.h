// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
#include <anki/gr/Sampler.h>
#include <anki/gr/gl/SamplerImpl.h>
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
		PixelFormat m_fmt;
		PtrSize m_relativeOffset = MAX_PTR_SIZE;
	};

	Array<VertexAttribute, MAX_VERTEX_ATTRIBUTES> m_attribs;

	Bool setVertexAttribute(U32 location, U32 buffBinding, const PixelFormat& fmt, PtrSize relativeOffset)
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
	};

	Array<VertexBuffer, MAX_VERTEX_ATTRIBUTES> m_vertBuffs;

	Bool bindVertexBuffer(U32 binding, BufferPtr buff, PtrSize offset, PtrSize stride)
	{
		VertexBuffer& b = m_vertBuffs[binding];
		b.m_buff = buff->m_impl.get();
		b.m_offset = offset;
		b.m_stride = stride;
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
		m_idx.m_buff = buff->m_impl.get();
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
	Array<U16, 4> m_viewport = {{MAX_U16, MAX_U16, MAX_U16, MAX_U16}};

	Bool setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy)
	{
		ANKI_ASSERT(minx != MAX_U16 && miny != MAX_U16 && maxx != MAX_U16 && maxy != MAX_U16);
		if(m_viewport[0] != minx || m_viewport[1] != miny || m_viewport[2] != maxx || m_viewport[3] != maxy)
		{
			m_viewport = {{minx, miny, maxx, maxy}};
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
		TextureImpl* m_tex = nullptr;
		SamplerImpl* m_sampler = reinterpret_cast<SamplerImpl*>(0x1);
		DepthStencilAspectBit m_aspect;
	};

	Array2d<TextureBinding, MAX_DESCRIPTOR_SETS, MAX_TEXTURE_BINDINGS> m_textures;

	Bool bindTexture(
		U32 set, U32 binding, TexturePtr tex, DepthStencilAspectBit aspect, Bool& texChanged, Bool& samplerChanged)
	{
		TextureBinding& b = m_textures[set][binding];
		TextureImpl* texi = tex->m_impl.get();

		texChanged = false;
		samplerChanged = false;

		if(texi != b.m_tex)
		{
			b.m_tex = texi;
			texChanged = true;
		}

		if(b.m_sampler != nullptr)
		{
			b.m_sampler = nullptr;
			samplerChanged = true;
		}

		b.m_aspect = aspect;
		return samplerChanged || texChanged;
	}

	Bool bindTextureAndSampler(U32 set, U32 binding, TexturePtr tex, SamplerPtr sampler, DepthStencilAspectBit aspect)
	{
		TextureBinding& b = m_textures[set][binding];
		b.m_tex = tex->m_impl.get();
		b.m_sampler = sampler->m_impl.get();
		b.m_aspect = aspect;
		return true;
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
		b.m_buff = buff->m_impl.get();
		b.m_offset = offset;
		b.m_range = range;
		return true;
	}

	Array2d<ShaderBufferBinding, MAX_DESCRIPTOR_SETS, MAX_STORAGE_BUFFER_BINDINGS> m_ssbos;

	Bool bindStorageBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range)
	{
		ShaderBufferBinding& b = m_ssbos[set][binding];
		b.m_buff = buff->m_impl.get();
		b.m_offset = offset;
		b.m_range = range;
		return true;
	}

	class ImageBinding
	{
	public:
		TextureImpl* m_tex = nullptr;
		U8 m_level;
	};

	Array2d<ImageBinding, MAX_DESCRIPTOR_SETS, MAX_IMAGE_BINDINGS> m_images;

	Bool bindImage(U32 set, U32 binding, TexturePtr img, U32 level)
	{
		ImageBinding& b = m_images[set][binding];
		b.m_tex = img->m_impl.get();
		b.m_level = level;
		return true;
	}

	ShaderProgramImpl* m_prog = nullptr;

	Bool bindShaderProgram(ShaderProgramPtr prog)
	{
		if(prog->m_impl.get() != m_prog)
		{
			m_prog = prog->m_impl.get();
			return true;
		}
		return false;
	}
	/// @}

	/// @name other
	/// @{
	FramebufferImpl* m_fb = nullptr;

	Bool beginRenderPass(const FramebufferPtr& fb)
	{
		ANKI_ASSERT(!insideRenderPass() && "Already inside a renderpass");
		m_fb = fb->m_impl.get();
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

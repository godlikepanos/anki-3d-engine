// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/Common.h>
#include <anki/gr/ShaderProgram.h>
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
	Bool8 m_mayContainUnsetState = true;

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

	template<typename TFunc>
	void setVertexAttribute(U32 location, U32 buffBinding, const PixelFormat& fmt, PtrSize relativeOffset, TFunc func)
	{
		VertexAttribute& attrib = m_attribs[location];
		if(attrib.m_buffBinding != buffBinding || attrib.m_fmt != fmt || attrib.m_relativeOffset != relativeOffset)
		{
			func();
			attrib.m_buffBinding = buffBinding;
			attrib.m_fmt = fmt;
			attrib.m_relativeOffset = relativeOffset;
		}
	}

	PtrSize m_indexBuffOffset = MAX_PTR_SIZE;
	GLenum m_indexType = 0;
	/// @}

	/// @name input_assembly
	/// @{
	U8 m_primitiveRestart = 2;

	template<typename TFunc>
	void enablePrimitiveRestart(Bool enable, TFunc func)
	{
		U enablei = enable ? 1 : 0;
		if(enablei != m_primitiveRestart)
		{
			m_primitiveRestart = enablei;
			func();
		}
	}
	/// @}

	/// @name viewport_state
	/// @{
	Array<U16, 4> m_viewport = {{MAX_U16, MAX_U16, MAX_U16, MAX_U16}};

	template<typename TFunc>
	void setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy, TFunc func)
	{
		ANKI_ASSERT(minx != MAX_U16 && miny != MAX_U16 && maxx != MAX_U16 && maxy != MAX_U16);
		if(m_viewport[0] != minx || m_viewport[1] != miny || m_viewport[2] != maxx || m_viewport[3] != maxy)
		{
			m_viewport = {minx, miny, maxx, maxy};
			func();
		}
	}
	/// @}

	/// @name rasterizer
	/// @{
	FillMode m_fillMode = FillMode::COUNT;

	template<typename TFunc>
	void setFillMode(FillMode mode, TFunc func)
	{
		if(m_fillMode != mode)
		{
			m_fillMode = mode;
			func();
		}
	}

	FaceSelectionMask m_cullMode = static_cast<FaceSelectionMask>(0);

	template<typename TFunc>
	void setCullMode(FaceSelectionMask mode, TFunc func)
	{
		if(m_cullMode != mode)
		{
			m_cullMode = mode;
			func();
		}
	}

	F32 m_polyOffsetFactor = -1.0;
	F32 m_polyOffsetUnits = -1.0;

	template<typename TFunc>
	void setPolygonOffset(F32 factor, F32 units, TFunc func)
	{
		if(factor != m_polyOffsetFactor || units != m_polyOffsetUnits)
		{
			m_polyOffsetFactor = factor;
			m_polyOffsetUnits = units;
			func();
		}
	}
	/// @}

	/// @name depth_stencil
	/// @{
	Bool8 m_stencilTestEnabled = 2;

	template<typename TFunc>
	void maybeEnableStencilTest(TFunc func)
	{
		Bool enable = !stencilTestDisabled(
			m_stencilFail[0], m_stencilPassDepthFail[0], m_stencilPassDepthPass[0], m_stencilCompare[0]);
		enable = enable
			|| !stencilTestDisabled(
					 m_stencilFail[1], m_stencilPassDepthFail[1], m_stencilPassDepthPass[1], m_stencilCompare[1]);

		if(enable != m_stencilTestEnabled)
		{
			m_stencilTestEnabled = enable;
			func(enable);
		}
	}

	Array<StencilOperation, 2> m_stencilFail = {{StencilOperation::COUNT, StencilOperation::COUNT}};
	Array<StencilOperation, 2> m_stencilPassDepthFail = {{StencilOperation::COUNT, StencilOperation::COUNT}};
	Array<StencilOperation, 2> m_stencilPassDepthPass = {{StencilOperation::COUNT, StencilOperation::COUNT}};

	template<typename TFunc>
	void setStencilOperations(FaceSelectionMask face,
		StencilOperation stencilFail,
		StencilOperation stencilPassDepthFail,
		StencilOperation stencilPassDepthPass,
		TFunc func)
	{
		Bool changed = false;
		if(!!(face & FaceSelectionMask::FRONT)
			&& (m_stencilFail[0] != stencilFail || m_stencilPassDepthFail[0] != stencilPassDepthFail
				   || m_stencilPassDepthPass[0] != stencilPassDepthPass))
		{
			m_stencilFail[0] = stencilFail;
			m_stencilPassDepthFail[0] = stencilPassDepthFail;
			m_stencilPassDepthPass[0] = stencilPassDepthPass;
			changed = true;
		}

		if(!!(face & FaceSelectionMask::BACK)
			&& (m_stencilFail[1] != stencilFail || m_stencilPassDepthFail[1] != stencilPassDepthFail
				   || m_stencilPassDepthPass[1] != stencilPassDepthPass))
		{
			m_stencilFail[1] = stencilFail;
			m_stencilPassDepthFail[1] = stencilPassDepthFail;
			m_stencilPassDepthPass[1] = stencilPassDepthPass;
			changed = true;
		}

		if(changed)
		{
			func();
		}
	}

	Array<Bool8, 2> m_glStencilFuncSeparateDirty = {{false, false}};
	Array<CompareOperation, 2> m_stencilCompare = {{CompareOperation::COUNT, CompareOperation::COUNT}};

	void setStencilCompareFunction(FaceSelectionMask face, CompareOperation comp)
	{
		if(!!(face & FaceSelectionMask::FRONT) && m_stencilCompare[0] != comp)
		{
			m_stencilCompare[0] = comp;
			m_glStencilFuncSeparateDirty[0] = true;
		}

		if(!!(face & FaceSelectionMask::BACK) && m_stencilCompare[1] != comp)
		{
			m_stencilCompare[1] = comp;
			m_glStencilFuncSeparateDirty[1] = true;
		}
	}

	static const U32 DUMMY_STENCIL_MASK = 0x969696;

	Array<U32, 2> m_stencilCompareMask = {{DUMMY_STENCIL_MASK, DUMMY_STENCIL_MASK}};

	void setStencilCompareMask(FaceSelectionMask face, U32 mask)
	{
		ANKI_ASSERT(mask != DUMMY_STENCIL_MASK && "Oops");

		if(!!(face & FaceSelectionMask::FRONT) && m_stencilCompareMask[0] != mask)
		{
			m_stencilCompareMask[0] = mask;
			m_glStencilFuncSeparateDirty[0] = true;
		}

		if(!!(face & FaceSelectionMask::BACK) && m_stencilCompareMask[1] != mask)
		{
			m_stencilCompareMask[1] = mask;
			m_glStencilFuncSeparateDirty[1] = true;
		}
	}

	Array<U32, 2> m_stencilWriteMask = {{DUMMY_STENCIL_MASK, DUMMY_STENCIL_MASK}};

	template<typename TFunc>
	void setStencilWriteMask(FaceSelectionMask face, U32 mask, TFunc func)
	{
		ANKI_ASSERT(mask != DUMMY_STENCIL_MASK && "Oops");

		Bool changed = false;
		if(!!(face & FaceSelectionMask::FRONT) && m_stencilWriteMask[0] != mask)
		{
			m_stencilWriteMask[0] = mask;
			changed = true;
		}

		if(!!(face & FaceSelectionMask::BACK) && m_stencilWriteMask[1] != mask)
		{
			m_stencilWriteMask[1] = mask;
			changed = true;
		}

		if(changed)
		{
			func();
		}
	}

	Array<U32, 2> m_stencilRef = {{DUMMY_STENCIL_MASK, DUMMY_STENCIL_MASK}};

	void setStencilReference(FaceSelectionMask face, U32 mask)
	{
		ANKI_ASSERT(mask != DUMMY_STENCIL_MASK && "Oops");

		if(!!(face & FaceSelectionMask::FRONT) && m_stencilRef[0] != mask)
		{
			m_stencilRef[0] = mask;
			m_glStencilFuncSeparateDirty[0] = true;
		}

		if(!!(face & FaceSelectionMask::BACK) && m_stencilRef[1] != mask)
		{
			m_stencilRef[1] = mask;
			m_glStencilFuncSeparateDirty[1] = true;
		}
	}

	Bool8 m_depthTestEnabled = 2; ///< 2 means don't know

	template<typename TFunc>
	void maybeEnableDepthTest(TFunc func)
	{
		ANKI_ASSERT(m_depthWrite != 2 && m_depthOp != CompareOperation::COUNT);
		Bool enable = m_depthWrite || m_depthOp != CompareOperation::ALWAYS;

		if(enable != m_depthTestEnabled)
		{
			m_depthTestEnabled = enable;
			func(enable);
		}
	}

	Bool8 m_depthWrite = 2;

	template<typename TFunc>
	void enableDepthWrite(Bool enable, TFunc func)
	{
		if(m_depthWrite != enable)
		{
			m_depthWrite = enable;
			func();
		}
	}

	CompareOperation m_depthOp = CompareOperation::COUNT;

	template<typename TFunc>
	void setDepthCompareFunction(CompareOperation op, TFunc func)
	{
		if(op != m_depthOp)
		{
			m_depthOp = op;
			func();
		}
	}
	/// @}

	/// @name color
	/// @{
	static const ColorBit INVALID_COLOR_MASK = static_cast<ColorBit>(MAX_U8);
	Array<ColorBit, MAX_COLOR_ATTACHMENTS> m_colorWriteMasks = {
		{INVALID_COLOR_MASK, INVALID_COLOR_MASK, INVALID_COLOR_MASK, INVALID_COLOR_MASK}};

	template<typename TFunc>
	void setColorChannelWriteMask(U32 attachment, ColorBit mask, TFunc func)
	{
		if(m_colorWriteMasks[attachment] != mask)
		{
			m_colorWriteMasks[attachment] = mask;
			func();
		}
	}

	Array<BlendMethod, MAX_COLOR_ATTACHMENTS> m_blendSrcMethod = {
		{BlendMethod::COUNT, BlendMethod::COUNT, BlendMethod::COUNT, BlendMethod::COUNT}};
	Array<BlendMethod, MAX_COLOR_ATTACHMENTS> m_blendDstMethod = {
		{BlendMethod::COUNT, BlendMethod::COUNT, BlendMethod::COUNT, BlendMethod::COUNT}};

	template<typename TFunc>
	void setBlendMethods(U32 attachment, BlendMethod src, BlendMethod dst, TFunc func)
	{
		if(m_blendSrcMethod[attachment] != src || m_blendDstMethod[attachment] != dst)
		{
			m_blendSrcMethod[attachment] = src;
			m_blendDstMethod[attachment] = dst;
			func();
		}
	}

	Array<BlendFunction, MAX_COLOR_ATTACHMENTS> m_blendFuncs = {
		{BlendFunction::COUNT, BlendFunction::COUNT, BlendFunction::COUNT, BlendFunction::COUNT}};

	template<typename TFunc>
	void setBlendFunction(U32 attachment, BlendFunction func, TFunc funct)
	{
		if(m_blendFuncs[attachment] != func)
		{
			m_blendFuncs[attachment] = func;
			funct();
		}
	}
	/// @}

	/// @name resources
	/// @{
	U64 m_progUuid = MAX_U64;

	template<typename TFunc>
	void bindShaderProgram(ShaderProgramPtr prog, TFunc func)
	{
		if(prog->getUuid() != m_progUuid)
		{
			m_progUuid = prog->getUuid();
			func();
		}
	}
	/// @}

	/// @name other
	/// @{
	U64 m_fbUuid = MAX_U64;
	U8 m_colorBuffCount = MAX_U8;
	Bool8 m_fbHasDepth = 2;
	Bool8 m_fbHasStencil = 2;

	template<typename TFunc>
	void beginRenderPass(const FramebufferPtr& fb, TFunc func)
	{
		ANKI_ASSERT(m_fbUuid == MAX_U64 && "Already inside a renderpass");

		const FramebufferImpl& impl = *fb->m_impl;
		m_fbUuid = fb->getUuid();
		m_colorBuffCount = impl.getColorBufferCount();
		m_fbHasDepth = impl.hasDepthBuffer();
		m_fbHasStencil = impl.hasStencilBuffer();
		func();
	}

	void endRenderPass()
	{
		ANKI_ASSERT(m_fbUuid != MAX_U64 && "Not inside a renderpass");
		m_fbUuid = MAX_U64;
	}

	Bool insideRenderPass() const
	{
		return m_fbUuid != MAX_U64;
	}
	/// @}

	/// @name drawcalls
	/// @{
	void checkIndexedDracall() const
	{
		ANKI_ASSERT(m_indexType != 0 && "Forgot to bind index buffer");
		checkDrawcall();
	}

	void checkNonIndexedDrawcall() const
	{
		checkDrawcall();
	}

	void checkDrawcall() const
	{
		ANKI_ASSERT(m_viewport[1] != MAX_U16 && "Forgot to set the viewport");
		ANKI_ASSERT(m_progUuid != MAX_U64 && "Forgot to bound a program");
		ANKI_ASSERT(m_fbUuid != MAX_U64 && "Forgot to begin a render pass");
	}

	void checkDispatch() const
	{
		ANKI_ASSERT(m_progUuid != MAX_U64 && "Forgot to bound a program");
		ANKI_ASSERT(m_fbUuid == MAX_U64 && "Forgot to end the render pass");
	}
	/// @}
};
/// @}

} // end namespace anki

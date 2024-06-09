// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DCommon.h>
#include <AnKi/Gr/D3D/D3DShaderProgram.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

/// @addtogroup d3d
/// @{

/// This is the way the command buffer sets the graphics state.
class GraphicsStateTracker
{
	friend class GraphicsPipelineFactory;

public:
	void bindVertexBuffer(U32 binding, VertexStepRate stepRate)
	{
		if(!m_state.m_vert.m_bindingsSetMask.get(binding) || m_state.m_vert.m_bindings[binding].m_stepRate != stepRate)
		{
			m_state.m_vert.m_bindingsSetMask.set(binding);
			m_state.m_vert.m_bindings[binding].m_stepRate = stepRate;
			m_hashes.m_vert = 0;
		}
	}

	void setVertexAttribute(VertexAttributeSemantic attribute, U32 buffBinding, Format fmt, U32 relativeOffset)
	{
		auto& attr = m_state.m_vert.m_attribs[attribute];
		if(!m_state.m_vert.m_attribsSetMask.get(attribute) || attr.m_fmt != fmt || attr.m_binding != buffBinding
		   || attr.m_relativeOffset != relativeOffset)
		{
			attr.m_fmt = fmt;
			attr.m_binding = buffBinding;
			attr.m_relativeOffset = relativeOffset;
			m_state.m_vert.m_attribsSetMask.set(attribute);
			m_hashes.m_vert = 0;
		}
	}

	void setFillMode(FillMode mode)
	{
		ANKI_ASSERT(mode < FillMode::kCount);
		if(m_state.m_rast.m_fillMode != mode)
		{
			m_state.m_rast.m_fillMode = mode;
			m_hashes.m_rast = 0;
		}
	}

	void setCullMode(FaceSelectionBit mode)
	{
		if(m_state.m_rast.m_cullMode != mode)
		{
			m_state.m_rast.m_cullMode = mode;
			m_hashes.m_rast = 0;
		}
	}

	void setStencilOperations(FaceSelectionBit face, StencilOperation stencilFail, StencilOperation stencilPassDepthFail,
							  StencilOperation stencilPassDepthPass)
	{
		ANKI_ASSERT(face != FaceSelectionBit::kNone);
		if(!!(face & FaceSelectionBit::kFront)
		   && (m_state.m_stencil.m_fail[0] != stencilFail || m_state.m_stencil.m_stencilPassDepthFail[0] != stencilPassDepthFail
			   || m_state.m_stencil.m_stencilPassDepthPass[0] != stencilPassDepthPass))
		{
			m_state.m_stencil.m_fail[0] = stencilFail;
			m_state.m_stencil.m_stencilPassDepthFail[0] = stencilPassDepthFail;
			m_state.m_stencil.m_stencilPassDepthPass[0] = stencilPassDepthPass;
			m_hashes.m_depthStencil = 0;
		}

		if(!!(face & FaceSelectionBit::kBack)
		   && (m_state.m_stencil.m_fail[1] != stencilFail || m_state.m_stencil.m_stencilPassDepthFail[1] != stencilPassDepthFail
			   || m_state.m_stencil.m_stencilPassDepthPass[1] != stencilPassDepthPass))
		{
			m_state.m_stencil.m_fail[1] = stencilFail;
			m_state.m_stencil.m_stencilPassDepthFail[1] = stencilPassDepthFail;
			m_state.m_stencil.m_stencilPassDepthPass[1] = stencilPassDepthPass;
			m_hashes.m_depthStencil = 0;
		}
	}

	void setStencilCompareOperation(FaceSelectionBit face, CompareOperation comp)
	{
		ANKI_ASSERT(face != FaceSelectionBit::kNone);
		if(!!(face & FaceSelectionBit::kFront) && m_state.m_stencil.m_compare[0] != comp)
		{
			m_state.m_stencil.m_compare[0] = comp;
			m_hashes.m_depthStencil = 0;
		}

		if(!!(face & FaceSelectionBit::kBack) && m_state.m_stencil.m_compare[1] != comp)
		{
			m_state.m_stencil.m_compare[1] = comp;
			m_hashes.m_depthStencil = 0;
		}
	}

	void setStencilCompareMask(FaceSelectionBit face, U32 mask)
	{
		ANKI_ASSERT(face != FaceSelectionBit::kNone);
		if(!!(face & FaceSelectionBit::kFront) && m_state.m_stencil.m_compareMask[0] != mask)
		{
			m_state.m_stencil.m_compareMask[0] = mask;
			m_hashes.m_depthStencil = 0;
		}

		if(!!(face & FaceSelectionBit::kBack) && m_state.m_stencil.m_compareMask[1] != mask)
		{
			m_state.m_stencil.m_compareMask[1] = mask;
			m_hashes.m_depthStencil = 0;
		}
	}

	void setStencilWriteMask(FaceSelectionBit face, U32 mask)
	{
		ANKI_ASSERT(face != FaceSelectionBit::kNone);
		if(!!(face & FaceSelectionBit::kFront) && m_state.m_stencil.m_writeMask[0] != mask)
		{
			m_state.m_stencil.m_writeMask[0] = mask;
			m_hashes.m_depthStencil = 0;
		}

		if(!!(face & FaceSelectionBit::kBack) && m_state.m_stencil.m_writeMask[1] != mask)
		{
			m_state.m_stencil.m_writeMask[1] = mask;
			m_hashes.m_depthStencil = 0;
		}
	}

	void setStencilReference([[maybe_unused]] FaceSelectionBit face, U32 ref)
	{
		ANKI_ASSERT(face == FaceSelectionBit::kFrontAndBack && "D3D only supports a single value for both sides");
		m_dynState.m_stencilRefMask = ref;
		m_dynState.m_stencilRefMaskDirty = true;
	}

	void setDepthWrite(Bool enable)
	{
		if(m_state.m_depth.m_writeEnabled != enable)
		{
			m_state.m_depth.m_writeEnabled = enable;
			m_hashes.m_depthStencil = 0;
		}
	}

	void setDepthCompareOperation(CompareOperation op)
	{
		ANKI_ASSERT(op < CompareOperation::kCount);
		if(m_state.m_depth.m_compare != op)
		{
			m_state.m_depth.m_compare = op;
			m_hashes.m_depthStencil = 0;
		}
	}

	void setPolygonOffset(F32 factor, F32 units)
	{
		if(m_state.m_rast.m_depthBias != factor || m_state.m_rast.m_slopeScaledDepthBias != factor)
		{
			m_state.m_rast.m_depthBias = factor;
			m_state.m_rast.m_slopeScaledDepthBias = units;
			m_state.m_rast.m_depthBiasClamp = 0.0f;
			m_hashes.m_rast = 0;
		}
	}

	void setAlphaToCoverage(Bool enable)
	{
		if(m_state.m_blend.m_alphaToCoverage != enable)
		{
			m_state.m_blend.m_alphaToCoverage = enable;
			m_hashes.m_blend = 0;
		}
	}

	void setColorChannelWriteMask(U32 attachment, ColorBit mask)
	{
		if(m_state.m_blend.m_colorRts[attachment].m_colorWritemask != mask)
		{
			m_state.m_blend.m_colorRts[attachment].m_colorWritemask = mask;
			m_hashes.m_blend = 0;
		}
	}

	void setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA)
	{
		auto& rt = m_state.m_blend.m_colorRts[attachment];
		if(rt.m_srcRgb != srcRgb || rt.m_srcA != srcA || rt.m_dstRgb != dstRgb || rt.m_dstA != dstA)
		{
			rt.m_srcRgb = srcRgb;
			rt.m_srcA = srcA;
			rt.m_dstRgb = dstRgb;
			rt.m_dstA = dstA;
			m_hashes.m_blend = 0;
		}
	}

	void setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA)
	{
		auto& rt = m_state.m_blend.m_colorRts[attachment];
		if(rt.m_funcRgb != funcRgb || rt.m_funcA != funcA)
		{
			rt.m_funcRgb = funcRgb;
			rt.m_funcA = funcA;
			m_hashes.m_blend = 0;
		}
	}

	void beginRenderPass(ConstWeakArray<Format> colorFormats, Format depthStencilFormat, U32 rtsWidth, U32 rtsHeight)
	{
		m_state.m_misc.m_colorRtMask.unsetAll();
		for(U8 i = 0; i < colorFormats.getSize(); ++i)
		{
			ANKI_ASSERT(colorFormats[i] != Format::kNone);
			m_state.m_misc.m_colorRtFormats[i] = colorFormats[i];
			m_state.m_misc.m_colorRtMask.set(i);
		}
		m_state.m_misc.m_depthStencilFormat = depthStencilFormat;
		m_hashes.m_misc = 0; // Always mark it dirty because calling beginRenderPass is a rare occurance and we want to avoid extra checks

		m_rtsWidth = rtsWidth;
		m_rtsHeight = rtsHeight;

		// Scissor needs to be re-adjusted if it's in its default size
		m_dynState.m_scissorDirty = true;
	}

	void bindShaderProgram(const ShaderProgramImpl* prog)
	{
		ANKI_ASSERT(prog);
		const ShaderReflection& refl = prog->m_refl;
		if(m_state.m_vert.m_activeAttribs != refl.m_vertex.m_vertexAttributeMask)
		{
			m_state.m_vert.m_activeAttribs = refl.m_vertex.m_vertexAttributeMask;
			m_hashes.m_vert = 0;
		}

		if(m_state.m_misc.m_colorRtMask != refl.m_fragment.m_colorAttachmentWritemask)
		{
			m_state.m_misc.m_colorRtMask = refl.m_fragment.m_colorAttachmentWritemask;
			m_hashes.m_misc = 0;
		}

		if(m_state.m_shaderProg != prog)
		{
			m_state.m_shaderProg = prog;
			m_hashes.m_shaderProg = 0;
		}
	}

	void setPrimitiveTopology(PrimitiveTopology topology)
	{
		if(m_state.m_misc.m_topology != topology)
		{
			m_state.m_misc.m_topology = topology;
			m_hashes.m_misc = 0;
			m_dynState.m_topology = topology;
			m_dynState.m_topologyDirty = true;
		}
	}

	void setViewport(U32 minx, U32 miny, U32 width, U32 height)
	{
		if(m_dynState.m_viewport[0] != minx || m_dynState.m_viewport[1] != miny || m_dynState.m_viewport[2] != width
		   || m_dynState.m_viewport[3] != height)
		{
			m_dynState.m_viewport[0] = minx;
			m_dynState.m_viewport[1] = miny;
			m_dynState.m_viewport[2] = width;
			m_dynState.m_viewport[3] = height;
			m_dynState.m_viewportDirty = true;
		}
	}

	void setScissor(U32 minx, U32 miny, U32 width, U32 height)
	{
		if(m_dynState.m_scissor[0] != minx || m_dynState.m_scissor[1] != miny || m_dynState.m_scissor[2] != width
		   || m_dynState.m_scissor[3] != height)
		{
			m_dynState.m_scissor[0] = minx;
			m_dynState.m_scissor[1] = miny;
			m_dynState.m_scissor[2] = width;
			m_dynState.m_scissor[3] = height;
			m_dynState.m_scissorDirty = true;
		}
	}

	const ShaderProgramImpl& getShaderProgram() const
	{
		ANKI_ASSERT(m_state.m_shaderProg);
		return *m_state.m_shaderProg;
	}

private:
	// The state vector
	ANKI_BEGIN_PACKED_STRUCT
	class State
	{
	public:
		class
		{
		public:
			class VertBinding
			{
			public:
				VertexStepRate m_stepRate;

				VertBinding()
				{
					// No init for opt
				}
			};

			class VertAttrib
			{
			public:
				U32 m_relativeOffset;
				Format m_fmt;
				U32 m_binding;

				VertAttrib()
				{
					// No init for opt
				}
			};

			Array<VertBinding, U32(VertexAttributeSemantic::kCount)> m_bindings;
			Array<VertAttrib, U32(VertexAttributeSemantic::kCount)> m_attribs;
			BitSet<U32(VertexAttributeSemantic::kCount)> m_activeAttribs = false;

			BitSet<U32(VertexAttributeSemantic::kCount)> m_bindingsSetMask = false;
			BitSet<U32(VertexAttributeSemantic::kCount)> m_attribsSetMask = false;
		} m_vert;

		class
		{
		public:
			FillMode m_fillMode = FillMode::kSolid;
			FaceSelectionBit m_cullMode = FaceSelectionBit::kBack;
			F32 m_depthBias = 0.0f;
			F32 m_depthBiasClamp = 0.0f;
			F32 m_slopeScaledDepthBias = 0.0f;
		} m_rast;

		class
		{
		public:
			Array<StencilOperation, 2> m_fail = {StencilOperation::kKeep, StencilOperation::kKeep};
			Array<StencilOperation, 2> m_stencilPassDepthFail = {StencilOperation::kKeep, StencilOperation::kKeep};
			Array<StencilOperation, 2> m_stencilPassDepthPass = {StencilOperation::kKeep, StencilOperation::kKeep};
			Array<CompareOperation, 2> m_compare = {CompareOperation::kAlways, CompareOperation::kAlways};
			Array<U32, 2> m_compareMask = {0x5A5A5A5A, 0x5A5A5A5A}; ///< Use a stupid number to initialize.
			Array<U32, 2> m_writeMask = {0x5A5A5A5A, 0x5A5A5A5A}; ///< Use a stupid number to initialize.
		} m_stencil;

		class
		{
		public:
			CompareOperation m_compare = CompareOperation::kLess;
			Bool m_writeEnabled = true;
		} m_depth;

		class
		{
		public:
			class Rt
			{
			public:
				ColorBit m_colorWritemask = ColorBit::kAll;
				BlendFactor m_srcRgb = BlendFactor::kOne;
				BlendFactor m_dstRgb = BlendFactor::kOne;
				BlendFactor m_srcA = BlendFactor::kZero;
				BlendFactor m_dstA = BlendFactor::kZero;
				BlendOperation m_funcRgb = BlendOperation::kAdd;
				BlendOperation m_funcA = BlendOperation::kAdd;
			};

			Array<Rt, kMaxColorRenderTargets> m_colorRts;
			Bool m_alphaToCoverage = false;
		} m_blend;

		class
		{
		public:
			Array<Format, kMaxColorRenderTargets> m_colorRtFormats = {};
			Format m_depthStencilFormat = Format::kNone;
			BitSet<kMaxColorRenderTargets> m_colorRtMask = {false};
			PrimitiveTopology m_topology = PrimitiveTopology::kTriangles;
		} m_misc;

		const ShaderProgramImpl* m_shaderProg = nullptr;
	} m_state;
	ANKI_END_PACKED_STRUCT

	class DynamicState
	{
	public:
		U32 m_stencilRefMask = 0x5A5A5A5A; ///< Use a stupid number to initialize.
		PrimitiveTopology m_topology = PrimitiveTopology::kTriangles; ///< Yes, it's both dynamic and static state

		Array<U32, 4> m_viewport = {};
		Array<U32, 4> m_scissor = {0, 0, kMaxU32, kMaxU32};

		Bool m_stencilRefMaskDirty : 1 = true;
		Bool m_topologyDirty : 1 = true;
		Bool m_viewportDirty : 1 = true;
		Bool m_scissorDirty : 1 = true;
	} m_dynState;

	class Hashes
	{
	public:
		U64 m_vert = 0;
		U64 m_rast = 0;
		U64 m_depthStencil = 0;
		U64 m_blend = 0;
		U64 m_misc = 0;
		U64 m_shaderProg = 0;
	} m_hashes;

	U64 m_globalHash = 0;
	U32 m_rtsWidth = 0;
	U32 m_rtsHeight = 0;

	Bool updateHashes();
};

class GraphicsPipelineFactory
{
public:
	~GraphicsPipelineFactory();

	/// Write state to the command buffer.
	/// @note It's thread-safe.
	void flushState(GraphicsStateTracker& state, D3D12GraphicsCommandListX& cmdList);

private:
	GrHashMap<U64, ID3D12PipelineState*> m_map;
	RWMutex m_mtx;
};
/// @}

} // end namespace anki

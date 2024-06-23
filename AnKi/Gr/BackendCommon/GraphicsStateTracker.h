// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/BackendCommon/Common.h>
#include <AnKi/Gr/ShaderProgram.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// This is the way the command buffer sets the graphics state.
class GraphicsStateTracker
{
	friend class GraphicsPipelineFactory;

public:
	void bindVertexBuffer(U32 binding, VertexStepRate stepRate
#if ANKI_GR_BACKEND_VULKAN
						  ,
						  U32 stride
#endif
	)
	{
		auto& vertState = m_staticState.m_vert;
		if(!vertState.m_bindingsSetMask.get(binding) || vertState.m_bindings[binding].m_stepRate != stepRate
#if ANKI_GR_BACKEND_VULKAN
		   || vertState.m_bindings[binding].m_stride != stride
#endif
		)
		{
			vertState.m_bindingsSetMask.set(binding);
#if ANKI_GR_BACKEND_VULKAN
			vertState.m_bindings[binding].m_stride = stride;
#endif
			vertState.m_bindings[binding].m_stepRate = stepRate;
			m_hashes.m_vert = 0;
		}
	}

	void setVertexAttribute(VertexAttributeSemantic attribute, U32 buffBinding, Format fmt, U32 relativeOffset)
	{
		auto& attr = m_staticState.m_vert.m_attribs[attribute];
		if(!m_staticState.m_vert.m_attribsSetMask.get(attribute) || attr.m_fmt != fmt || attr.m_binding != buffBinding
		   || attr.m_relativeOffset != relativeOffset)
		{
			attr.m_fmt = fmt;
			attr.m_binding = buffBinding;
			attr.m_relativeOffset = relativeOffset;
			m_staticState.m_vert.m_attribsSetMask.set(attribute);
			m_hashes.m_vert = 0;
		}
	}

	void setFillMode(FillMode mode)
	{
		ANKI_ASSERT(mode < FillMode::kCount);
		if(m_staticState.m_rast.m_fillMode != mode)
		{
			m_staticState.m_rast.m_fillMode = mode;
			m_hashes.m_rast = 0;
		}
	}

	void setCullMode(FaceSelectionBit mode)
	{
		if(m_staticState.m_rast.m_cullMode != mode)
		{
			m_staticState.m_rast.m_cullMode = mode;
			m_hashes.m_rast = 0;
		}
	}

	void setStencilOperations(FaceSelectionBit face, StencilOperation stencilFail, StencilOperation stencilPassDepthFail,
							  StencilOperation stencilPassDepthPass)
	{
		ANKI_ASSERT(face != FaceSelectionBit::kNone);
		auto& front = m_staticState.m_stencil.m_face[0];
		if(!!(face & FaceSelectionBit::kFront)
		   && (front.m_fail != stencilFail || front.m_stencilPassDepthFail != stencilPassDepthFail
			   || front.m_stencilPassDepthPass != stencilPassDepthPass))
		{
			front.m_fail = stencilFail;
			front.m_stencilPassDepthFail = stencilPassDepthFail;
			front.m_stencilPassDepthPass = stencilPassDepthPass;
			m_hashes.m_depthStencil = 0;
		}

		auto& back = m_staticState.m_stencil.m_face[1];
		if(!!(face & FaceSelectionBit::kBack)
		   && (back.m_fail != stencilFail || back.m_stencilPassDepthFail != stencilPassDepthFail
			   || back.m_stencilPassDepthPass != stencilPassDepthPass))
		{
			back.m_fail = stencilFail;
			back.m_stencilPassDepthFail = stencilPassDepthFail;
			back.m_stencilPassDepthPass = stencilPassDepthPass;
			m_hashes.m_depthStencil = 0;
		}
	}

	void setStencilCompareOperation(FaceSelectionBit face, CompareOperation comp)
	{
		ANKI_ASSERT(face != FaceSelectionBit::kNone);
		if(!!(face & FaceSelectionBit::kFront) && m_staticState.m_stencil.m_face[0].m_compare != comp)
		{
			m_staticState.m_stencil.m_face[0].m_compare = comp;
			m_hashes.m_depthStencil = 0;
		}

		if(!!(face & FaceSelectionBit::kBack) && m_staticState.m_stencil.m_face[1].m_compare != comp)
		{
			m_staticState.m_stencil.m_face[1].m_compare = comp;
			m_hashes.m_depthStencil = 0;
		}
	}

	void setStencilCompareMask(FaceSelectionBit face, U32 mask)
	{
		ANKI_ASSERT(face != FaceSelectionBit::kNone);

#if ANKI_GR_BACKEND_VULKAN
		if(!!(face & FaceSelectionBit::kFront) && m_dynState.m_stencilFaces[0].m_compareMask != mask)
		{
			m_dynState.m_stencilFaces[0].m_compareMask = mask;
			m_dynState.m_stencilCompareMaskDirty = true;
		}

		if(!!(face & FaceSelectionBit::kBack) && m_dynState.m_stencilFaces[1].m_compareMask != mask)
		{
			m_dynState.m_stencilFaces[1].m_compareMask = mask;
			m_dynState.m_stencilCompareMaskDirty = true;
		}
#else
		if(!!(face & FaceSelectionBit::kFront) && m_staticState.m_stencil.m_face[0].m_compareMask != mask)
		{
			m_staticState.m_stencil.m_face[0].m_compareMask = mask;
			m_hashes.m_depthStencil = 0;
		}

		if(!!(face & FaceSelectionBit::kBack) && m_staticState.m_stencil.m_face[1].m_compareMask != mask)
		{
			m_staticState.m_stencil.m_face[1].m_compareMask = mask;
			m_hashes.m_depthStencil = 0;
		}
#endif
	}

	void setStencilWriteMask(FaceSelectionBit face, U32 mask)
	{
		ANKI_ASSERT(face != FaceSelectionBit::kNone);

#if ANKI_GR_BACKEND_VULKAN
		if(!!(face & FaceSelectionBit::kFront) && m_dynState.m_stencilFaces[0].m_writeMask != mask)
		{
			m_dynState.m_stencilFaces[0].m_writeMask = mask;
			m_dynState.m_stencilWriteMaskDirty = true;
		}

		if(!!(face & FaceSelectionBit::kBack) && m_dynState.m_stencilFaces[1].m_writeMask != mask)
		{
			m_dynState.m_stencilFaces[1].m_writeMask = mask;
			m_dynState.m_stencilWriteMaskDirty = true;
		}
#else
		if(!!(face & FaceSelectionBit::kFront) && m_staticState.m_stencil.m_face[0].m_writeMask != mask)
		{
			m_staticState.m_stencil.m_face[0].m_writeMask = mask;
			m_hashes.m_depthStencil = 0;
		}

		if(!!(face & FaceSelectionBit::kBack) && m_staticState.m_stencil.m_face[1].m_writeMask != mask)
		{
			m_staticState.m_stencil.m_face[1].m_writeMask = mask;
			m_hashes.m_depthStencil = 0;
		}
#endif
	}

	void setStencilReference(FaceSelectionBit face, U32 ref)
	{
		ANKI_ASSERT(face != FaceSelectionBit::kNone);
		ANKI_ASSERT((ANKI_GR_BACKEND_VULKAN || face == FaceSelectionBit::kFrontAndBack) && "D3D only supports a single value for both sides");

		if(!!(face & FaceSelectionBit::kFront) && m_dynState.m_stencilFaces[0].m_ref != ref)
		{
			m_dynState.m_stencilFaces[0].m_ref = ref;
			m_dynState.m_stencilRefDirty = true;
		}

		if(!!(face & FaceSelectionBit::kBack) && m_dynState.m_stencilFaces[1].m_ref != ref)
		{
			m_dynState.m_stencilFaces[1].m_ref = ref;
			m_dynState.m_stencilRefDirty = true;
		}
	}

	void setDepthWrite(Bool enable)
	{
		if(m_staticState.m_depth.m_writeEnabled != enable)
		{
			m_staticState.m_depth.m_writeEnabled = enable;
			m_hashes.m_depthStencil = 0;
		}
	}

	void setDepthCompareOperation(CompareOperation op)
	{
		ANKI_ASSERT(op < CompareOperation::kCount);
		if(m_staticState.m_depth.m_compare != op)
		{
			m_staticState.m_depth.m_compare = op;
			m_hashes.m_depthStencil = 0;
		}
	}

	void setPolygonOffset(F32 factor, F32 units)
	{
#if ANKI_GR_BACKEND_VULKAN
		if(m_dynState.m_depthBiasConstantFactor != factor || m_dynState.m_depthBiasSlopeFactor != units)
		{
			m_dynState.m_depthBiasConstantFactor = factor;
			m_dynState.m_depthBiasSlopeFactor = units;
			m_dynState.m_depthBiasClamp = 0.0f;
			m_dynState.m_depthBiasDirty = true;

			m_staticState.m_rast.m_depthBiasEnabled = factor != 0.0f || units != 0.0f;
		}
#else
		if(m_staticState.m_rast.m_depthBias != factor || m_staticState.m_rast.m_slopeScaledDepthBias != units)
		{
			m_staticState.m_rast.m_depthBias = factor;
			m_staticState.m_rast.m_slopeScaledDepthBias = units;
			m_staticState.m_rast.m_depthBiasClamp = 0.0f;
			m_hashes.m_rast = 0;
		}
#endif
	}

	void setAlphaToCoverage(Bool enable)
	{
		if(m_staticState.m_blend.m_alphaToCoverage != enable)
		{
			m_staticState.m_blend.m_alphaToCoverage = enable;
			m_hashes.m_blend = 0;
		}
	}

	void setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA)
	{
		auto& rt = m_staticState.m_blend.m_colorRts[attachment];
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
		auto& rt = m_staticState.m_blend.m_colorRts[attachment];
		if(rt.m_funcRgb != funcRgb || rt.m_funcA != funcA)
		{
			rt.m_funcRgb = funcRgb;
			rt.m_funcA = funcA;
			m_hashes.m_blend = 0;
		}
	}

	void beginRenderPass(ConstWeakArray<Format> colorFormats, Format depthStencilFormat, UVec2 rtsSize
#if ANKI_GR_BACKEND_VULKAN
						 ,
						 Bool rendersToSwapchain
#endif
	)
	{
		m_staticState.m_misc.m_colorRtFormats.fill(Format::kNone);
		m_staticState.m_misc.m_colorRtMask.unsetAll();
		for(U8 i = 0; i < colorFormats.getSize(); ++i)
		{
			ANKI_ASSERT(colorFormats[i] != Format::kNone);
			m_staticState.m_misc.m_colorRtFormats[i] = colorFormats[i];
			m_staticState.m_misc.m_colorRtMask.set(i);
		}
		m_staticState.m_misc.m_depthStencilFormat = depthStencilFormat;

		m_hashes.m_misc = 0; // Always mark it dirty because calling beginRenderPass is a rare occurance and we want to avoid extra checks

		if(m_rtsSize != rtsSize
#if ANKI_GR_BACKEND_VULKAN
		   || m_staticState.m_misc.m_rendersToSwapchain != rendersToSwapchain
#endif
		)
		{
			m_rtsSize = rtsSize;

			// Re-set the viewport and scissor because they depend on the size of the render targets
			m_dynState.m_scissorDirty = true;
			m_dynState.m_viewportDirty = true;
		}

#if ANKI_GR_BACKEND_VULKAN
		m_staticState.m_misc.m_rendersToSwapchain = rendersToSwapchain;
#endif
	}

	void bindShaderProgram(ShaderProgram* prog)
	{
		ANKI_ASSERT(prog);
		if(prog != m_staticState.m_shaderProg)
		{
			const ShaderReflection& refl = prog->getReflection();
			if(m_staticState.m_vert.m_activeAttribs != refl.m_vertex.m_vertexAttributeMask)
			{
				m_staticState.m_vert.m_activeAttribs = refl.m_vertex.m_vertexAttributeMask;
				m_hashes.m_vert = 0;
			}

#if ANKI_GR_BACKEND_VULKAN
			if(!!(prog->getShaderTypes() & ShaderTypeBit::kVertex) && refl.m_vertex.m_vertexAttributeMask.getSetBitCount())
			{
				if(m_staticState.m_shaderProg)
				{
					const Bool semanticsChanged = m_staticState.m_shaderProg->getReflection().m_vertex.m_vkVertexAttributeLocations
												  != refl.m_vertex.m_vkVertexAttributeLocations;
					if(semanticsChanged)
					{
						m_hashes.m_vert = 0;
					}
				}

				for(VertexAttributeSemantic s : EnumIterable<VertexAttributeSemantic>())
				{
					m_staticState.m_vert.m_attribs[s].m_semanticToVertexAttributeLocation = refl.m_vertex.m_vkVertexAttributeLocations[s];
				}
			}
#endif

			if(m_staticState.m_misc.m_colorRtMask != refl.m_fragment.m_colorAttachmentWritemask)
			{
				m_staticState.m_misc.m_colorRtMask = refl.m_fragment.m_colorAttachmentWritemask;
				m_hashes.m_misc = 0;
			}

			m_staticState.m_shaderProg = prog;
			m_hashes.m_shaderProg = 0;
		}
	}

	void setPrimitiveTopology(PrimitiveTopology topology)
	{
		if(m_staticState.m_ia.m_topology != topology)
		{
			m_staticState.m_ia.m_topology = topology;
			m_hashes.m_ia = 0;
#if ANKI_GR_BACKEND_DIRECT3D
			m_dynState.m_topologyDirty = true;
#endif
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

			// Scissor needs to be re-adjusted if it's in its default size
			m_dynState.m_scissorDirty = true;
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

	void setLineWidth(F32 width)
	{
#if ANKI_GR_BACKEND_VULKAN
		if(m_dynState.m_lineWidth != width)
		{
			m_dynState.m_lineWidth = width;
			m_dynState.m_lineWidthDirty = true;
		}
#else
		ANKI_ASSERT(width == 1.0f && "DX only accepts 1.0 line width");
#endif
	}

#if ANKI_GR_BACKEND_VULKAN
	void setEnablePipelineStatistics(Bool enable)
	{
		m_staticState.m_misc.m_pipelineStatisticsEnabled = enable;
	}
#endif

	void setPrimitiveRestart(Bool enable)
	{
		if(m_staticState.m_ia.m_primitiveRestartEnabled != enable)
		{
			m_staticState.m_ia.m_primitiveRestartEnabled = enable;
			m_hashes.m_ia = 0;
		}
	}

	void setColorChannelWriteMask(U32 attachment, ColorBit mask)
	{
		if(m_staticState.m_blend.m_colorRts[attachment].m_channelWriteMask != mask)
		{
			m_staticState.m_blend.m_colorRts[attachment].m_channelWriteMask = mask;
			m_hashes.m_blend = 0;
		}
	}

	ShaderProgram& getShaderProgram()
	{
		ANKI_ASSERT(m_staticState.m_shaderProg);
		return *m_staticState.m_shaderProg;
	}

private:
	ANKI_BEGIN_PACKED_STRUCT
	class StaticState
	{
	public:
		class Vertex
		{
		public:
			class Binding
			{
			public:
#if ANKI_GR_BACKEND_VULKAN
				U32 m_stride;
#endif
				VertexStepRate m_stepRate;

				Binding()
				{
					// No init for opt
				}
			};

			class Attribute
			{
			public:
				U32 m_relativeOffset;
				Format m_fmt;
				U32 m_binding;
#if ANKI_GR_BACKEND_VULKAN
				U8 m_semanticToVertexAttributeLocation;
#endif

				Attribute()
				{
					// No init for opt
				}
			};

			Array<Binding, U32(VertexAttributeSemantic::kCount)> m_bindings;
			Array<Attribute, U32(VertexAttributeSemantic::kCount)> m_attribs;
			BitSet<U32(VertexAttributeSemantic::kCount)> m_activeAttribs = false;

			BitSet<U32(VertexAttributeSemantic::kCount)> m_bindingsSetMask = false;
			BitSet<U32(VertexAttributeSemantic::kCount)> m_attribsSetMask = false;
		} m_vert;

		class InputAssembler
		{
		public:
			PrimitiveTopology m_topology = PrimitiveTopology::kTriangles;
			Bool m_primitiveRestartEnabled = false;
		} m_ia;

		class Rast
		{
		public:
			FillMode m_fillMode = FillMode::kSolid;
			FaceSelectionBit m_cullMode = FaceSelectionBit::kBack;
#if ANKI_GR_BACKEND_VULKAN
			Bool m_depthBiasEnabled = false;
#else
			F32 m_depthBias = 0.0f;
			F32 m_depthBiasClamp = 0.0f;
			F32 m_slopeScaledDepthBias = 0.0f;
#endif
		} m_rast;

		class
		{
		public:
			class StencilFace
			{
			public:
				StencilOperation m_fail = StencilOperation::kKeep;
				StencilOperation m_stencilPassDepthFail = StencilOperation::kKeep;
				StencilOperation m_stencilPassDepthPass = StencilOperation::kKeep;
				CompareOperation m_compare = CompareOperation::kAlways;
#if ANKI_GR_BACKEND_DIRECT3D
				U32 m_compareMask = 0x5A5A5A5A; ///< Use a stupid number to initialize.
				U32 m_writeMask = 0x5A5A5A5A; ///< Use a stupid number to initialize.
#endif
			};

			Array<StencilFace, 2> m_face;
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
				ColorBit m_channelWriteMask = ColorBit::kAll;
				BlendFactor m_srcRgb = BlendFactor::kOne;
				BlendFactor m_dstRgb = BlendFactor::kZero;
				BlendFactor m_srcA = BlendFactor::kOne;
				BlendFactor m_dstA = BlendFactor::kZero;
				BlendOperation m_funcRgb = BlendOperation::kAdd;
				BlendOperation m_funcA = BlendOperation::kAdd;
			};

			Array<Rt, kMaxColorRenderTargets> m_colorRts;
			Bool m_alphaToCoverage = false;
		} m_blend;

		class Misc
		{
		public:
			Array<Format, kMaxColorRenderTargets> m_colorRtFormats = {};
			Format m_depthStencilFormat = Format::kNone;
			BitSet<kMaxColorRenderTargets> m_colorRtMask = {false};

#if ANKI_GR_BACKEND_VULKAN
			Bool m_rendersToSwapchain = false;
			Bool m_pipelineStatisticsEnabled = false;
#endif
		} m_misc;

		ShaderProgram* m_shaderProg = nullptr;
	} m_staticState;
	ANKI_END_PACKED_STRUCT

	class DynamicState
	{
	public:
		class StencilFace
		{
		public:
#if ANKI_GR_BACKEND_VULKAN
			U32 m_compareMask = 0x5A5A5A5A; ///< Use a stupid number to initialize.
			U32 m_writeMask = 0x5A5A5A5A; ///< Use a stupid number to initialize.
#endif
			U32 m_ref = 0x5A5A5A5A; ///< Use a stupid number to initialize.
		};

		Array<StencilFace, 2> m_stencilFaces;

		Array<U32, 4> m_viewport = {};
		Array<U32, 4> m_scissor = {0, 0, kMaxU32, kMaxU32};

#if ANKI_GR_BACKEND_VULKAN
		F32 m_depthBiasConstantFactor = 0.0f;
		F32 m_depthBiasClamp = 0.0f;
		F32 m_depthBiasSlopeFactor = 0.0f;

		F32 m_lineWidth = 1.0f;
#endif

#if ANKI_GR_BACKEND_VULKAN
		Bool m_stencilCompareMaskDirty : 1 = true;
		Bool m_stencilWriteMaskDirty : 1 = true;
		Bool m_depthBiasDirty : 1 = true;
		Bool m_lineWidthDirty : 1 = true;
#else
		Bool m_topologyDirty : 1 = true;
#endif
		Bool m_stencilRefDirty : 1 = true;
		Bool m_viewportDirty : 1 = true;
		Bool m_scissorDirty : 1 = true;
	} m_dynState;

	class Hashes
	{
	public:
		U64 m_vert = 0;
		U64 m_rast = 0;
		U64 m_ia = 0;
		U64 m_depthStencil = 0;
		U64 m_blend = 0;
		U64 m_misc = 0;
		U64 m_shaderProg = 0;
	} m_hashes;

	U64 m_globalHash = 0;

	UVec2 m_rtsSize = UVec2(0u);

	Bool updateHashes();
};
/// @}

} // end namespace anki

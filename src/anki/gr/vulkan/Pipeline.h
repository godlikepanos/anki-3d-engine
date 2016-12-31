// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/DescriptorSet.h>
#include <anki/gr/ShaderProgram.h>
#include <anki/gr/vulkan/ShaderProgramImpl.h>
#include <anki/util/HashMap.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

class VertexBufferBinding
{
public:
	PtrSize m_stride = MAX_PTR_SIZE; ///< Vertex stride.
	VertexStepRate m_stepRate = VertexStepRate::VERTEX;

	Bool operator==(const VertexBufferBinding& b) const
	{
		return m_stride == b.m_stride && m_stepRate == b.m_stepRate;
	}

	Bool operator!=(const VertexBufferBinding& b) const
	{
		return !(*this == b);
	}
};

class VertexAttributeBinding
{
public:
	PixelFormat m_format;
	PtrSize m_offset = 0;
	U8 m_binding = 0;

	Bool operator==(const VertexAttributeBinding& b) const
	{
		return m_format == b.m_format && m_offset == b.m_offset && m_binding == b.m_binding;
	}

	Bool operator!=(const VertexAttributeBinding& b) const
	{
		return !(*this == b);
	}
};

class VertexStateInfo
{
public:
	U8 m_bindingCount = 0;
	Array<VertexBufferBinding, MAX_VERTEX_ATTRIBUTES> m_bindings;
	U8 m_attributeCount = 0;
	Array<VertexAttributeBinding, MAX_VERTEX_ATTRIBUTES> m_attributes;
};

class InputAssemblerStateInfo
{
public:
	PrimitiveTopology m_topology = PrimitiveTopology::TRIANGLES;
	Bool8 m_primitiveRestartEnabled = false;
};

class TessellationStateInfo
{
public:
	U32 m_patchControlPointCount = 3;
};

class ViewportStateInfo
{
public:
	Bool8 m_scissorEnabled = false;
};

class RasterizerStateInfo
{
public:
	FillMode m_fillMode = FillMode::SOLID;
	FaceSelectionBit m_cullMode = FaceSelectionBit::BACK;
	Bool8 m_depthBiasEnabled = false;
};

class DepthStateInfo
{
public:
	Bool8 m_depthWriteEnabled = true;
	CompareOperation m_depthCompareFunction = CompareOperation::LESS;
};

class StencilStateInfo
{
public:
	class S
	{
	public:
		StencilOperation m_stencilFailOperation = StencilOperation::KEEP;
		StencilOperation m_stencilPassDepthFailOperation = StencilOperation::KEEP;
		StencilOperation m_stencilPassDepthPassOperation = StencilOperation::KEEP;
		CompareOperation m_compareFunction = CompareOperation::ALWAYS;
	} m_face[2];
};

class ColorAttachmentStateInfo
{
public:
	BlendFactor m_srcBlendMethod = BlendFactor::ONE;
	BlendFactor m_dstBlendMethod = BlendFactor::ZERO;
	BlendOperation m_blendFunction = BlendOperation::ADD;
	ColorBit m_channelWriteMask = ColorBit::ALL;
};

class ColorStateInfo
{
public:
	Bool8 m_alphaToCoverageEnabled = false;
	U8 m_attachmentCount = 0;
	Array<ColorAttachmentStateInfo, MAX_COLOR_ATTACHMENTS> m_attachments;
};

class PipelineInfoState
{
public:
	PipelineInfoState()
	{
		// Do a special construction. The state will be hashed and the padding may contain garbage. With this trick
		// zero the padding
		memset(this, 0, sizeof(*this));

#define ANKI_CONSTRUCT_AND_ZERO_PADDING(memb_) new(&memb_) decltype(memb_)()

		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_vertex);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_inputAssembler);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_tessellation);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_viewport);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_rasterizer);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_depth);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_stencil);
		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_color);

#undef ANKI_CONSTRUCT_AND_ZERO_PADDING
	}

	VertexStateInfo m_vertex;
	InputAssemblerStateInfo m_inputAssembler;
	TessellationStateInfo m_tessellation;
	ViewportStateInfo m_viewport;
	RasterizerStateInfo m_rasterizer;
	DepthStateInfo m_depth;
	StencilStateInfo m_stencil;
	ColorStateInfo m_color;
};

/// Track changes in the static state.
class PipelineStateTracker
{
public:
	void bindVertexBuffer(U32 binding, PtrSize stride)
	{
		VertexBufferBinding b;
		b.m_stride = stride;
		b.m_stepRate = VertexStepRate::VERTEX;
		if(m_state.m_vertex.m_bindings[binding] != b)
		{
			m_state.m_vertex.m_bindings[binding].m_stride = stride;
			m_dirty.m_vertBindings.set(binding);
		}
	}

	void setVertexAttribute(U32 location, U32 buffBinding, const PixelFormat& fmt, PtrSize relativeOffset)
	{
		VertexAttributeBinding b;
		b.m_binding = buffBinding;
		b.m_format = fmt;
		b.m_offset = relativeOffset;
		if(m_state.m_vertex.m_attributes[location] != b)
		{
			m_state.m_vertex.m_attributes[location] = b;
			m_dirty.m_attribs.set(location);
		}
	}

	void setPrimitiveRestart(Bool enable)
	{
		if(m_state.m_inputAssembler.m_primitiveRestartEnabled != enable)
		{
			m_state.m_inputAssembler.m_primitiveRestartEnabled = enable;
			m_dirty.m_ia = true;
		}
	}

	void setFillMode(FillMode mode)
	{
		if(m_state.m_rasterizer.m_fillMode != mode)
		{
			m_state.m_rasterizer.m_fillMode = mode;
			m_dirty.m_raster = true;
		}
	}

	void setCullMode(FaceSelectionBit mode)
	{
		if(m_state.m_rasterizer.m_cullMode != mode)
		{
			m_state.m_rasterizer.m_cullMode = mode;
			m_dirty.m_raster = true;
		}
	}

	void setPolygonOffset(F32 factor, F32 units)
	{
		const Bool enable = factor != 0.0 && units != 0.0;
		if(m_state.m_rasterizer.m_depthBiasEnabled != enable)
		{
			m_state.m_rasterizer.m_depthBiasEnabled = enable;
			m_dirty.m_raster = true;
		}
	}

	void setStencilOperations(FaceSelectionBit face,
		StencilOperation stencilFail,
		StencilOperation stencilPassDepthFail,
		StencilOperation stencilPassDepthPass)
	{
		if(!!(face & FaceSelectionBit::FRONT)
			&& (m_state.m_stencil.m_face[0].m_stencilFailOperation != stencilFail
				   || m_state.m_stencil.m_face[0].m_stencilPassDepthFailOperation != stencilPassDepthFail
				   || m_state.m_stencil.m_face[0].m_stencilPassDepthPassOperation != stencilPassDepthPass))
		{
			m_state.m_stencil.m_face[0].m_stencilFailOperation = stencilFail;
			m_state.m_stencil.m_face[0].m_stencilPassDepthFailOperation = stencilPassDepthFail;
			m_state.m_stencil.m_face[0].m_stencilPassDepthPassOperation = stencilPassDepthPass;
			m_dirty.m_stencil = true;
		}

		if(!!(face & FaceSelectionBit::BACK)
			&& (m_state.m_stencil.m_face[1].m_stencilFailOperation != stencilFail
				   || m_state.m_stencil.m_face[1].m_stencilPassDepthFailOperation != stencilPassDepthFail
				   || m_state.m_stencil.m_face[1].m_stencilPassDepthPassOperation != stencilPassDepthPass))
		{
			m_state.m_stencil.m_face[1].m_stencilFailOperation = stencilFail;
			m_state.m_stencil.m_face[1].m_stencilPassDepthFailOperation = stencilPassDepthFail;
			m_state.m_stencil.m_face[1].m_stencilPassDepthPassOperation = stencilPassDepthPass;
			m_dirty.m_stencil = true;
		}
	}

	void setStencilCompareOperation(FaceSelectionBit face, CompareOperation comp)
	{
		if(!!(face & FaceSelectionBit::FRONT) && m_state.m_stencil.m_face[0].m_compareFunction != comp)
		{
			m_state.m_stencil.m_face[0].m_compareFunction = comp;
			m_dirty.m_stencil = true;
		}

		if(!!(face & FaceSelectionBit::BACK) && m_state.m_stencil.m_face[1].m_compareFunction != comp)
		{
			m_state.m_stencil.m_face[1].m_compareFunction = comp;
			m_dirty.m_stencil = true;
		}
	}

	void setStencilCompareMask(FaceSelectionBit face, U32 mask);

	void setStencilWriteMask(FaceSelectionBit face, U32 mask);

	void setStencilReference(FaceSelectionBit face, U32 ref);

	void setDepthWrite(Bool enable);

	void setDepthCompareOperation(CompareOperation op);

	void setAlphaToCoverage(Bool enable);

	void setColorChannelWriteMask(U32 attachment, ColorBit mask);

	void setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA);

	void bindShaderProgram(const ShaderProgramPtr& prog)
	{
		const ShaderProgramImpl& impl = *prog->m_impl;
		m_shaderColorAttachmentWritemask = impl.m_colorAttachmentWritemask;
		m_shaderAttributeMask = impl.m_attributeMask;
	}

	void beginRenderPass(FramebufferPtr fb);

private:
	PipelineInfoState m_state;

	class Hashes
	{
	public:
		U64 m_vertex = 0;
		BitSet<MAX_VERTEX_ATTRIBUTES, U8> m_shaderAttributeMaskWhenHashed = {false};

		U64 m_ia = 0;
		U64 m_raster = 0;
	} m_hashes;

	class DirtyBits
	{
	public:
		BitSet<MAX_VERTEX_ATTRIBUTES, U8> m_attribs = {true};
		BitSet<MAX_VERTEX_ATTRIBUTES, U8> m_vertBindings = {true};

		Bool8 m_ia = true;
		Bool8 m_raster = true;
		Bool8 m_stencil = true;
	} m_dirty;

	// Vertex
	BitSet<MAX_VERTEX_ATTRIBUTES, U8> m_shaderAttributeMask = {false};

	// Color & blend
	BitSet<MAX_COLOR_ATTACHMENTS, U8> m_shaderColorAttachmentWritemask = {false};
};

/// Small wrapper on top of the pipeline.
class Pipeline
{
	friend class PipelineFactory;

public:
	VkPipeline getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

private:
	VkPipeline m_handle = VK_NULL_HANDLE;
};

/// Given some state it creates/hashes pipelines.
class PipelineFactory
{
public:
	void init(GrAllocator<U8> alloc, VkDevice dev, VkPipelineCache pplineCache);

	void destroy();

	/// @note Not thread-safe.
	void newPipeline(PipelineStateTracker& state, Pipeline& ppline);

private:
	class Pipeline;
	class Hasher;

	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	VkPipelineCache m_pplineCache = VK_NULL_HANDLE;

	HashMap<U64, Pipeline, Hasher> m_pplines;
};
/// @}

} // end namespace anki

// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/DescriptorSet.h>
#include <AnKi/Gr/ShaderProgram.h>
#include <AnKi/Gr/Vulkan/ShaderProgramImpl.h>
#include <AnKi/Gr/Framebuffer.h>
#include <AnKi/Gr/Vulkan/FramebufferImpl.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

/// @addtogroup vulkan
/// @{

class VertexBufferBindingPipelineState
{
public:
	U32 m_stride = kMaxU32; ///< Vertex stride.
	VertexStepRate m_stepRate = VertexStepRate::kVertex;
	Array<U8, 3> m_padding = {};

	Bool operator==(const VertexBufferBindingPipelineState& b) const
	{
		return m_stride == b.m_stride && m_stepRate == b.m_stepRate;
	}

	Bool operator!=(const VertexBufferBindingPipelineState& b) const
	{
		return !(*this == b);
	}
};
static_assert(sizeof(VertexBufferBindingPipelineState) == 2 * sizeof(U32), "Packed because it will be hashed");

class VertexAttributeBindingPipelineState
{
public:
	PtrSize m_offset = 0;
	Format m_format = Format::kNone;
	U8 m_binding = 0;
	Array<U8, 3> m_padding = {};

	Bool operator==(const VertexAttributeBindingPipelineState& b) const
	{
		return m_format == b.m_format && m_offset == b.m_offset && m_binding == b.m_binding;
	}

	Bool operator!=(const VertexAttributeBindingPipelineState& b) const
	{
		return !(*this == b);
	}
};
static_assert(sizeof(VertexAttributeBindingPipelineState) == 2 * sizeof(PtrSize), "Packed because it will be hashed");

class VertexPipelineState
{
public:
	Array<VertexBufferBindingPipelineState, kMaxVertexAttributes> m_bindings;
	Array<VertexAttributeBindingPipelineState, kMaxVertexAttributes> m_attributes;
};
static_assert(sizeof(VertexPipelineState)
				  == sizeof(VertexBufferBindingPipelineState) * kMaxVertexAttributes
						 + sizeof(VertexAttributeBindingPipelineState) * kMaxVertexAttributes,
			  "Packed because it will be hashed");

class InputAssemblerPipelineState
{
public:
	PrimitiveTopology m_topology = PrimitiveTopology::kTriangles;
	Bool m_primitiveRestartEnabled = false;
};
static_assert(sizeof(InputAssemblerPipelineState) == sizeof(U8) * 2, "Packed because it will be hashed");

class RasterizerPipelineState
{
public:
	FillMode m_fillMode = FillMode::kSolid;
	FaceSelectionBit m_cullMode = FaceSelectionBit::kBack;
	RasterizationOrder m_rasterizationOrder = RasterizationOrder::kOrdered;
	U8 m_padding = 0;
	F32 m_depthBiasConstantFactor = 0.0f;
	F32 m_depthBiasSlopeFactor = 0.0f;
};
static_assert(sizeof(RasterizerPipelineState) == sizeof(U32) * 3, "Packed because it will be hashed");

class DepthPipelineState
{
public:
	Bool m_depthWriteEnabled = true;
	CompareOperation m_depthCompareFunction = CompareOperation::kLess;
};
static_assert(sizeof(DepthPipelineState) == sizeof(U8) * 2, "Packed because it will be hashed");

class StencilPipelineState
{
public:
	class S
	{
	public:
		StencilOperation m_stencilFailOperation = StencilOperation::kKeep;
		StencilOperation m_stencilPassDepthFailOperation = StencilOperation::kKeep;
		StencilOperation m_stencilPassDepthPassOperation = StencilOperation::kKeep;
		CompareOperation m_compareFunction = CompareOperation::kAlways;
	};

	Array<S, 2> m_face;
};
static_assert(sizeof(StencilPipelineState) == sizeof(U32) * 2, "Packed because it will be hashed");

class ColorAttachmentState
{
public:
	BlendFactor m_srcBlendFactorRgb = BlendFactor::kOne;
	BlendFactor m_srcBlendFactorA = BlendFactor::kOne;
	BlendFactor m_dstBlendFactorRgb = BlendFactor::kZero;
	BlendFactor m_dstBlendFactorA = BlendFactor::kZero;
	BlendOperation m_blendFunctionRgb = BlendOperation::kAdd;
	BlendOperation m_blendFunctionA = BlendOperation::kAdd;
	ColorBit m_channelWriteMask = ColorBit::kAll;
};
static_assert(sizeof(ColorAttachmentState) == sizeof(U8) * 7, "Packed because it will be hashed");

class ColorPipelineState
{
public:
	Bool m_alphaToCoverageEnabled = false;
	Array<ColorAttachmentState, kMaxColorRenderTargets> m_attachments;
};
static_assert(sizeof(ColorPipelineState) == sizeof(ColorAttachmentState) * kMaxColorRenderTargets + sizeof(U8),
			  "Packed because it will be hashed");

class AllPipelineState
{
public:
	const ShaderProgramImpl* m_prog = nullptr;
	VkRenderPass m_rpass = VK_NULL_HANDLE;

	VertexPipelineState m_vertex;
	InputAssemblerPipelineState m_inputAssembler;
	RasterizerPipelineState m_rasterizer;
	DepthPipelineState m_depth;
	StencilPipelineState m_stencil;
	ColorPipelineState m_color;

	void reset()
	{
		::new(this) AllPipelineState();
	}
};

/// Track changes in the static state.
class PipelineStateTracker
{
	friend class PipelineFactory;

public:
	PipelineStateTracker()
	{
	}

	PipelineStateTracker(const PipelineStateTracker&) = delete; // Non-copyable

	PipelineStateTracker& operator=(const PipelineStateTracker&) = delete; // Non-copyable

	void bindVertexBuffer(U32 binding, PtrSize stride, VertexStepRate stepRate)
	{
		VertexBufferBindingPipelineState b;
		b.m_stride = U32(stride);
		b.m_stepRate = stepRate;
		if(m_state.m_vertex.m_bindings[binding] != b)
		{
			m_state.m_vertex.m_bindings[binding] = b;
			m_dirty.m_vertBindings.set(binding);
		}
		m_set.m_vertBindings.set(binding);
	}

	void setVertexAttribute(U32 location, U32 buffBinding, const Format fmt, PtrSize relativeOffset)
	{
		VertexAttributeBindingPipelineState b;
		b.m_binding = U8(buffBinding);
		b.m_format = fmt;
		b.m_offset = relativeOffset;
		if(m_state.m_vertex.m_attributes[location] != b)
		{
			m_state.m_vertex.m_attributes[location] = b;
			m_dirty.m_attribs.set(location);
		}
		m_set.m_attribs.set(location);
	}

	void setPrimitiveRestart(Bool enable)
	{
		if(m_state.m_inputAssembler.m_primitiveRestartEnabled != enable)
		{
			m_state.m_inputAssembler.m_primitiveRestartEnabled = enable;
			m_dirty.m_inputAssembler = true;
		}
	}

	void setFillMode(FillMode mode)
	{
		if(m_state.m_rasterizer.m_fillMode != mode)
		{
			m_state.m_rasterizer.m_fillMode = mode;
			m_dirty.m_rasterizer = true;
		}
	}

	void setCullMode(FaceSelectionBit mode)
	{
		if(m_state.m_rasterizer.m_cullMode != mode)
		{
			m_state.m_rasterizer.m_cullMode = mode;
			m_dirty.m_rasterizer = true;
		}
	}

	void setPolygonOffset(F32 factor, F32 units)
	{
		if(m_state.m_rasterizer.m_depthBiasConstantFactor != factor
		   || m_state.m_rasterizer.m_depthBiasSlopeFactor != units)
		{
			m_state.m_rasterizer.m_depthBiasConstantFactor = factor;
			m_state.m_rasterizer.m_depthBiasSlopeFactor = units;
			m_dirty.m_rasterizer = true;
		}
	}

	void setRasterizationOrder(RasterizationOrder order)
	{
		if(m_state.m_rasterizer.m_rasterizationOrder != order)
		{
			m_state.m_rasterizer.m_rasterizationOrder = order;
			m_dirty.m_rasterizer = true;
		}
	}

	void setStencilOperations(FaceSelectionBit face, StencilOperation stencilFail,
							  StencilOperation stencilPassDepthFail, StencilOperation stencilPassDepthPass)
	{
		if(!!(face & FaceSelectionBit::kFront)
		   && (m_state.m_stencil.m_face[0].m_stencilFailOperation != stencilFail
			   || m_state.m_stencil.m_face[0].m_stencilPassDepthFailOperation != stencilPassDepthFail
			   || m_state.m_stencil.m_face[0].m_stencilPassDepthPassOperation != stencilPassDepthPass))
		{
			m_state.m_stencil.m_face[0].m_stencilFailOperation = stencilFail;
			m_state.m_stencil.m_face[0].m_stencilPassDepthFailOperation = stencilPassDepthFail;
			m_state.m_stencil.m_face[0].m_stencilPassDepthPassOperation = stencilPassDepthPass;
			m_dirty.m_stencil = true;
		}

		if(!!(face & FaceSelectionBit::kBack)
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
		if(!!(face & FaceSelectionBit::kFront) && m_state.m_stencil.m_face[0].m_compareFunction != comp)
		{
			m_state.m_stencil.m_face[0].m_compareFunction = comp;
			m_dirty.m_stencil = true;
		}

		if(!!(face & FaceSelectionBit::kBack) && m_state.m_stencil.m_face[1].m_compareFunction != comp)
		{
			m_state.m_stencil.m_face[1].m_compareFunction = comp;
			m_dirty.m_stencil = true;
		}
	}

	void setDepthWrite(Bool enable)
	{
		if(m_state.m_depth.m_depthWriteEnabled != enable)
		{
			m_state.m_depth.m_depthWriteEnabled = enable;
			m_dirty.m_depth = true;
		}
	}

	void setDepthCompareOperation(CompareOperation op)
	{
		if(m_state.m_depth.m_depthCompareFunction != op)
		{
			m_state.m_depth.m_depthCompareFunction = op;
			m_dirty.m_depth = true;
		}
	}

	void setAlphaToCoverage(Bool enable)
	{
		if(m_state.m_color.m_alphaToCoverageEnabled != enable)
		{
			m_state.m_color.m_alphaToCoverageEnabled = enable;
			m_dirty.m_color = true;
		}
	}

	void setColorChannelWriteMask(U32 attachment, ColorBit mask)
	{
		if(m_state.m_color.m_attachments[attachment].m_channelWriteMask != mask)
		{
			m_state.m_color.m_attachments[attachment].m_channelWriteMask = mask;
			m_dirty.m_colAttachments.set(attachment);
		}
	}

	void setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA)
	{
		ColorAttachmentState& c = m_state.m_color.m_attachments[attachment];
		if(c.m_srcBlendFactorRgb != srcRgb || c.m_dstBlendFactorRgb != dstRgb || c.m_srcBlendFactorA != srcA
		   || c.m_dstBlendFactorA != dstA)
		{
			c.m_srcBlendFactorRgb = srcRgb;
			c.m_dstBlendFactorRgb = dstRgb;
			c.m_srcBlendFactorA = srcA;
			c.m_dstBlendFactorA = dstA;
			m_dirty.m_colAttachments.set(attachment);
		}
	}

	void setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA)
	{
		ColorAttachmentState& c = m_state.m_color.m_attachments[attachment];
		if(c.m_blendFunctionRgb != funcRgb || c.m_blendFunctionA != funcA)
		{
			c.m_blendFunctionRgb = funcRgb;
			c.m_blendFunctionA = funcA;
			m_dirty.m_colAttachments.set(attachment);
		}
	}

	void bindShaderProgram(const ShaderProgramImpl* prog)
	{
		if(prog != m_state.m_prog)
		{
			m_shaderColorAttachmentWritemask = prog->getReflectionInfo().m_colorAttachmentWritemask;
			m_shaderAttributeMask = prog->getReflectionInfo().m_attributeMask;
			m_state.m_prog = prog;
			m_dirty.m_prog = true;
		}
	}

	void beginRenderPass(const FramebufferImpl* fb)
	{
		ANKI_ASSERT(m_state.m_rpass == VK_NULL_HANDLE);
		Bool d, s;
		fb->getAttachmentInfo(m_fbColorAttachmentMask, d, s);
		m_fbDepth = d;
		m_fbStencil = s;
		m_defaultFb = fb->hasPresentableTexture();

		m_state.m_rpass = fb->getCompatibleRenderPass();
		m_dirty.m_rpass = true;
	}

	void endRenderPass()
	{
		ANKI_ASSERT(m_state.m_rpass);
		m_state.m_rpass = VK_NULL_HANDLE;
	}

	void setPrimitiveTopology(PrimitiveTopology topology)
	{
		if(m_state.m_inputAssembler.m_topology != topology)
		{
			m_state.m_inputAssembler.m_topology = topology;
			m_dirty.m_inputAssembler = true;
		}
	}

	PrimitiveTopology getPrimitiveTopology() const
	{
		return m_state.m_inputAssembler.m_topology;
	}

	Bool getEnablePipelineStatistics() const
	{
		return m_pipelineStatisticsEnabled;
	}

	void setEnablePipelineStatistics(Bool enable)
	{
		m_pipelineStatisticsEnabled = enable;
	}

	/// Flush state
	void flush(U64& pipelineHash, Bool& stateDirty)
	{
		const Bool dirtyHashes = updateHashes();
		if(dirtyHashes)
		{
			updateSuperHash();
		}

		if(m_hashes.m_superHash != m_hashes.m_lastSuperHash)
		{
			m_hashes.m_lastSuperHash = m_hashes.m_superHash;
			stateDirty = true;
		}
		else
		{
			stateDirty = false;
		}

		pipelineHash = m_hashes.m_superHash;
		ANKI_ASSERT(pipelineHash);
	}

	/// Populate the internal pipeline create info structure.
	const VkGraphicsPipelineCreateInfo& updatePipelineCreateInfo();

	void reset();

private:
	AllPipelineState m_state;

	class DirtyBits
	{
	public:
		Bool m_prog : 1;
		Bool m_rpass : 1;
		Bool m_inputAssembler : 1;
		Bool m_rasterizer : 1;
		Bool m_depth : 1;
		Bool m_stencil : 1;
		Bool m_color : 1;

		// Vertex
		BitSet<kMaxVertexAttributes, U8> m_attribs = {true};
		BitSet<kMaxVertexAttributes, U8> m_vertBindings = {true};

		BitSet<kMaxColorRenderTargets, U8> m_colAttachments = {true};

		DirtyBits()
			: m_prog(true)
			, m_rpass(true)
			, m_inputAssembler(true)
			, m_rasterizer(true)
			, m_depth(true)
			, m_stencil(true)
			, m_color(true)
		{
		}
	} m_dirty;

	class SetBits
	{
	public:
		BitSet<kMaxVertexAttributes, U8> m_attribs = {false};
		BitSet<kMaxVertexAttributes, U8> m_vertBindings = {false};
	} m_set;

	// Shader info
	BitSet<kMaxVertexAttributes, U8> m_shaderAttributeMask = {false};
	BitSet<kMaxColorRenderTargets, U8> m_shaderColorAttachmentWritemask = {false};

	// Renderpass info
	Bool m_fbDepth = false;
	Bool m_fbStencil = false;
	Bool m_defaultFb = false;
	BitSet<kMaxColorRenderTargets, U8> m_fbColorAttachmentMask = {false};

	class Hashes
	{
	public:
		U64 m_prog;
		U64 m_rpass;
		Array<U64, kMaxVertexAttributes> m_vertexAttribs;
		U64 m_ia;
		U64 m_raster;
		U64 m_depth;
		U64 m_stencil;
		U64 m_color;
		Array<U64, kMaxColorRenderTargets> m_colAttachments;

		U64 m_superHash;
		U64 m_lastSuperHash;

		Hashes()
		{
			zeroMemory(*this);
		}
	} m_hashes;

	// Create info
	class CreateInfo
	{
	public:
		Array<VkVertexInputBindingDescription, kMaxVertexAttributes> m_vertBindings;
		Array<VkVertexInputAttributeDescription, kMaxVertexAttributes> m_attribs;
		VkPipelineVertexInputStateCreateInfo m_vert;
		VkPipelineInputAssemblyStateCreateInfo m_ia;
		VkPipelineViewportStateCreateInfo m_vp;
		VkPipelineTessellationStateCreateInfo m_tess;
		VkPipelineRasterizationStateCreateInfo m_rast;
		VkPipelineMultisampleStateCreateInfo m_ms;
		VkPipelineDepthStencilStateCreateInfo m_ds;
		Array<VkPipelineColorBlendAttachmentState, kMaxColorRenderTargets> m_colAttachments;
		VkPipelineColorBlendStateCreateInfo m_color;
		VkPipelineDynamicStateCreateInfo m_dyn;
		VkGraphicsPipelineCreateInfo m_ppline;
		VkPipelineRasterizationStateRasterizationOrderAMD m_rasterOrder;
	} m_ci;

	Bool m_pipelineStatisticsEnabled = false;

	Bool updateHashes();
	void updateSuperHash();
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
	VkPipeline m_handle ANKI_DEBUG_CODE(= 0);
};

/// Given some state it creates/hashes pipelines.
class PipelineFactory
{
public:
	PipelineFactory()
	{
	}

	~PipelineFactory()
	{
	}

	void init(HeapMemoryPool* pool, VkDevice dev, VkPipelineCache pplineCache
#if ANKI_PLATFORM_MOBILE
			  ,
			  Mutex* globalCreatePipelineMtx
#endif
	)
	{
		ANKI_ASSERT(pool);
		m_pool = pool;
		m_dev = dev;
		m_pplineCache = pplineCache;
#if ANKI_PLATFORM_MOBILE
		m_globalCreatePipelineMtx = globalCreatePipelineMtx;
#endif
	}

	void destroy();

	/// @note Thread-safe.
	void getOrCreatePipeline(PipelineStateTracker& state, Pipeline& ppline, Bool& stateDirty);

private:
	class PipelineInternal;
	class Hasher;

	HeapMemoryPool* m_pool = nullptr;
	VkDevice m_dev = VK_NULL_HANDLE;
	VkPipelineCache m_pplineCache = VK_NULL_HANDLE;

	HashMap<U64, PipelineInternal, Hasher> m_pplines;
	RWMutex m_pplinesMtx;
#if ANKI_PLATFORM_MOBILE
	Mutex* m_globalCreatePipelineMtx = nullptr;
#endif
};
/// @}

} // end namespace anki

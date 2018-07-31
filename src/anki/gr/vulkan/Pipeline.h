// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/DescriptorSet.h>
#include <anki/gr/ShaderProgram.h>
#include <anki/gr/vulkan/ShaderProgramImpl.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/vulkan/FramebufferImpl.h>
#include <anki/util/HashMap.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// @note Non copyable because that complicates the hashing.
class PPVertexBufferBinding : public NonCopyable
{
public:
	PtrSize m_stride = MAX_PTR_SIZE; ///< Vertex stride.
	VertexStepRate m_stepRate = VertexStepRate::VERTEX;

	Bool operator==(const PPVertexBufferBinding& b) const
	{
		return m_stride == b.m_stride && m_stepRate == b.m_stepRate;
	}

	Bool operator!=(const PPVertexBufferBinding& b) const
	{
		return !(*this == b);
	}
};

class PPVertexAttributeBinding : public NonCopyable
{
public:
	PtrSize m_offset = 0;
	Format m_format = Format::NONE;
	U8 m_binding = 0;

	Bool operator==(const PPVertexAttributeBinding& b) const
	{
		return m_format == b.m_format && m_offset == b.m_offset && m_binding == b.m_binding;
	}

	Bool operator!=(const PPVertexAttributeBinding& b) const
	{
		return !(*this == b);
	}
};

class PPVertexStateInfo : public NonCopyable
{
public:
	Array<PPVertexBufferBinding, MAX_VERTEX_ATTRIBUTES> m_bindings;
	Array<PPVertexAttributeBinding, MAX_VERTEX_ATTRIBUTES> m_attributes;
};

class PPInputAssemblerStateInfo : public NonCopyable
{
public:
	PrimitiveTopology m_topology = PrimitiveTopology::TRIANGLES;
	Bool8 m_primitiveRestartEnabled = false;
};

class PPTessellationStateInfo : public NonCopyable
{
public:
	U32 m_patchControlPointCount = 3;
};

class PPViewportStateInfo : public NonCopyable
{
public:
	Bool8 m_scissorEnabled = false;
};

class PPRasterizerStateInfo : public NonCopyable
{
public:
	FillMode m_fillMode = FillMode::SOLID;
	FaceSelectionBit m_cullMode = FaceSelectionBit::BACK;
	RasterizationOrder m_rasterizationOrder = RasterizationOrder::ORDERED;
	F32 m_depthBiasConstantFactor = 0.0;
	F32 m_depthBiasSlopeFactor = 0.0;
};

class PPDepthStateInfo : public NonCopyable
{
public:
	Bool8 m_depthWriteEnabled = true;
	CompareOperation m_depthCompareFunction = CompareOperation::LESS;
};

class PPStencilStateInfo : public NonCopyable
{
public:
	class S : public NonCopyable
	{
	public:
		StencilOperation m_stencilFailOperation = StencilOperation::KEEP;
		StencilOperation m_stencilPassDepthFailOperation = StencilOperation::KEEP;
		StencilOperation m_stencilPassDepthPassOperation = StencilOperation::KEEP;
		CompareOperation m_compareFunction = CompareOperation::ALWAYS;
	};

	Array<S, 2> m_face;
};

class PPColorAttachmentStateInfo : public NonCopyable
{
public:
	BlendFactor m_srcBlendFactorRgb = BlendFactor::ONE;
	BlendFactor m_srcBlendFactorA = BlendFactor::ONE;
	BlendFactor m_dstBlendFactorRgb = BlendFactor::ZERO;
	BlendFactor m_dstBlendFactorA = BlendFactor::ZERO;
	BlendOperation m_blendFunctionRgb = BlendOperation::ADD;
	BlendOperation m_blendFunctionA = BlendOperation::ADD;
	ColorBit m_channelWriteMask = ColorBit::ALL;
};

class PPColorStateInfo : public NonCopyable
{
public:
	Bool8 m_alphaToCoverageEnabled = false;
	Array<PPColorAttachmentStateInfo, MAX_COLOR_ATTACHMENTS> m_attachments;
};

class PipelineInfoState : public NonCopyable
{
public:
	PipelineInfoState()
	{
		reset();
	}

	ShaderProgramPtr m_prog;
	PPVertexStateInfo m_vertex;
	PPInputAssemblerStateInfo m_inputAssembler;
	PPTessellationStateInfo m_tessellation;
	PPViewportStateInfo m_viewport;
	PPRasterizerStateInfo m_rasterizer;
	PPDepthStateInfo m_depth;
	PPStencilStateInfo m_stencil;
	PPColorStateInfo m_color;

	void reset()
	{
		m_prog.reset(nullptr);

		// Do a special construction. The state will be hashed and the padding may contain garbage. With this trick
		// zero the padding
		memset(this, 0, sizeof(*this));

#define ANKI_CONSTRUCT_AND_ZERO_PADDING(memb_) new(&memb_) decltype(memb_)()

		ANKI_CONSTRUCT_AND_ZERO_PADDING(m_prog);
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
};

/// Track changes in the static state.
class PipelineStateTracker : public NonCopyable
{
	friend class PipelineFactory;

public:
	PipelineStateTracker()
	{
	}

	void bindVertexBuffer(U32 binding, PtrSize stride, VertexStepRate stepRate)
	{
		PPVertexBufferBinding b;
		b.m_stride = stride;
		b.m_stepRate = stepRate;
		if(m_state.m_vertex.m_bindings[binding] != b)
		{
			m_state.m_vertex.m_bindings[binding].m_stride = b.m_stride;
			m_state.m_vertex.m_bindings[binding].m_stepRate = b.m_stepRate;
			m_dirty.m_vertBindings.set(binding);
		}
		m_set.m_vertBindings.set(binding);
	}

	void setVertexAttribute(U32 location, U32 buffBinding, const Format fmt, PtrSize relativeOffset)
	{
		PPVertexAttributeBinding b;
		b.m_binding = buffBinding;
		b.m_format = fmt;
		b.m_offset = relativeOffset;
		if(m_state.m_vertex.m_attributes[location] != b)
		{
			m_state.m_vertex.m_attributes[location].m_binding = buffBinding;
			m_state.m_vertex.m_attributes[location].m_format = fmt;
			m_state.m_vertex.m_attributes[location].m_offset = relativeOffset;
			m_dirty.m_attribs.set(location);
		}
		m_set.m_attribs.set(location);
	}

	void setPrimitiveRestart(Bool enable)
	{
		if(m_state.m_inputAssembler.m_primitiveRestartEnabled != enable)
		{
			m_state.m_inputAssembler.m_primitiveRestartEnabled = enable;
			m_dirty.m_other |= DirtyBit::IA;
		}
	}

	void setFillMode(FillMode mode)
	{
		if(m_state.m_rasterizer.m_fillMode != mode)
		{
			m_state.m_rasterizer.m_fillMode = mode;
			m_dirty.m_other |= DirtyBit::RASTER;
		}
	}

	void setCullMode(FaceSelectionBit mode)
	{
		if(m_state.m_rasterizer.m_cullMode != mode)
		{
			m_state.m_rasterizer.m_cullMode = mode;
			m_dirty.m_other |= DirtyBit::RASTER;
		}
	}

	void setPolygonOffset(F32 factor, F32 units)
	{
		if(m_state.m_rasterizer.m_depthBiasConstantFactor != factor
			|| m_state.m_rasterizer.m_depthBiasSlopeFactor != units)
		{
			m_state.m_rasterizer.m_depthBiasConstantFactor = factor;
			m_state.m_rasterizer.m_depthBiasSlopeFactor = units;
			m_dirty.m_other |= DirtyBit::RASTER;
		}
	}

	void setRasterizationOrder(RasterizationOrder order)
	{
		if(m_state.m_rasterizer.m_rasterizationOrder != order)
		{
			m_state.m_rasterizer.m_rasterizationOrder = order;
			m_dirty.m_other |= DirtyBit::RASTER;
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
			m_dirty.m_other |= DirtyBit::STENCIL;
		}

		if(!!(face & FaceSelectionBit::BACK)
			&& (m_state.m_stencil.m_face[1].m_stencilFailOperation != stencilFail
				   || m_state.m_stencil.m_face[1].m_stencilPassDepthFailOperation != stencilPassDepthFail
				   || m_state.m_stencil.m_face[1].m_stencilPassDepthPassOperation != stencilPassDepthPass))
		{
			m_state.m_stencil.m_face[1].m_stencilFailOperation = stencilFail;
			m_state.m_stencil.m_face[1].m_stencilPassDepthFailOperation = stencilPassDepthFail;
			m_state.m_stencil.m_face[1].m_stencilPassDepthPassOperation = stencilPassDepthPass;
			m_dirty.m_other |= DirtyBit::STENCIL;
		}
	}

	void setStencilCompareOperation(FaceSelectionBit face, CompareOperation comp)
	{
		if(!!(face & FaceSelectionBit::FRONT) && m_state.m_stencil.m_face[0].m_compareFunction != comp)
		{
			m_state.m_stencil.m_face[0].m_compareFunction = comp;
			m_dirty.m_other |= DirtyBit::STENCIL;
		}

		if(!!(face & FaceSelectionBit::BACK) && m_state.m_stencil.m_face[1].m_compareFunction != comp)
		{
			m_state.m_stencil.m_face[1].m_compareFunction = comp;
			m_dirty.m_other |= DirtyBit::STENCIL;
		}
	}

	void setDepthWrite(Bool enable)
	{
		if(m_state.m_depth.m_depthWriteEnabled != enable)
		{
			m_state.m_depth.m_depthWriteEnabled = enable;
			m_dirty.m_other |= DirtyBit::DEPTH;
		}
	}

	void setDepthCompareOperation(CompareOperation op)
	{
		if(m_state.m_depth.m_depthCompareFunction != op)
		{
			m_state.m_depth.m_depthCompareFunction = op;
			m_dirty.m_other |= DirtyBit::DEPTH;
		}
	}

	void setAlphaToCoverage(Bool enable)
	{
		if(m_state.m_color.m_alphaToCoverageEnabled != enable)
		{
			m_state.m_color.m_alphaToCoverageEnabled = enable;
			m_dirty.m_other |= DirtyBit::COLOR;
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
		PPColorAttachmentStateInfo& c = m_state.m_color.m_attachments[attachment];
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
		PPColorAttachmentStateInfo& c = m_state.m_color.m_attachments[attachment];
		if(c.m_blendFunctionRgb != funcRgb || c.m_blendFunctionA != funcA)
		{
			c.m_blendFunctionRgb = funcRgb;
			c.m_blendFunctionA = funcA;
			m_dirty.m_colAttachments.set(attachment);
		}
	}

	void bindShaderProgram(const ShaderProgramPtr& prog)
	{
		if(prog != m_state.m_prog)
		{
			const ShaderProgramImpl& impl = static_cast<const ShaderProgramImpl&>(*prog);
			m_shaderColorAttachmentWritemask = impl.getReflectionInfo().m_colorAttachmentWritemask;
			m_shaderAttributeMask = impl.getReflectionInfo().m_attributeMask;
			m_state.m_prog = prog;
			m_dirty.m_other |= DirtyBit::PROG;
		}
	}

	void beginRenderPass(const FramebufferPtr& fb)
	{
		ANKI_ASSERT(m_rpass == VK_NULL_HANDLE);
		Bool d, s;
		const FramebufferImpl& fbimpl = static_cast<const FramebufferImpl&>(*fb);
		fbimpl.getAttachmentInfo(m_fbColorAttachmentMask, d, s);
		m_fbDepth = d;
		m_fbStencil = s;
		m_rpass = fbimpl.getCompatibleRenderPass();
		m_defaultFb = fbimpl.hasPresentableTexture();
		m_fb = fb;
	}

	void endRenderPass()
	{
		ANKI_ASSERT(m_rpass);
		m_rpass = VK_NULL_HANDLE;
		m_fb.reset(nullptr);
	}

	void setPrimitiveTopology(PrimitiveTopology topology)
	{
		if(m_state.m_inputAssembler.m_topology != topology)
		{
			m_state.m_inputAssembler.m_topology = topology;
			m_dirty.m_other |= DirtyBit::IA;
		}
	}

	/// Flush state
	void flush(U64& pipelineHash, Bool& stateDirty)
	{
		Bool dirtyHashes = updateHashes();
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

	FramebufferPtr getFb() const
	{
		ANKI_ASSERT(m_fb.isCreated());
		return m_fb;
	}

	void reset();

private:
	PipelineInfoState m_state;

	class Hashes
	{
	public:
		U64 m_prog;
		Array<U64, MAX_VERTEX_ATTRIBUTES> m_vertexAttribs;
		U64 m_ia;
		U64 m_raster;
		U64 m_depth;
		U64 m_stencil;
		U64 m_color;
		Array<U64, MAX_COLOR_ATTACHMENTS> m_colAttachments;

		U64 m_superHash;
		U64 m_lastSuperHash;

		Hashes()
		{
			zeroMemory(*this);
		}
	} m_hashes;

	enum class DirtyBit : U8
	{
		PROG = 1 << 0,
		IA = 1 << 1,
		RASTER = 1 << 2,
		STENCIL = 1 << 3,
		DEPTH = 1 << 4,
		COLOR = 1 << 5,

		NONE = 0,
		ALL = PROG | IA | RASTER | STENCIL | DEPTH | COLOR
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(DirtyBit, friend)

	class DirtyBits
	{
	public:
		DirtyBit m_other = DirtyBit::ALL;

		BitSet<MAX_VERTEX_ATTRIBUTES, U8> m_attribs = {true};
		BitSet<MAX_VERTEX_ATTRIBUTES, U8> m_vertBindings = {true};

		BitSet<MAX_COLOR_ATTACHMENTS, U8> m_colAttachments = {true};
	} m_dirty;

	class SetBits
	{
	public:
		BitSet<MAX_VERTEX_ATTRIBUTES, U8> m_attribs = {false};
		BitSet<MAX_VERTEX_ATTRIBUTES, U8> m_vertBindings = {false};
	} m_set;

	// Shader info
	BitSet<MAX_VERTEX_ATTRIBUTES, U8> m_shaderAttributeMask = {false};
	BitSet<MAX_COLOR_ATTACHMENTS, U8> m_shaderColorAttachmentWritemask = {false};

	// Renderpass info
	Bool8 m_fbDepth = false;
	Bool8 m_fbStencil = false;
	Bool8 m_defaultFb = false;
	BitSet<MAX_COLOR_ATTACHMENTS, U8> m_fbColorAttachmentMask = {false};
	VkRenderPass m_rpass = VK_NULL_HANDLE;
	FramebufferPtr m_fb; ///< Hold the reference.

	// Create info
	class CreateInfo
	{
	public:
		Array<VkVertexInputBindingDescription, MAX_VERTEX_ATTRIBUTES> m_vertBindings;
		Array<VkVertexInputAttributeDescription, MAX_VERTEX_ATTRIBUTES> m_attribs;
		VkPipelineVertexInputStateCreateInfo m_vert;
		VkPipelineInputAssemblyStateCreateInfo m_ia;
		VkPipelineViewportStateCreateInfo m_vp;
		VkPipelineTessellationStateCreateInfo m_tess;
		VkPipelineRasterizationStateCreateInfo m_rast;
		VkPipelineMultisampleStateCreateInfo m_ms;
		VkPipelineDepthStencilStateCreateInfo m_ds;
		Array<VkPipelineColorBlendAttachmentState, MAX_COLOR_ATTACHMENTS> m_colAttachments;
		VkPipelineColorBlendStateCreateInfo m_color;
		VkPipelineDynamicStateCreateInfo m_dyn;
		VkGraphicsPipelineCreateInfo m_ppline;
		VkPipelineRasterizationStateRasterizationOrderAMD m_rasterOrder;
	} m_ci;

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
	VkPipeline m_handle ANKI_DBG_NULLIFY;
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

	void init(GrAllocator<U8> alloc, VkDevice dev, VkPipelineCache pplineCache)
	{
		m_alloc = alloc;
		m_dev = dev;
		m_pplineCache = pplineCache;
	}

	void destroy();

	/// @note Thread-safe.
	void newPipeline(PipelineStateTracker& state, Pipeline& ppline, Bool& stateDirty);

private:
	class PipelineInternal;
	class Hasher;

	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	VkPipelineCache m_pplineCache = VK_NULL_HANDLE;

	HashMap<U64, PipelineInternal, Hasher> m_pplines;
	SpinLock m_pplinesMtx;
};
/// @}

} // end namespace anki

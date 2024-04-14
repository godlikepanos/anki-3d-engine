// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkPipeline.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Gr/Common/Functions.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

void PipelineStateTracker::reset()
{
	m_state.reset();
	m_hashes = {};
	m_dirty = {};
	m_set = {};
	m_shaderVertexAttributeMask.unsetAll();
	m_shaderColorAttachmentWritemask.unsetAll();
	m_fbDepth = false;
	m_fbStencil = false;
	m_rendersToSwapchain = false;
	m_fbColorAttachmentCount = 0;
}

Bool PipelineStateTracker::updateHashes()
{
	Bool stateDirty = false;

	// Prog
	if(m_dirty.m_prog)
	{
		m_dirty.m_prog = false;
		stateDirty = true;
		m_hashes.m_prog = m_state.m_prog->getUuid();
	}

	// Vertex
	if(m_dirty.m_attribs.getAnySet() || m_dirty.m_vertBindings.getAnySet())
	{
		for(VertexAttribute i : EnumIterable<VertexAttribute>())
		{
			if(m_shaderVertexAttributeMask.get(i))
			{
				ANKI_ASSERT(m_set.m_attribs.get(i) && "Forgot to set the attribute");

				Bool dirty = false;
				if(m_dirty.m_attribs.get(i))
				{
					m_dirty.m_attribs.unset(i);
					dirty = true;
				}

				const U binding = m_state.m_vertex.m_attributes[i].m_binding;
				ANKI_ASSERT(m_set.m_vertBindings.get(binding) && "Forgot to set a vertex binding");

				if(m_dirty.m_vertBindings.get(binding))
				{
					m_dirty.m_vertBindings.unset(binding);
					dirty = true;
				}

				if(dirty)
				{
					m_hashes.m_vertexAttribs[i] = computeHash(&m_state.m_vertex.m_attributes[i], sizeof(m_state.m_vertex.m_attributes[i]));
					m_hashes.m_vertexAttribs[i] =
						appendHash(&m_state.m_vertex.m_bindings[i], sizeof(m_state.m_vertex.m_bindings[i]), m_hashes.m_vertexAttribs[i]);

					stateDirty = true;
				}
			}
		}
	}

	// IA
	if(m_dirty.m_inputAssembler)
	{
		m_dirty.m_inputAssembler = false;
		stateDirty = true;
		m_hashes.m_ia = computeHash(&m_state.m_inputAssembler, sizeof(m_state.m_inputAssembler));
	}

	// Rasterizer
	if(m_dirty.m_rasterizer)
	{
		m_dirty.m_rasterizer = false;
		stateDirty = true;
		m_hashes.m_raster = computeHash(&m_state.m_rasterizer, sizeof(m_state.m_rasterizer));
	}

	// Depth
	if(m_fbDepth && m_dirty.m_depth)
	{
		m_dirty.m_depth = false;
		stateDirty = true;
		m_hashes.m_depth = computeHash(&m_state.m_depth, sizeof(m_state.m_depth));
	}

	// Stencil
	if(m_fbStencil && m_dirty.m_stencil)
	{
		m_dirty.m_stencil = false;
		stateDirty = true;
		m_hashes.m_stencil = computeHash(&m_state.m_stencil, sizeof(m_state.m_stencil));
	}

	// Color
	if(m_fbColorAttachmentCount)
	{
		ANKI_ASSERT((m_fbColorAttachmentCount == m_shaderColorAttachmentWritemask.getSetBitCount() || !m_shaderColorAttachmentWritemask)
					&& "Shader and FB should have same attachment mask or shader mask should be zero");

		if(m_dirty.m_color)
		{
			m_dirty.m_color = false;
			m_hashes.m_color = m_state.m_color.m_alphaToCoverageEnabled ? 1 : 2;
			stateDirty = true;
		}

		for(U32 i = 0; i < m_fbColorAttachmentCount; ++i)
		{
			if(m_dirty.m_colAttachments.get(i))
			{
				m_dirty.m_colAttachments.unset(i);
				m_hashes.m_colAttachments[i] = computeHash(&m_state.m_color.m_attachments[i], sizeof(m_state.m_color.m_attachments[i]));
				stateDirty = true;
			}
		}
	}

	// Rpass
	if(m_dirty.m_rpass)
	{
		m_dirty.m_rpass = false;
		stateDirty = true;
		m_hashes.m_rpass = computeHash(m_state.m_attachmentFormats.getBegin(), m_state.m_attachmentFormats.getSizeInBytes());
	}

	return stateDirty;
}

void PipelineStateTracker::updateSuperHash()
{
	Array<U64, sizeof(Hashes) / sizeof(U64)> buff;
	U count = 0;

	// Prog
	buff[count++] = m_hashes.m_prog;

	// Rpass
	buff[count++] = m_hashes.m_rpass;

	// Vertex
	if(!!m_shaderVertexAttributeMask)
	{
		for(VertexAttribute i : EnumIterable<VertexAttribute>())
		{
			if(m_shaderVertexAttributeMask.get(i))
			{
				buff[count++] = m_hashes.m_vertexAttribs[i];
			}
		}
	}

	// IA
	buff[count++] = m_hashes.m_ia;

	// Rasterizer
	buff[count++] = m_hashes.m_raster;

	// Depth
	if(m_fbDepth)
	{
		buff[count++] = m_hashes.m_depth;
	}

	// Stencil
	if(m_fbStencil)
	{
		buff[count++] = m_hashes.m_stencil;
	}

	// Color
	if(m_fbColorAttachmentCount)
	{
		buff[count++] = m_hashes.m_color;

		for(U i = 0; i < m_fbColorAttachmentCount; ++i)
		{
			buff[count++] = m_hashes.m_colAttachments[i];
		}
	}

	// Super hash
	m_hashes.m_superHash = computeHash(&buff[0], count * sizeof(buff[0]));
}

const VkGraphicsPipelineCreateInfo& PipelineStateTracker::updatePipelineCreateInfo()
{
	VkGraphicsPipelineCreateInfo& ci = m_ci.m_ppline;

	ci = {};
	ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	if(m_pipelineStatisticsEnabled)
	{
		ci.flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR;
	}

	// Prog
	ci.pStages = static_cast<const ShaderProgramImpl&>(*m_state.m_prog).getShaderCreateInfos(ci.stageCount);

	// Vert
	VkPipelineVertexInputStateCreateInfo& vertCi = m_ci.m_vert;
	vertCi = {};
	vertCi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertCi.pVertexAttributeDescriptions = &m_ci.m_attribs[0];
	vertCi.pVertexBindingDescriptions = &m_ci.m_vertBindings[0];

	BitSet<U32(VertexAttribute::kCount), U8> bindingSet = {false};
	for(VertexAttribute semantic : EnumIterable<VertexAttribute>())
	{
		if(m_shaderVertexAttributeMask.get(semantic))
		{
			VkVertexInputAttributeDescription& attrib = m_ci.m_attribs[vertCi.vertexAttributeDescriptionCount++];
			attrib.binding = m_state.m_vertex.m_attributes[semantic].m_binding;
			attrib.format = convertFormat(m_state.m_vertex.m_attributes[semantic].m_format);
			attrib.location = m_semanticToVertexAttributeLocation[semantic];
			attrib.offset = U32(m_state.m_vertex.m_attributes[semantic].m_offset);

			if(!bindingSet.get(attrib.binding))
			{
				bindingSet.set(attrib.binding);

				VkVertexInputBindingDescription& binding = m_ci.m_vertBindings[vertCi.vertexBindingDescriptionCount++];

				binding.binding = attrib.binding;
				binding.inputRate = convertVertexStepRate(m_state.m_vertex.m_bindings[attrib.binding].m_stepRate);
				binding.stride = m_state.m_vertex.m_bindings[attrib.binding].m_stride;
			}
		}
	}

	ci.pVertexInputState = &vertCi;

	// IA
	VkPipelineInputAssemblyStateCreateInfo& iaCi = m_ci.m_ia;
	iaCi = {};
	iaCi.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	iaCi.primitiveRestartEnable = m_state.m_inputAssembler.m_primitiveRestartEnabled;
	iaCi.topology = convertTopology(m_state.m_inputAssembler.m_topology);
	ci.pInputAssemblyState = &iaCi;

	// Viewport
	VkPipelineViewportStateCreateInfo& vpCi = m_ci.m_vp;
	vpCi = {};
	vpCi.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vpCi.scissorCount = 1;
	vpCi.viewportCount = 1;
	ci.pViewportState = &vpCi;

	// Raster
	VkPipelineRasterizationStateCreateInfo& rastCi = m_ci.m_rast;
	rastCi = {};
	rastCi.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rastCi.depthClampEnable = false;
	rastCi.rasterizerDiscardEnable = false;
	rastCi.polygonMode = convertFillMode(m_state.m_rasterizer.m_fillMode);
	rastCi.cullMode = convertCullMode(m_state.m_rasterizer.m_cullMode);
	rastCi.frontFace = (!m_rendersToSwapchain) ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE; // For viewport flip
	rastCi.depthBiasEnable = m_state.m_rasterizer.m_depthBiasEnabled;
	rastCi.lineWidth = 1.0;
	ci.pRasterizationState = &rastCi;

	if(m_state.m_rasterizer.m_rasterizationOrder != RasterizationOrder::kOrdered)
	{
		VkPipelineRasterizationStateRasterizationOrderAMD& rastOrderCi = m_ci.m_rasterOrder;
		rastOrderCi = {};
		rastOrderCi.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD;
		rastOrderCi.rasterizationOrder = convertRasterizationOrder(m_state.m_rasterizer.m_rasterizationOrder);

		appendPNextList(rastCi, &rastOrderCi);
	}

	// MS
	VkPipelineMultisampleStateCreateInfo& msCi = m_ci.m_ms;
	msCi = {};
	msCi.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	msCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	ci.pMultisampleState = &msCi;

	// DS
	if(m_fbDepth || m_fbStencil)
	{
		VkPipelineDepthStencilStateCreateInfo& dsCi = m_ci.m_ds;
		dsCi = {};
		dsCi.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

		if(m_fbDepth)
		{
			dsCi.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			dsCi.depthTestEnable = m_state.m_depth.m_depthCompareFunction != CompareOperation::kAlways || m_state.m_depth.m_depthWriteEnabled;
			dsCi.depthWriteEnable = m_state.m_depth.m_depthWriteEnabled;
			dsCi.depthCompareOp = convertCompareOp(m_state.m_depth.m_depthCompareFunction);
		}

		if(m_fbStencil)
		{
			const StencilPipelineState& ss = m_state.m_stencil;

			dsCi.stencilTestEnable = !stencilTestDisabled(ss.m_face[0].m_stencilFailOperation, ss.m_face[0].m_stencilPassDepthFailOperation,
														  ss.m_face[0].m_stencilPassDepthPassOperation, ss.m_face[0].m_compareFunction)
									 || !stencilTestDisabled(ss.m_face[1].m_stencilFailOperation, ss.m_face[1].m_stencilPassDepthFailOperation,
															 ss.m_face[1].m_stencilPassDepthPassOperation, ss.m_face[1].m_compareFunction);

			dsCi.front.failOp = convertStencilOp(ss.m_face[0].m_stencilFailOperation);
			dsCi.front.passOp = convertStencilOp(ss.m_face[0].m_stencilPassDepthPassOperation);
			dsCi.front.depthFailOp = convertStencilOp(ss.m_face[0].m_stencilPassDepthFailOperation);
			dsCi.front.compareOp = convertCompareOp(ss.m_face[0].m_compareFunction);
			dsCi.back.failOp = convertStencilOp(ss.m_face[1].m_stencilFailOperation);
			dsCi.back.passOp = convertStencilOp(ss.m_face[1].m_stencilPassDepthPassOperation);
			dsCi.back.depthFailOp = convertStencilOp(ss.m_face[1].m_stencilPassDepthFailOperation);
			dsCi.back.compareOp = convertCompareOp(ss.m_face[1].m_compareFunction);
		}

		ci.pDepthStencilState = &dsCi;
	}

	// Color/blend
	if(m_fbColorAttachmentCount)
	{
		VkPipelineColorBlendStateCreateInfo& colCi = m_ci.m_color;
		colCi = {};
		colCi.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colCi.attachmentCount = m_fbColorAttachmentCount;
		colCi.pAttachments = &m_ci.m_colAttachments[0];

		for(U i = 0; i < colCi.attachmentCount; ++i)
		{
			VkPipelineColorBlendAttachmentState& out = m_ci.m_colAttachments[i];
			const ColorAttachmentState& in = m_state.m_color.m_attachments[i];

			out.blendEnable = !blendingDisabled(in.m_srcBlendFactorRgb, in.m_dstBlendFactorRgb, in.m_srcBlendFactorA, in.m_dstBlendFactorA,
												in.m_blendFunctionRgb, in.m_blendFunctionA);
			out.srcColorBlendFactor = convertBlendFactor(in.m_srcBlendFactorRgb);
			out.dstColorBlendFactor = convertBlendFactor(in.m_dstBlendFactorRgb);
			out.srcAlphaBlendFactor = convertBlendFactor(in.m_srcBlendFactorA);
			out.dstAlphaBlendFactor = convertBlendFactor(in.m_dstBlendFactorA);
			out.colorBlendOp = convertBlendOperation(in.m_blendFunctionRgb);
			out.alphaBlendOp = convertBlendOperation(in.m_blendFunctionA);

			out.colorWriteMask = convertColorWriteMask(in.m_channelWriteMask);
		}

		ci.pColorBlendState = &colCi;
	}

	// Dyn state
	VkPipelineDynamicStateCreateInfo& dynCi = m_ci.m_dyn;
	dynCi = {};
	dynCi.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

	// Renderpass related (Dynamic rendering)
	VkPipelineRenderingCreateInfoKHR& dynRendering = m_ci.m_dynamicRendering;
	dynRendering = {};
	dynRendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	dynRendering.colorAttachmentCount = m_fbColorAttachmentCount;
	dynRendering.pColorAttachmentFormats = m_ci.m_dynamicRenderingAttachmentFormats.getBegin();
	for(U i = 0; i < m_fbColorAttachmentCount; ++i)
	{
		m_ci.m_dynamicRenderingAttachmentFormats[i] = convertFormat(m_state.m_attachmentFormats[i]);
	}

	if(m_fbDepth)
	{
		dynRendering.depthAttachmentFormat = convertFormat(m_state.m_attachmentFormats[kMaxColorRenderTargets]);
	}

	if(m_fbStencil)
	{
		dynRendering.stencilAttachmentFormat = convertFormat(m_state.m_attachmentFormats[kMaxColorRenderTargets]);
	}

	appendPNextList(ci, &m_ci.m_dynamicRendering);

	// Almost all state is dynamic. Depth bias is static
	static constexpr Array<VkDynamicState, 10> kDyn = {
		{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_BLEND_CONSTANTS, VK_DYNAMIC_STATE_DEPTH_BOUNDS,
		 VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK, VK_DYNAMIC_STATE_STENCIL_REFERENCE, VK_DYNAMIC_STATE_LINE_WIDTH,
		 VK_DYNAMIC_STATE_DEPTH_BIAS, VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR}};

	dynCi.dynamicStateCount = (m_vrsCapable) ? kDyn.getSize() : (kDyn.getSize() - 1);
	dynCi.pDynamicStates = &kDyn[0];
	ci.pDynamicState = &dynCi;

	// The rest
	ci.layout = static_cast<const ShaderProgramImpl&>(*m_state.m_prog).getPipelineLayout().getHandle();
	ci.subpass = 0;

	return ci;
}

class PipelineFactory::PipelineInternal
{
public:
	VkPipeline m_handle = VK_NULL_HANDLE;
};

class PipelineFactory::Hasher
{
public:
	U64 operator()(U64 h)
	{
		return h;
	}
};

PipelineFactory::PipelineFactory()
{
}

PipelineFactory::~PipelineFactory()
{
}

void PipelineFactory::destroy()
{
	for(auto it : m_pplines)
	{
		if(it.m_handle)
		{
			vkDestroyPipeline(getVkDevice(), it.m_handle, nullptr);
		}
	}

	m_pplines.destroy();
}

void PipelineFactory::getOrCreatePipeline(PipelineStateTracker& state, Pipeline& ppline, Bool& stateDirty)
{
	ANKI_TRACE_SCOPED_EVENT(VkPipelineGetOrCreate);

	U64 hash;
	state.flush(hash, stateDirty);

	if(!stateDirty) [[unlikely]]
	{
		ppline.m_handle = VK_NULL_HANDLE;
		return;
	}

	// Check if ppline exists
	{
		RLockGuard<RWMutex> lock(m_pplinesMtx);
		auto it = m_pplines.find(hash);
		if(it != m_pplines.getEnd())
		{
			ppline.m_handle = (*it).m_handle;
			ANKI_TRACE_INC_COUNTER(VkPipelineCacheHit, 1);
			return;
		}
	}

	// Doesnt exist. Need to create it

	WLockGuard<RWMutex> lock(m_pplinesMtx);

	// Check again
	auto it = m_pplines.find(hash);
	if(it != m_pplines.getEnd())
	{
		ppline.m_handle = (*it).m_handle;
		return;
	}

	// Create it for real
	PipelineInternal pp;
	const VkGraphicsPipelineCreateInfo& ci = state.updatePipelineCreateInfo();

	{
		ANKI_TRACE_SCOPED_EVENT(VkPipelineCreate);

#if ANKI_PLATFORM_MOBILE
		if(m_globalCreatePipelineMtx)
		{
			m_globalCreatePipelineMtx->lock();
		}
#endif

		ANKI_VK_CHECKF(vkCreateGraphicsPipelines(getVkDevice(), m_pplineCache, 1, &ci, nullptr, &pp.m_handle));

#if ANKI_PLATFORM_MOBILE
		if(m_globalCreatePipelineMtx)
		{
			m_globalCreatePipelineMtx->unlock();
		}
#endif
	}

	ANKI_TRACE_INC_COUNTER(VkPipelineCacheMiss, 1);

	m_pplines.emplace(hash, pp);
	ppline.m_handle = pp.m_handle;

	// Print shader info
	getGrManagerImpl().printPipelineShaderInfo(pp.m_handle, state.m_state.m_prog->getName(), hash);
}

} // end namespace anki

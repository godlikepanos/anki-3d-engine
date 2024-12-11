// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkGraphicsState.h>
#include <AnKi/Gr/BackendCommon/Functions.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Gr/Vulkan/VkShaderProgram.h>
#include <AnKi/Util/Filesystem.h>

namespace anki {

static VkViewport computeViewport(const U32 viewport[], U32 fbWidth, U32 fbHeight)
{
	const U32 minx = viewport[0];
	const U32 miny = viewport[1];
	const U32 width = min<U32>(fbWidth, viewport[2]);
	const U32 height = min<U32>(fbHeight, viewport[3]);
	ANKI_ASSERT(width > 0 && height > 0);
	ANKI_ASSERT(minx + width <= fbWidth);
	ANKI_ASSERT(miny + height <= fbHeight);

	const VkViewport s = {.x = F32(minx), .y = F32(height + miny), .width = F32(width), .height = -F32(height), .minDepth = 0.0f, .maxDepth = 1.0f};

	return s;
}

static VkRect2D computeScissor(const U32 scissor[], U32 fbWidth, U32 fbHeight)
{
	const U32 minx = scissor[0];
	const U32 miny = scissor[1];
	const U32 width = min<U32>(fbWidth, scissor[2]);
	const U32 height = min<U32>(fbHeight, scissor[3]);
	ANKI_ASSERT(width > 0 && height > 0);
	ANKI_ASSERT(minx + width <= fbWidth);
	ANKI_ASSERT(miny + height <= fbHeight);

	VkRect2D out = {};
	out.extent.width = width;
	out.extent.height = height;
	out.offset.x = minx;
	out.offset.y = fbHeight - (miny + height);

	return out;
}

GraphicsPipelineFactory::~GraphicsPipelineFactory()
{
	for(auto pso : m_map)
	{
		vkDestroyPipeline(getVkDevice(), pso, nullptr);
	}
}

void GraphicsPipelineFactory::flushState(GraphicsStateTracker& state, VkCommandBuffer& cmdb)
{
	const GraphicsStateTracker::StaticState& staticState = state.m_staticState;
	GraphicsStateTracker::DynamicState& dynState = state.m_dynState;

	// Set dynamic state
	const auto& ss = staticState.m_stencil;
	const Bool stencilTestEnabled = anki::stencilTestEnabled(ss.m_face[0].m_fail, ss.m_face[0].m_stencilPassDepthFail,
															 ss.m_face[0].m_stencilPassDepthPass, ss.m_face[0].m_compare)
									|| anki::stencilTestEnabled(ss.m_face[1].m_fail, ss.m_face[1].m_stencilPassDepthFail,
																ss.m_face[1].m_stencilPassDepthPass, ss.m_face[1].m_compare);

	const Bool hasStencilRt =
		staticState.m_misc.m_depthStencilFormat != Format::kNone && getFormatInfo(staticState.m_misc.m_depthStencilFormat).isStencil();

	if(stencilTestEnabled && hasStencilRt && dynState.m_stencilCompareMaskDirty)
	{
		ANKI_ASSERT(dynState.m_stencilFaces[0].m_compareMask != 0x5A5A5A5A && dynState.m_stencilFaces[1].m_compareMask != 0x5A5A5A5A);
		dynState.m_stencilCompareMaskDirty = false;

		if(dynState.m_stencilFaces[0].m_compareMask == dynState.m_stencilFaces[1].m_compareMask)
		{
			vkCmdSetStencilCompareMask(cmdb, VK_STENCIL_FACE_FRONT_AND_BACK, dynState.m_stencilFaces[0].m_compareMask);
		}
		else
		{
			vkCmdSetStencilCompareMask(cmdb, VK_STENCIL_FACE_FRONT_BIT, dynState.m_stencilFaces[0].m_compareMask);
			vkCmdSetStencilCompareMask(cmdb, VK_STENCIL_FACE_BACK_BIT, dynState.m_stencilFaces[1].m_compareMask);
		}
	}

	if(stencilTestEnabled && hasStencilRt && dynState.m_stencilWriteMaskDirty)
	{
		ANKI_ASSERT(dynState.m_stencilFaces[0].m_writeMask != 0x5A5A5A5A && dynState.m_stencilFaces[1].m_writeMask != 0x5A5A5A5A);
		dynState.m_stencilWriteMaskDirty = false;

		if(dynState.m_stencilFaces[0].m_writeMask == dynState.m_stencilFaces[1].m_writeMask)
		{
			vkCmdSetStencilWriteMask(cmdb, VK_STENCIL_FACE_FRONT_AND_BACK, dynState.m_stencilFaces[0].m_writeMask);
		}
		else
		{
			vkCmdSetStencilWriteMask(cmdb, VK_STENCIL_FACE_FRONT_BIT, dynState.m_stencilFaces[0].m_writeMask);
			vkCmdSetStencilWriteMask(cmdb, VK_STENCIL_FACE_BACK_BIT, dynState.m_stencilFaces[1].m_writeMask);
		}
	}

	if(stencilTestEnabled && hasStencilRt && dynState.m_stencilRefDirty)
	{
		ANKI_ASSERT(dynState.m_stencilFaces[0].m_ref != 0x5A5A5A5A && dynState.m_stencilFaces[1].m_ref != 0x5A5A5A5A);
		dynState.m_stencilRefDirty = false;

		if(dynState.m_stencilFaces[0].m_ref == dynState.m_stencilFaces[1].m_ref)
		{
			vkCmdSetStencilReference(cmdb, VK_STENCIL_FACE_FRONT_AND_BACK, dynState.m_stencilFaces[0].m_ref);
		}
		else
		{
			vkCmdSetStencilReference(cmdb, VK_STENCIL_FACE_FRONT_BIT, dynState.m_stencilFaces[0].m_ref);
			vkCmdSetStencilReference(cmdb, VK_STENCIL_FACE_BACK_BIT, dynState.m_stencilFaces[1].m_ref);
		}
	}

	const Bool hasDepthRt =
		staticState.m_misc.m_depthStencilFormat != Format::kNone && getFormatInfo(staticState.m_misc.m_depthStencilFormat).isDepth();

	const Bool depthTestEnabled = anki::depthTestEnabled(staticState.m_depth.m_compare, staticState.m_depth.m_writeEnabled);

	if(hasDepthRt && depthTestEnabled && dynState.m_depthBiasDirty)
	{
		dynState.m_depthBiasDirty = false;

		vkCmdSetDepthBias(cmdb, dynState.m_depthBiasConstantFactor, dynState.m_depthBiasClamp, dynState.m_depthBiasSlopeFactor);
	}

	if(dynState.m_viewportDirty)
	{
		ANKI_ASSERT(dynState.m_viewport[2] != 0 && dynState.m_viewport[3] != 0);
		dynState.m_viewportDirty = false;
		const VkViewport vp = computeViewport(dynState.m_viewport.getBegin(), state.m_rtsSize.x(), state.m_rtsSize.y());
		vkCmdSetViewport(cmdb, 0, 1, &vp);
	}

	if(dynState.m_scissorDirty)
	{
		dynState.m_scissorDirty = false;
		const VkRect2D rect = computeScissor(dynState.m_scissor.getBegin(), state.m_rtsSize.x(), state.m_rtsSize.y());
		vkCmdSetScissor(cmdb, 0, 1, &rect);
	}

	if(dynState.m_lineWidthDirty)
	{
		dynState.m_lineWidthDirty = false;
		vkCmdSetLineWidth(cmdb, dynState.m_lineWidth);
	}

	// Static state
	const Bool rebindPso = state.updateHashes();

	// Find the PSO
	VkPipeline pso = VK_NULL_HANDLE;
	{
		RLockGuard<RWMutex> lock(m_mtx);

		auto it = m_map.find(state.m_globalHash);
		if(it != m_map.getEnd())
		{
			pso = *it;
		}
	}

	if(pso) [[likely]]
	{
		if(rebindPso)
		{
			vkCmdBindPipeline(cmdb, VK_PIPELINE_BIND_POINT_GRAPHICS, pso);
		}

		return;
	}

	// PSO not found, proactively create it WITHOUT a lock (we dont't want to serialize pipeline creation)

	const ShaderProgramImpl& prog = static_cast<const ShaderProgramImpl&>(*staticState.m_shaderProg);

	VkGraphicsPipelineCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	if(staticState.m_misc.m_pipelineStatisticsEnabled)
	{
		ci.flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR;
	}

	ci.pStages = prog.getShaderCreateInfos(ci.stageCount);

	// Vertex stuff
	Array<VkVertexInputBindingDescription, U32(VertexAttributeSemantic::kCount)> vertBindings;
	Array<VkVertexInputAttributeDescription, U32(VertexAttributeSemantic::kCount)> attribs;

	VkPipelineVertexInputStateCreateInfo vertCi = {};
	vertCi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertCi.pVertexAttributeDescriptions = &attribs[0];
	vertCi.pVertexBindingDescriptions = &vertBindings[0];

	BitSet<U32(VertexAttributeSemantic::kCount), U8> bindingSet = {false};
	for(VertexAttributeSemantic semantic : EnumBitsIterable<VertexAttributeSemantic, VertexAttributeSemanticBit>(staticState.m_vert.m_activeAttribs))
	{
		VkVertexInputAttributeDescription& attrib = attribs[vertCi.vertexAttributeDescriptionCount++];
		attrib.binding = staticState.m_vert.m_attribs[semantic].m_binding;
		attrib.format = convertFormat(staticState.m_vert.m_attribs[semantic].m_fmt);
		attrib.location = staticState.m_vert.m_attribs[semantic].m_semanticToVertexAttributeLocation;
		attrib.offset = staticState.m_vert.m_attribs[semantic].m_relativeOffset;

		if(!bindingSet.get(attrib.binding))
		{
			bindingSet.set(attrib.binding);

			VkVertexInputBindingDescription& binding = vertBindings[vertCi.vertexBindingDescriptionCount++];

			binding.binding = attrib.binding;
			binding.inputRate = convertVertexStepRate(staticState.m_vert.m_bindings[attrib.binding].m_stepRate);
			binding.stride = staticState.m_vert.m_bindings[attrib.binding].m_stride;
		}
	}

	ci.pVertexInputState = &vertCi;

	// IA
	VkPipelineInputAssemblyStateCreateInfo iaCi = {};
	iaCi.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	iaCi.primitiveRestartEnable = staticState.m_ia.m_primitiveRestartEnabled;
	iaCi.topology = convertTopology(staticState.m_ia.m_topology);
	ci.pInputAssemblyState = &iaCi;

	// Viewport
	VkPipelineViewportStateCreateInfo vpCi = {};
	vpCi.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vpCi.scissorCount = 1;
	vpCi.viewportCount = 1;
	ci.pViewportState = &vpCi;

	// Raster
	VkPipelineRasterizationStateCreateInfo rastCi = {};
	rastCi.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rastCi.depthClampEnable = false;
	rastCi.rasterizerDiscardEnable = false;
	rastCi.polygonMode = convertFillMode(staticState.m_rast.m_fillMode);
	rastCi.cullMode = convertCullMode(staticState.m_rast.m_cullMode);
	rastCi.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rastCi.depthBiasEnable = staticState.m_rast.m_depthBiasEnabled;
	rastCi.lineWidth = 1.0f;
	ci.pRasterizationState = &rastCi;

	// MS
	VkPipelineMultisampleStateCreateInfo msCi = {};
	msCi.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	msCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	ci.pMultisampleState = &msCi;

	// Depth stencil
	VkPipelineDepthStencilStateCreateInfo dsCi;
	if(hasDepthRt || hasStencilRt)
	{
		dsCi = {};
		dsCi.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

		if(hasDepthRt)
		{
			dsCi.depthTestEnable = depthTestEnabled;
			dsCi.depthWriteEnable = staticState.m_depth.m_writeEnabled;
			dsCi.depthCompareOp = convertCompareOp(staticState.m_depth.m_compare);
		}

		if(hasStencilRt)
		{
			const auto& ss = staticState.m_stencil;

			dsCi.stencilTestEnable = stencilTestEnabled;
			dsCi.front.failOp = convertStencilOp(ss.m_face[0].m_fail);
			dsCi.front.passOp = convertStencilOp(ss.m_face[0].m_stencilPassDepthPass);
			dsCi.front.depthFailOp = convertStencilOp(ss.m_face[0].m_stencilPassDepthFail);
			dsCi.front.compareOp = convertCompareOp(ss.m_face[0].m_compare);
			dsCi.back.failOp = convertStencilOp(ss.m_face[1].m_fail);
			dsCi.back.passOp = convertStencilOp(ss.m_face[1].m_stencilPassDepthPass);
			dsCi.back.depthFailOp = convertStencilOp(ss.m_face[1].m_stencilPassDepthFail);
			dsCi.back.compareOp = convertCompareOp(ss.m_face[1].m_compare);
		}

		ci.pDepthStencilState = &dsCi;
	}

	// Color/blend
	Array<VkPipelineColorBlendAttachmentState, kMaxColorRenderTargets> colAttachments = {};

	VkPipelineColorBlendStateCreateInfo colCi = {};
	colCi.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	if(staticState.m_misc.m_colorRtMask.getAnySet())
	{
		colCi.attachmentCount = staticState.m_misc.m_colorRtMask.getSetBitCount();
		colCi.pAttachments = &colAttachments[0];

		for(U i = 0; i < colCi.attachmentCount; ++i)
		{
			VkPipelineColorBlendAttachmentState& out = colAttachments[i];
			const auto& in = staticState.m_blend.m_colorRts[i];

			out.blendEnable = blendingEnabled(in.m_srcRgb, in.m_dstRgb, in.m_srcA, in.m_dstA, in.m_funcRgb, in.m_funcA);
			out.srcColorBlendFactor = convertBlendFactor(in.m_srcRgb);
			out.dstColorBlendFactor = convertBlendFactor(in.m_dstRgb);
			out.srcAlphaBlendFactor = convertBlendFactor(in.m_srcA);
			out.dstAlphaBlendFactor = convertBlendFactor(in.m_dstA);
			out.colorBlendOp = convertBlendOperation(in.m_funcRgb);
			out.alphaBlendOp = convertBlendOperation(in.m_funcA);

			out.colorWriteMask = convertColorWriteMask(in.m_channelWriteMask);
		}

		ci.pColorBlendState = &colCi;
	}

	// Renderpass related (Dynamic rendering)
	Array<VkFormat, kMaxColorRenderTargets> dynamicRenderingAttachmentFormats = {};
	VkPipelineRenderingCreateInfoKHR dynRendering = {};
	dynRendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	dynRendering.colorAttachmentCount = staticState.m_misc.m_colorRtMask.getSetBitCount();
	dynRendering.pColorAttachmentFormats = dynamicRenderingAttachmentFormats.getBegin();
	for(U i = 0; i < kMaxColorRenderTargets; ++i)
	{
		dynamicRenderingAttachmentFormats[i] =
			(staticState.m_misc.m_colorRtMask.get(i)) ? convertFormat(staticState.m_misc.m_colorRtFormats[i]) : VK_FORMAT_UNDEFINED;
	}

	if(hasDepthRt)
	{
		dynRendering.depthAttachmentFormat = convertFormat(staticState.m_misc.m_depthStencilFormat);
	}

	if(hasStencilRt)
	{
		dynRendering.stencilAttachmentFormat = convertFormat(staticState.m_misc.m_depthStencilFormat);
	}

	appendPNextList(ci, &dynRendering);

	// Almost all state is dynamic. Depth bias is static
	VkPipelineDynamicStateCreateInfo dynCi = {};
	dynCi.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

	static constexpr Array<VkDynamicState, 10> kDyn = {
		{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_BLEND_CONSTANTS, VK_DYNAMIC_STATE_DEPTH_BOUNDS,
		 VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK, VK_DYNAMIC_STATE_STENCIL_REFERENCE, VK_DYNAMIC_STATE_LINE_WIDTH,
		 VK_DYNAMIC_STATE_DEPTH_BIAS, VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR}};

	dynCi.dynamicStateCount = (getGrManagerImpl().getDeviceCapabilities().m_vrs) ? kDyn.getSize() : (kDyn.getSize() - 1);
	dynCi.pDynamicStates = &kDyn[0];
	ci.pDynamicState = &dynCi;

	// The rest
	ci.layout = prog.getPipelineLayout().getHandle();
	ci.subpass = 0;

	// Create the pipeline
	{
		ANKI_TRACE_SCOPED_EVENT(VkPipelineCreate);

#if ANKI_PLATFORM_MOBILE
		if(PipelineCache::getSingleton().m_globalCreatePipelineMtx)
		{
			PipelineCache::getSingleton().m_globalCreatePipelineMtx->lock();
		}
#endif

		ANKI_VK_CHECKF(vkCreateGraphicsPipelines(getVkDevice(), PipelineCache::getSingleton().m_cacheHandle, 1, &ci, nullptr, &pso));

#if ANKI_PLATFORM_MOBILE
		if(PipelineCache::getSingleton().m_globalCreatePipelineMtx)
		{
			PipelineCache::getSingleton().m_globalCreatePipelineMtx->unlock();
		}
#endif
	}

	// Now try to add the PSO to the hashmap
	{
		WLockGuard<RWMutex> lock(m_mtx);

		auto it = m_map.find(state.m_globalHash);
		if(it == m_map.getEnd())
		{
			// Not found, add it
			m_map.emplace(state.m_globalHash, pso);
		}
		else
		{
			// Found, remove the PSO that was proactively created and use the old one
			vkDestroyPipeline(getVkDevice(), pso, nullptr);
			pso = *it;
		}
	}

	// Final thing, bind the PSO
	vkCmdBindPipeline(cmdb, VK_PIPELINE_BIND_POINT_GRAPHICS, pso);
}

Error PipelineCache::init(CString cacheDir)
{
	ANKI_ASSERT(cacheDir);
	m_dumpSize = g_diskShaderCacheMaxSizeCVar;
	m_dumpFilename.sprintf("%s/VkPipelineCache", cacheDir.cstr());

	// Try read the pipeline cache file.
	GrDynamicArray<U8, PtrSize> diskDump;
	if(fileExists(m_dumpFilename.toCString()))
	{
		File file;
		ANKI_CHECK(file.open(m_dumpFilename.toCString(), FileOpenFlag::kBinary | FileOpenFlag::kRead));

		const PtrSize diskDumpSize = file.getSize();
		if(diskDumpSize <= sizeof(U8) * VK_UUID_SIZE)
		{
			ANKI_VK_LOGI("Pipeline cache dump appears to be empty: %s", &m_dumpFilename[0]);
		}
		else
		{
			// Get current pipeline UUID and compare it with the cache's
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(getGrManagerImpl().getPhysicalDevice(), &props);

			Array<U8, VK_UUID_SIZE> cacheUuid;
			ANKI_CHECK(file.read(&cacheUuid[0], VK_UUID_SIZE));

			if(memcmp(&cacheUuid[0], &props.pipelineCacheUUID[0], VK_UUID_SIZE) != 0)
			{
				ANKI_VK_LOGI("Pipeline cache dump is not compatible with the current device: %s", &m_dumpFilename[0]);
			}
			else
			{
				diskDump.resize(diskDumpSize - VK_UUID_SIZE);
				ANKI_CHECK(file.read(&diskDump[0], diskDumpSize - VK_UUID_SIZE));
			}
		}
	}
	else
	{
		ANKI_VK_LOGI("Pipeline cache dump not found: %s", &m_dumpFilename[0]);
	}

	// Create the cache
	VkPipelineCacheCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	if(diskDump.getSize())
	{
		ANKI_VK_LOGI("Will load %zu bytes of pipeline cache", diskDump.getSize());
		ci.initialDataSize = diskDump.getSize();
		ci.pInitialData = &diskDump[0];
	}

	ANKI_VK_CHECK(vkCreatePipelineCache(getVkDevice(), &ci, nullptr, &m_cacheHandle));

#if ANKI_PLATFORM_MOBILE
	ANKI_ASSERT(GrManager::getSingleton().getDeviceCapabilities().m_gpuVendor != GpuVendor::kUnknown);
	if(GrManager::getSingleton().getDeviceCapabilities().m_gpuVendor == GpuVendor::kQualcomm)
	{
		// Calling vkCreateGraphicsPipeline from multiple threads crashes qualcomm's compiler
		ANKI_VK_LOGI("Enabling workaround for vkCreateGraphicsPipeline crashing when called from multiple threads");
		m_globalCreatePipelineMtx = anki::newInstance<Mutex>(GrMemoryPool::getSingleton());
	}
#endif

	return Error::kNone;
}

void PipelineCache::destroy()
{
	const Error err = destroyInternal();
	if(err)
	{
		ANKI_VK_LOGE("An error occurred while storing the pipeline cache to disk. Will ignore");
	}

	m_dumpFilename.destroy();
}

Error PipelineCache::destroyInternal()
{
#if ANKI_PLATFORM_MOBILE
	deleteInstance(GrMemoryPool::getSingleton(), m_globalCreatePipelineMtx);
#endif

	if(m_cacheHandle)
	{
		// Get size of cache
		size_t size = 0;
		ANKI_VK_CHECK(vkGetPipelineCacheData(getVkDevice(), m_cacheHandle, &size, nullptr));
		size = min(size, m_dumpSize);

		if(size > 0)
		{
			// Read cache
			GrDynamicArray<U8, PtrSize> cacheData;
			cacheData.resize(size);
			ANKI_VK_CHECK(vkGetPipelineCacheData(getVkDevice(), m_cacheHandle, &size, &cacheData[0]));

			// Write file
			File file;
			ANKI_CHECK(file.open(&m_dumpFilename[0], FileOpenFlag::kBinary | FileOpenFlag::kWrite));

			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(getGrManagerImpl().getPhysicalDevice(), &props);

			ANKI_CHECK(file.write(&props.pipelineCacheUUID[0], VK_UUID_SIZE));
			ANKI_CHECK(file.write(&cacheData[0], size));

			ANKI_VK_LOGI("Dumped %zu bytes of the pipeline cache", size);
		}

		// Destroy cache
		vkDestroyPipelineCache(getVkDevice(), m_cacheHandle, nullptr);
		m_cacheHandle = VK_NULL_HANDLE;
	}

	return Error::kNone;
}

} // end namespace anki

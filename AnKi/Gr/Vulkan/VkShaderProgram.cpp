// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkShaderProgram.h>
#include <AnKi/Gr/Vulkan/VkShader.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Gr/Vulkan/VkPipelineFactory.h>
#include <AnKi/Gr/BackendCommon/Functions.h>

namespace anki {

ShaderProgram* ShaderProgram::newInstance(const ShaderProgramInitInfo& init)
{
	ShaderProgramImpl* impl = anki::newInstance<ShaderProgramImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

ConstWeakArray<U8> ShaderProgram::getShaderGroupHandles() const
{
	return static_cast<const ShaderProgramImpl&>(*this).getShaderGroupHandlesInternal();
}

Buffer& ShaderProgram::getShaderGroupHandlesGpuBuffer() const
{
	return static_cast<const ShaderProgramImpl&>(*this).getShaderGroupHandlesGpuBufferInternal();
}

ShaderProgramImpl::~ShaderProgramImpl()
{
	if(m_graphics.m_pplineFactory)
	{
		m_graphics.m_pplineFactory->destroy();
		deleteInstance(GrMemoryPool::getSingleton(), m_graphics.m_pplineFactory);
	}

	if(m_compute.m_ppline)
	{
		vkDestroyPipeline(getVkDevice(), m_compute.m_ppline, nullptr);
	}

	if(m_rt.m_ppline)
	{
		vkDestroyPipeline(getVkDevice(), m_rt.m_ppline, nullptr);
	}
}

Error ShaderProgramImpl::init(const ShaderProgramInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());

	// Create the shader references
	//
	GrHashMap<U64, U32> shaderUuidToMShadersIdx; // Shader UUID to m_shaders idx
	if(inf.m_computeShader)
	{
		m_shaders.emplaceBack(inf.m_computeShader);
	}
	else if(inf.m_graphicsShaders[ShaderType::kFragment])
	{
		for(Shader* s : inf.m_graphicsShaders)
		{
			if(s)
			{
				m_shaders.emplaceBack(s);
			}
		}
	}
	else
	{
		// Ray tracing

		m_shaders.resizeStorage(inf.m_rayTracingShaders.m_rayGenShaders.getSize() + inf.m_rayTracingShaders.m_missShaders.getSize()
								+ 1); // Plus at least one hit shader

		for(Shader* s : inf.m_rayTracingShaders.m_rayGenShaders)
		{
			m_shaders.emplaceBack(s);
		}

		for(Shader* s : inf.m_rayTracingShaders.m_missShaders)
		{
			m_shaders.emplaceBack(s);
		}

		m_rt.m_missShaderCount = inf.m_rayTracingShaders.m_missShaders.getSize();

		for(const RayTracingHitGroup& group : inf.m_rayTracingShaders.m_hitGroups)
		{
			if(group.m_anyHitShader)
			{
				auto it = shaderUuidToMShadersIdx.find(group.m_anyHitShader->getUuid());
				if(it == shaderUuidToMShadersIdx.getEnd())
				{
					shaderUuidToMShadersIdx.emplace(group.m_anyHitShader->getUuid(), m_shaders.getSize());
					m_shaders.emplaceBack(group.m_anyHitShader);
				}
			}

			if(group.m_closestHitShader)
			{
				auto it = shaderUuidToMShadersIdx.find(group.m_closestHitShader->getUuid());
				if(it == shaderUuidToMShadersIdx.getEnd())
				{
					shaderUuidToMShadersIdx.emplace(group.m_closestHitShader->getUuid(), m_shaders.getSize());
					m_shaders.emplaceBack(group.m_closestHitShader);
				}
			}
		}
	}

	ANKI_ASSERT(m_shaders.getSize() > 0);

	// Link reflection
	//
	Bool firstLink = true;
	for(ShaderPtr& shader : m_shaders)
	{
		m_shaderTypes |= ShaderTypeBit(1 << shader->getShaderType());

		const ShaderImpl& simpl = static_cast<const ShaderImpl&>(*shader);
		if(firstLink)
		{
			m_refl = simpl.m_reflection;
			firstLink = false;
		}
		else
		{
			ANKI_CHECK(linkShaderReflection(m_refl, simpl.m_reflection, m_refl));
		}

		m_refl.validate();
	}

	// Get bindings
	//
	Array2d<DSBinding, kMaxDescriptorSets, kMaxBindingsPerDescriptorSet> bindings;
	Array<U32, kMaxDescriptorSets> counts = {};
	U32 descriptorSetCount = 0;
	for(U32 set = 0; set < kMaxDescriptorSets; ++set)
	{
		for(U8 binding = 0; m_refl.m_descriptorSetMask.get(set) && binding < kMaxBindingsPerDescriptorSet; ++binding)
		{
			if(m_refl.m_descriptorArraySizes[set][binding])
			{
				DSBinding b;
				b.m_arraySize = m_refl.m_descriptorArraySizes[set][binding];
				b.m_binding = binding;
				b.m_type = convertDescriptorType(m_refl.m_descriptorTypes[set][binding], m_refl.m_descriptorFlags[set][binding]);

				bindings[set][counts[set]++] = b;
			}
		}

		// We may end up with ppline layouts with "empty" dslayouts. That's fine, we want it.
		if(counts[set])
		{
			descriptorSetCount = set + 1;
		}
	}

	// Create the descriptor set layouts
	//
	for(U32 set = 0; set < descriptorSetCount; ++set)
	{
		ANKI_CHECK(DSLayoutFactory::getSingleton().getOrCreateDescriptorSetLayout(
			WeakArray<DSBinding>((counts[set]) ? &bindings[set][0] : nullptr, counts[set]), m_descriptorSetLayouts[set]));

		// Even if the dslayout is empty we will have to list it because we'll have to bind a DS for it.
		m_refl.m_descriptorSetMask.set(set);
	}

	// Create the ppline layout
	//
	WeakArray<const DSLayout*> dsetLayouts((descriptorSetCount) ? &m_descriptorSetLayouts[0] : nullptr, descriptorSetCount);
	ANKI_CHECK(PipelineLayoutFactory::getSingleton().newPipelineLayout(dsetLayouts, m_refl.m_pushConstantsSize, m_pplineLayout));

	// Get some masks
	//
	const Bool graphicsProg = !!(m_shaderTypes & ShaderTypeBit::kAllGraphics);
	if(graphicsProg)
	{
		const U32 attachmentCount = m_refl.m_colorAttachmentWritemask.getSetBitCount();
		for(U32 i = 0; i < attachmentCount; ++i)
		{
			ANKI_ASSERT(m_refl.m_colorAttachmentWritemask.get(i) && "Should write to all attachments");
		}
	}

	// Init the create infos
	//
	if(graphicsProg)
	{
		for(const ShaderPtr& shader : m_shaders)
		{
			const ShaderImpl& shaderImpl = static_cast<const ShaderImpl&>(*shader);

			VkPipelineShaderStageCreateInfo& createInf = m_graphics.m_shaderCreateInfos[m_graphics.m_shaderCreateInfoCount++];
			createInf = {};
			createInf.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			createInf.stage = VkShaderStageFlagBits(convertShaderTypeBit(ShaderTypeBit(1 << shader->getShaderType())));
			createInf.pName = "main";
			createInf.module = shaderImpl.m_handle;
		}
	}

	// Create the factory
	//
	if(graphicsProg)
	{
		m_graphics.m_pplineFactory = anki::newInstance<PipelineFactory>(GrMemoryPool::getSingleton());
	}

	// Create the pipeline if compute
	//
	if(!!(m_shaderTypes & ShaderTypeBit::kCompute))
	{
		const ShaderImpl& shaderImpl = static_cast<const ShaderImpl&>(*m_shaders[0]);

		VkComputePipelineCreateInfo ci = {};

		if(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kKHR_pipeline_executable_properties))
		{
			ci.flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR;
		}

		ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		ci.layout = m_pplineLayout.getHandle();

		ci.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		ci.stage.pName = "main";
		ci.stage.module = shaderImpl.m_handle;

		ANKI_TRACE_SCOPED_EVENT(VkPipelineCreate);
		ANKI_VK_CHECK(vkCreateComputePipelines(getVkDevice(), PipelineCache::getSingleton().m_cacheHandle, 1, &ci, nullptr, &m_compute.m_ppline));
		getGrManagerImpl().printPipelineShaderInfo(m_compute.m_ppline, getName());
	}

	// Create the RT pipeline
	//
	if(!!(m_shaderTypes & ShaderTypeBit::kAllRayTracing))
	{
		// Create shaders
		GrDynamicArray<VkPipelineShaderStageCreateInfo> stages;
		stages.resize(m_shaders.getSize());
		for(U32 i = 0; i < stages.getSize(); ++i)
		{
			const ShaderImpl& impl = static_cast<const ShaderImpl&>(*m_shaders[i]);

			VkPipelineShaderStageCreateInfo& stage = stages[i];
			stage = {};
			stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage.stage = VkShaderStageFlagBits(convertShaderTypeBit(ShaderTypeBit(1 << impl.getShaderType())));
			stage.pName = "main";
			stage.module = impl.m_handle;
		}

		// Create groups
		VkRayTracingShaderGroupCreateInfoKHR defaultGroup = {};
		defaultGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		defaultGroup.generalShader = VK_SHADER_UNUSED_KHR;
		defaultGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		defaultGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		defaultGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

		U32 groupCount = inf.m_rayTracingShaders.m_rayGenShaders.getSize() + inf.m_rayTracingShaders.m_missShaders.getSize()
						 + inf.m_rayTracingShaders.m_hitGroups.getSize();
		GrDynamicArray<VkRayTracingShaderGroupCreateInfoKHR> groups;
		groups.resize(groupCount, defaultGroup);

		// 1st group is the ray gen
		groupCount = 0;
		for(U32 i = 0; i < inf.m_rayTracingShaders.m_rayGenShaders.getSize(); ++i)
		{
			groups[groupCount].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			groups[groupCount].generalShader = groupCount;
			++groupCount;
		}

		// Miss
		for(U32 i = 0; i < inf.m_rayTracingShaders.m_missShaders.getSize(); ++i)
		{
			groups[groupCount].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			groups[groupCount].generalShader = groupCount;
			++groupCount;
		}

		// The rest of the groups are hit
		for(U32 i = 0; i < inf.m_rayTracingShaders.m_hitGroups.getSize(); ++i)
		{
			groups[groupCount].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			if(inf.m_rayTracingShaders.m_hitGroups[i].m_anyHitShader)
			{
				groups[groupCount].anyHitShader = *shaderUuidToMShadersIdx.find(inf.m_rayTracingShaders.m_hitGroups[i].m_anyHitShader->getUuid());
			}

			if(inf.m_rayTracingShaders.m_hitGroups[i].m_closestHitShader)
			{
				groups[groupCount].closestHitShader =
					*shaderUuidToMShadersIdx.find(inf.m_rayTracingShaders.m_hitGroups[i].m_closestHitShader->getUuid());
			}

			++groupCount;
		}

		ANKI_ASSERT(groupCount == groups.getSize());

		VkRayTracingPipelineCreateInfoKHR ci = {};
		ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		ci.stageCount = stages.getSize();
		ci.pStages = &stages[0];
		ci.groupCount = groups.getSize();
		ci.pGroups = &groups[0];
		ci.maxPipelineRayRecursionDepth = inf.m_rayTracingShaders.m_maxRecursionDepth;
		ci.layout = m_pplineLayout.getHandle();

		{
			ANKI_TRACE_SCOPED_EVENT(VkPipelineCreate);
			ANKI_VK_CHECK(vkCreateRayTracingPipelinesKHR(getVkDevice(), VK_NULL_HANDLE, PipelineCache::getSingleton().m_cacheHandle, 1, &ci, nullptr,
														 &m_rt.m_ppline));
		}

		// Get RT handles
		const U32 handleArraySize = getGrManagerImpl().getPhysicalDeviceRayTracingProperties().shaderGroupHandleSize * groupCount;
		m_rt.m_allHandles.resize(handleArraySize, 0_U8);
		ANKI_VK_CHECK(vkGetRayTracingShaderGroupHandlesKHR(getVkDevice(), m_rt.m_ppline, 0, groupCount, handleArraySize, &m_rt.m_allHandles[0]));

		// Upload RT handles
		BufferInitInfo buffInit("RT handles");
		buffInit.m_size = m_rt.m_allHandles.getSizeInBytes();
		buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
		buffInit.m_usage = BufferUsageBit::kAllCompute & BufferUsageBit::kAllRead;
		m_rt.m_allHandlesBuff = getGrManagerImpl().newBuffer(buffInit);

		void* mapped = m_rt.m_allHandlesBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite);
		memcpy(mapped, m_rt.m_allHandles.getBegin(), m_rt.m_allHandles.getSizeInBytes());
		m_rt.m_allHandlesBuff->unmap();
	}

	// Get shader sizes and a few other things
	//
	for(const ShaderPtr& s : m_shaders)
	{
		if(!s.isCreated())
		{
			continue;
		}

		const ShaderType type = s->getShaderType();
		const U32 size = s->getShaderBinarySize();

		m_shaderBinarySizes[type] = size;

		if(type == ShaderType::kFragment)
		{
			m_hasDiscard = s->hasDiscard();
		}
	}

	return Error::kNone;
}

} // end namespace anki

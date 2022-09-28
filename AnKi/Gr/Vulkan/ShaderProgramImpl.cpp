// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/ShaderProgramImpl.h>
#include <AnKi/Gr/Shader.h>
#include <AnKi/Gr/Vulkan/ShaderImpl.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Gr/Vulkan/Pipeline.h>

namespace anki {

ShaderProgramImpl::~ShaderProgramImpl()
{
	if(m_graphics.m_pplineFactory)
	{
		m_graphics.m_pplineFactory->destroy();
		deleteInstance(getMemoryPool(), m_graphics.m_pplineFactory);
	}

	if(m_compute.m_ppline)
	{
		vkDestroyPipeline(getDevice(), m_compute.m_ppline, nullptr);
	}

	if(m_rt.m_ppline)
	{
		vkDestroyPipeline(getDevice(), m_rt.m_ppline, nullptr);
	}

	m_shaders.destroy(getMemoryPool());
	m_rt.m_allHandles.destroy(getMemoryPool());
}

Error ShaderProgramImpl::init(const ShaderProgramInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());

	// Create the shader references
	//
	HashMapRaii<U64, U32> shaderUuidToMShadersIdx(&getMemoryPool()); // Shader UUID to m_shaders idx
	if(inf.m_computeShader)
	{
		m_shaders.emplaceBack(getMemoryPool(), inf.m_computeShader);
	}
	else if(inf.m_graphicsShaders[ShaderType::kVertex])
	{
		for(const ShaderPtr& s : inf.m_graphicsShaders)
		{
			if(s)
			{
				m_shaders.emplaceBack(getMemoryPool(), s);
			}
		}
	}
	else
	{
		// Ray tracing

		m_shaders.resizeStorage(getMemoryPool(), inf.m_rayTracingShaders.m_rayGenShaders.getSize()
													 + inf.m_rayTracingShaders.m_missShaders.getSize()
													 + 1); // Plus at least one hit shader

		for(const ShaderPtr& s : inf.m_rayTracingShaders.m_rayGenShaders)
		{
			m_shaders.emplaceBack(getMemoryPool(), s);
		}

		for(const ShaderPtr& s : inf.m_rayTracingShaders.m_missShaders)
		{
			m_shaders.emplaceBack(getMemoryPool(), s);
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
					m_shaders.emplaceBack(getMemoryPool(), group.m_anyHitShader);
				}
			}

			if(group.m_closestHitShader)
			{
				auto it = shaderUuidToMShadersIdx.find(group.m_closestHitShader->getUuid());
				if(it == shaderUuidToMShadersIdx.getEnd())
				{
					shaderUuidToMShadersIdx.emplace(group.m_closestHitShader->getUuid(), m_shaders.getSize());
					m_shaders.emplaceBack(getMemoryPool(), group.m_closestHitShader);
				}
			}
		}
	}

	ANKI_ASSERT(m_shaders.getSize() > 0);

	// Merge bindings
	//
	Array2d<DescriptorBinding, kMaxDescriptorSets, kMaxBindingsPerDescriptorSet> bindings;
	Array<U32, kMaxDescriptorSets> counts = {};
	U32 descriptorSetCount = 0;
	for(U32 set = 0; set < kMaxDescriptorSets; ++set)
	{
		for(ShaderPtr& shader : m_shaders)
		{
			m_stages |= ShaderTypeBit(1 << shader->getShaderType());

			const ShaderImpl& simpl = static_cast<const ShaderImpl&>(*shader);

			m_refl.m_activeBindingMask[set] |= simpl.m_activeBindingMask[set];

			for(U32 i = 0; i < simpl.m_bindings[set].getSize(); ++i)
			{
				Bool bindingFound = false;
				for(U32 j = 0; j < counts[set]; ++j)
				{
					if(bindings[set][j].m_binding == simpl.m_bindings[set][i].m_binding)
					{
						// Found the binding

						ANKI_ASSERT(bindings[set][j].m_type == simpl.m_bindings[set][i].m_type);

						bindings[set][j].m_stageMask |= simpl.m_bindings[set][i].m_stageMask;

						bindingFound = true;
						break;
					}
				}

				if(!bindingFound)
				{
					// New binding

					bindings[set][counts[set]++] = simpl.m_bindings[set][i];
				}
			}

			if(simpl.m_pushConstantsSize > 0)
			{
				if(m_refl.m_pushConstantsSize > 0)
				{
					ANKI_ASSERT(m_refl.m_pushConstantsSize == simpl.m_pushConstantsSize);
				}

				m_refl.m_pushConstantsSize = max(m_refl.m_pushConstantsSize, simpl.m_pushConstantsSize);
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
		DescriptorSetLayoutInitInfo dsinf;
		dsinf.m_bindings = WeakArray<DescriptorBinding>((counts[set]) ? &bindings[set][0] : nullptr, counts[set]);

		ANKI_CHECK(
			getGrManagerImpl().getDescriptorSetFactory().newDescriptorSetLayout(dsinf, m_descriptorSetLayouts[set]));

		// Even if the dslayout is empty we will have to list it because we'll have to bind a DS for it.
		m_refl.m_descriptorSetMask.set(set);
	}

	// Create the ppline layout
	//
	WeakArray<DescriptorSetLayout> dsetLayouts((descriptorSetCount) ? &m_descriptorSetLayouts[0] : nullptr,
											   descriptorSetCount);
	ANKI_CHECK(getGrManagerImpl().getPipelineLayoutFactory().newPipelineLayout(dsetLayouts, m_refl.m_pushConstantsSize,
																			   m_pplineLayout));

	// Get some masks
	//
	const Bool graphicsProg = !!(m_stages & ShaderTypeBit::kAllGraphics);
	if(graphicsProg)
	{
		m_refl.m_attributeMask =
			static_cast<const ShaderImpl&>(*inf.m_graphicsShaders[ShaderType::kVertex]).m_attributeMask;

		m_refl.m_colorAttachmentWritemask =
			static_cast<const ShaderImpl&>(*inf.m_graphicsShaders[ShaderType::kFragment]).m_colorAttachmentWritemask;

		const U32 attachmentCount = m_refl.m_colorAttachmentWritemask.getEnabledBitCount();
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

			VkPipelineShaderStageCreateInfo& createInf =
				m_graphics.m_shaderCreateInfos[m_graphics.m_shaderCreateInfoCount++];
			createInf = {};
			createInf.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			createInf.stage = VkShaderStageFlagBits(convertShaderTypeBit(ShaderTypeBit(1 << shader->getShaderType())));
			createInf.pName = "main";
			createInf.module = shaderImpl.m_handle;
			createInf.pSpecializationInfo = shaderImpl.getSpecConstInfo();
		}
	}

	// Create the factory
	//
	if(graphicsProg)
	{
		m_graphics.m_pplineFactory = anki::newInstance<PipelineFactory>(getMemoryPool());
		m_graphics.m_pplineFactory->init(&getMemoryPool(), getGrManagerImpl().getDevice(),
										 getGrManagerImpl().getPipelineCache()
#if ANKI_PLATFORM_MOBILE
											 ,
										 getGrManagerImpl().getGlobalCreatePipelineMutex()
#endif
		);
	}

	// Create the pipeline if compute
	//
	if(!!(m_stages & ShaderTypeBit::kCompute))
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
		ci.stage.pSpecializationInfo = shaderImpl.getSpecConstInfo();

		ANKI_TRACE_SCOPED_EVENT(VK_PIPELINE_CREATE);
		ANKI_VK_CHECK(vkCreateComputePipelines(getDevice(), getGrManagerImpl().getPipelineCache(), 1, &ci, nullptr,
											   &m_compute.m_ppline));
		getGrManagerImpl().printPipelineShaderInfo(m_compute.m_ppline, getName(), ShaderTypeBit::kCompute);
	}

	// Create the RT pipeline
	//
	if(!!(m_stages & ShaderTypeBit::kAllRayTracing))
	{
		// Create shaders
		DynamicArrayRaii<VkPipelineShaderStageCreateInfo> stages(&getMemoryPool(), m_shaders.getSize());
		for(U32 i = 0; i < stages.getSize(); ++i)
		{
			const ShaderImpl& impl = static_cast<const ShaderImpl&>(*m_shaders[i]);

			VkPipelineShaderStageCreateInfo& stage = stages[i];
			stage = {};
			stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage.stage = VkShaderStageFlagBits(convertShaderTypeBit(ShaderTypeBit(1 << impl.getShaderType())));
			stage.pName = "main";
			stage.module = impl.m_handle;
			stage.pSpecializationInfo = impl.getSpecConstInfo();
		}

		// Create groups
		VkRayTracingShaderGroupCreateInfoKHR defaultGroup = {};
		defaultGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		defaultGroup.generalShader = VK_SHADER_UNUSED_KHR;
		defaultGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		defaultGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		defaultGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

		U32 groupCount = inf.m_rayTracingShaders.m_rayGenShaders.getSize()
						 + inf.m_rayTracingShaders.m_missShaders.getSize()
						 + inf.m_rayTracingShaders.m_hitGroups.getSize();
		DynamicArrayRaii<VkRayTracingShaderGroupCreateInfoKHR> groups(&getMemoryPool(), groupCount, defaultGroup);

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
				groups[groupCount].anyHitShader =
					*shaderUuidToMShadersIdx.find(inf.m_rayTracingShaders.m_hitGroups[i].m_anyHitShader->getUuid());
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
			ANKI_TRACE_SCOPED_EVENT(VK_PIPELINE_CREATE);
			ANKI_VK_CHECK(vkCreateRayTracingPipelinesKHR(
				getDevice(), VK_NULL_HANDLE, getGrManagerImpl().getPipelineCache(), 1, &ci, nullptr, &m_rt.m_ppline));
		}

		// Get RT handles
		const U32 handleArraySize =
			getGrManagerImpl().getPhysicalDeviceRayTracingProperties().shaderGroupHandleSize * groupCount;
		m_rt.m_allHandles.create(getMemoryPool(), handleArraySize, 0);
		ANKI_VK_CHECK(vkGetRayTracingShaderGroupHandlesKHR(getDevice(), m_rt.m_ppline, 0, groupCount, handleArraySize,
														   &m_rt.m_allHandles[0]));
	}

	return Error::kNone;
}

} // end namespace anki

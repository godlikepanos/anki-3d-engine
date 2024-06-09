// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkShaderProgram.h>
#include <AnKi/Gr/Vulkan/VkShader.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Gr/Vulkan/VkPipelineFactory.h>
#include <AnKi/Gr/BackendCommon/Functions.h>
#include <AnKi/Gr/Vulkan/VkBuffer.h>

#include <AnKi/ShaderCompiler/Dxc.h>
#include <ThirdParty/SpirvCross/spirv.hpp>

namespace anki {

/// Used to avoid keeping alive many shader modules that are essentially the same code. Mainly used to save memory because graphics ShaderPrograms
/// need to keep alive the shader modules for later when the pipeline is created.
class ShaderModuleFactory : public MakeSingletonSimple<ShaderModuleFactory>
{
public:
	~ShaderModuleFactory()
	{
		ANKI_ASSERT(m_entries.getSize() == 0 && "Forgot to release shader modules");
	}

	/// @note Thread-safe
	VkShaderModule getOrCreateShaderModule(ConstWeakArray<U32> spirv)
	{
		const U64 hash = computeHash(spirv.getBegin(), spirv.getSizeInBytes());

		LockGuard lock(m_mtx);

		Entry* entry = nullptr;
		for(Entry& e : m_entries)
		{
			if(e.m_hash == hash)
			{
				entry = &e;
				break;
			}
		}

		if(entry)
		{
			++entry->m_refcount;
			return entry->m_module;
		}
		else
		{
			VkShaderModuleCreateInfo ci = {};
			ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			ci.codeSize = spirv.getSizeInBytes();
			ci.pCode = spirv.getBegin();

			Entry entry;
			ANKI_VK_CHECKF(vkCreateShaderModule(getVkDevice(), &ci, nullptr, &entry.m_module));

			entry.m_hash = hash;
			m_entries.emplaceBack(entry);

			return entry.m_module;
		}
	}

	/// @note Thread-safe
	void releaseShaderModule(VkShaderModule smodule)
	{
		LockGuard lock(m_mtx);

		U32 idx = kMaxU32;
		for(U32 i = 0; i < m_entries.getSize(); ++i)
		{
			if(m_entries[i].m_module == smodule)
			{
				idx = i;
				break;
			}
		}

		ANKI_ASSERT(idx != kMaxU32);
		ANKI_ASSERT(m_entries[idx].m_refcount > 0);
		--m_entries[idx].m_refcount;
		if(m_entries[idx].m_refcount == 0)
		{
			vkDestroyShaderModule(getVkDevice(), m_entries[idx].m_module, nullptr);
			m_entries.erase(m_entries.getBegin() + idx);
		}
	}

private:
	class Entry
	{
	public:
		U64 m_hash = 0;
		VkShaderModule m_module = 0;
		U32 m_refcount = 1;
	};

	GrDynamicArray<Entry> m_entries;
	Mutex m_mtx;
};

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
	const Bool graphicsProg = !!(m_shaderTypes & ShaderTypeBit::kAllGraphics);
	if(graphicsProg)
	{
		for(const VkPipelineShaderStageCreateInfo& ci : m_graphics.m_shaderCreateInfos)
		{
			if(ci.module != 0)
			{
				ShaderModuleFactory::getSingleton().releaseShaderModule(ci.module);
			}
		}
	}

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
			ANKI_CHECK(ShaderReflection::linkShaderReflection(m_refl, simpl.m_reflection, m_refl));
		}

		m_refl.validate();
	}

	// Rewite SPIR-V to fix the bindings
	//
	GrDynamicArray<GrDynamicArray<U32>> rewrittenSpirvs;
	rewriteSpirv(m_refl.m_descriptor, rewrittenSpirvs);

	// Create the shader modules
	//
	const Bool graphicsProg = !!(m_shaderTypes & ShaderTypeBit::kAllGraphics);
	GrDynamicArray<VkShaderModule> shaderModules;
	shaderModules.resize(m_shaders.getSize());
	for(U32 ishader = 0; ishader < shaderModules.getSize(); ++ishader)
	{
		if(graphicsProg)
		{
			// Graphics prog, need to keep the modules alive for later
			shaderModules[ishader] = ShaderModuleFactory::getSingleton().getOrCreateShaderModule(rewrittenSpirvs[ishader]);
		}
		else
		{
			VkShaderModuleCreateInfo ci = {};
			ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			ci.codeSize = rewrittenSpirvs[ishader].getSizeInBytes();
			ci.pCode = rewrittenSpirvs[ishader].getBegin();

			ANKI_VK_CHECK(vkCreateShaderModule(getVkDevice(), &ci, nullptr, &shaderModules[ishader]));
		}
	}

	// Create the ppline layout
	//
	ANKI_CHECK(PipelineLayoutFactory2::getSingleton().getOrCreatePipelineLayout(m_refl.m_descriptor, m_pplineLayout));

	// Init the create infos
	//
	if(graphicsProg)
	{
		for(U32 ishader = 0; ishader < m_shaders.getSize(); ++ishader)
		{
			const ShaderImpl& shaderImpl = static_cast<const ShaderImpl&>(*m_shaders[ishader]);

			VkPipelineShaderStageCreateInfo& createInf = m_graphics.m_shaderCreateInfos[m_graphics.m_shaderCreateInfoCount++];
			createInf = {};
			createInf.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			createInf.stage = VkShaderStageFlagBits(convertShaderTypeBit(ShaderTypeBit(1 << shaderImpl.getShaderType())));
			createInf.pName = "main";
			createInf.module = shaderModules[ishader];
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
		VkComputePipelineCreateInfo ci = {};

		if(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kKHR_pipeline_executable_properties))
		{
			ci.flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR;
		}

		ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		ci.layout = m_pplineLayout->getHandle();

		ci.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		ci.stage.pName = "main";
		ci.stage.module = shaderModules[0];

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
			stage.module = shaderModules[i];
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
		ci.layout = m_pplineLayout->getHandle();

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

	// Non graphics programs have created their pipeline, destroy the shader modules
	//
	if(!graphicsProg)
	{
		for(VkShaderModule smodule : shaderModules)
		{
			vkDestroyShaderModule(getVkDevice(), smodule, nullptr);
		}
	}

	return Error::kNone;
}

void ShaderProgramImpl::rewriteSpirv(ShaderReflectionDescriptorRelated& refl, GrDynamicArray<GrDynamicArray<U32>>& rewrittenSpirvs)
{
	// Find a binding for the bindless DS
	if(refl.m_hasVkBindlessDescriptorSet)
	{
		for(U8 iset = 0; iset < kMaxDescriptorSets; ++iset)
		{
			if(refl.m_bindingCounts[iset] == 0)
			{
				refl.m_vkBindlessDescriptorSet = iset;
				break;
			}
		}
	}

	// Re-write all SPIRVs and compute the new bindings
	rewrittenSpirvs.resize(m_shaders.getSize());
	Bool hasBindless = false;
	Array<U16, kMaxDescriptorSets> vkBindingCount = {};
	for(U32 ishader = 0; ishader < m_shaders.getSize(); ++ishader)
	{
		ConstWeakArray<U32> inSpirv = static_cast<const ShaderImpl&>(*m_shaders[ishader]).m_spirvBin;
		GrDynamicArray<U32>& outSpv = rewrittenSpirvs[ishader];

		outSpv.resize(inSpirv.getSize());
		memcpy(outSpv.getBegin(), inSpirv.getBegin(), inSpirv.getSizeInBytes());

		visitSpirv(WeakArray<U32>(outSpv), [&](U32 cmd, WeakArray<U32> instructions) {
			if(cmd == spv::OpDecorate && instructions[1] == spv::DecorationBinding
			   && instructions[2] >= kDxcVkBindingShifts[0][HlslResourceType::kFirst]
			   && instructions[2] < kDxcVkBindingShifts[kMaxDescriptorSets - 1][HlslResourceType::kCount - 1])
			{
				const U32 binding = instructions[2];

				// Look at the binding and derive a few things. See the DXC compilation on what they mean
				U32 set = kMaxDescriptorSets;
				HlslResourceType hlslResourceType = HlslResourceType::kCount;
				for(set = 0; set < kMaxDescriptorSets; ++set)
				{
					for(HlslResourceType hlslResourceType_ : EnumIterable<HlslResourceType>())
					{
						if(binding >= kDxcVkBindingShifts[set][hlslResourceType_] && binding < kDxcVkBindingShifts[set][hlslResourceType_] + 1000)
						{
							hlslResourceType = hlslResourceType_;
							break;
						}
					}

					if(hlslResourceType != HlslResourceType::kCount)
					{
						break;
					}
				}

				ANKI_ASSERT(set < kMaxDescriptorSets);
				ANKI_ASSERT(hlslResourceType < HlslResourceType::kCount);
				const U32 registerBindingPoint = binding - kDxcVkBindingShifts[set][hlslResourceType];

				// Find the binding
				U32 foundBindingIdx = kMaxU32;
				for(U32 i = 0; i < refl.m_bindingCounts[set]; ++i)
				{
					const ShaderReflectionBinding& x = refl.m_bindings[set][i];
					if(x.m_registerBindingPoint == registerBindingPoint && hlslResourceType == descriptorTypeToHlslResourceType(x.m_type, x.m_flags))
					{
						ANKI_ASSERT(foundBindingIdx == kMaxU32);
						foundBindingIdx = i;
					}
				}

				// Rewrite it
				ANKI_ASSERT(foundBindingIdx != kMaxU32);
				if(refl.m_bindings[set][foundBindingIdx].m_vkBinding != kMaxU16)
				{
					// Binding was set in another shader, just rewrite the SPIR-V
					instructions[2] = refl.m_bindings[set][foundBindingIdx].m_vkBinding;
				}
				else
				{
					// Binding is new
					refl.m_bindings[set][foundBindingIdx].m_vkBinding = vkBindingCount[set];
					instructions[2] = vkBindingCount[set];
					++vkBindingCount[set];
				}
			}
			else if(cmd == spv::OpDecorate && instructions[1] == spv::DecorationDescriptorSet && instructions[2] == kDxcVkBindlessRegisterSpace)
			{
				// Bindless set, rewrite its set
				instructions[2] = refl.m_vkBindlessDescriptorSet;
				hasBindless = true;
			}
		});
	}

	ANKI_ASSERT(hasBindless == refl.m_hasVkBindlessDescriptorSet);
}

} // end namespace anki

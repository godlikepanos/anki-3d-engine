// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/ShaderProgramImpl.h>
#include <anki/gr/Shader.h>
#include <anki/gr/vulkan/ShaderImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/vulkan/Pipeline.h>

namespace anki
{

ShaderProgramImpl::ShaderProgramImpl(GrManager* manager)
	: VulkanObject(manager)
{
}

ShaderProgramImpl::~ShaderProgramImpl()
{
	if(m_pplineFactory)
	{
		m_pplineFactory->destroy();
		getAllocator().deleteInstance(m_pplineFactory);
	}

	if(m_computePpline)
	{
		vkDestroyPipeline(getDevice(), m_computePpline, nullptr);
	}
}

Error ShaderProgramImpl::init(const Array<ShaderPtr, U(ShaderType::COUNT)>& shaders)
{
	ShaderTypeBit shaderMask = ShaderTypeBit::NONE;
	m_shaders = shaders;

	// Merge bindings
	//
	Array2d<DescriptorBinding, MAX_DESCRIPTOR_SETS, MAX_BINDINGS_PER_DESCRIPTOR_SET> bindings;
	Array<U, MAX_DESCRIPTOR_SETS> counts = {};
	U descriptorSetCount = 0;
	for(U set = 0; set < MAX_DESCRIPTOR_SETS; ++set)
	{
		for(ShaderType stype = ShaderType::FIRST; stype < ShaderType::COUNT; ++stype)
		{
			if(!shaders[stype].isCreated())
			{
				continue;
			}

			shaderMask |= static_cast<ShaderTypeBit>(1 << stype);

			const ShaderImpl& simpl = *shaders[stype]->m_impl;

			m_refl.m_descriptorSetMask |= simpl.m_descriptorSetMask;
			m_refl.m_activeBindingMask[set] |= simpl.m_activeBindingMask[set];

			for(U i = 0; i < simpl.m_bindings[set].getSize(); ++i)
			{
				Bool bindingFound = false;
				for(U j = 0; j < counts[set]; ++j)
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
		}

		if(counts[set])
		{
			descriptorSetCount = set + 1;
		}
	}

	// Create the descriptor set layouts
	//
	for(U set = 0; set < descriptorSetCount; ++set)
	{
		DescriptorSetLayoutInitInfo inf;
		inf.m_bindings = WeakArray<DescriptorBinding>((counts[set]) ? &bindings[set][0] : nullptr, counts[set]);

		ANKI_CHECK(
			getGrManagerImpl().getDescriptorSetFactory().newDescriptorSetLayout(inf, m_descriptorSetLayouts[set]));
	}

	// Create the ppline layout
	//
	WeakArray<DescriptorSetLayout> dsetLayouts(
		(descriptorSetCount) ? &m_descriptorSetLayouts[0] : nullptr, descriptorSetCount);
	ANKI_CHECK(getGrManagerImpl().getPipelineLayoutFactory().newPipelineLayout(dsetLayouts, m_pplineLayout));

	// Get some masks
	//
	const Bool graphicsProg = !!(shaderMask & ShaderTypeBit::VERTEX);
	if(graphicsProg)
	{
		m_refl.m_attributeMask = shaders[ShaderType::VERTEX]->m_impl->m_attributeMask;
		m_refl.m_colorAttachmentWritemask = shaders[ShaderType::FRAGMENT]->m_impl->m_colorAttachmentWritemask;

		const U attachmentCount = m_refl.m_colorAttachmentWritemask.getEnabledBitCount();
		for(U i = 0; i < attachmentCount; ++i)
		{
			ANKI_ASSERT(m_refl.m_colorAttachmentWritemask.get(i) && "Should write to all attachments");
		}
	}

	// Init the create infos
	//
	if(graphicsProg)
	{
		for(ShaderType stype = ShaderType::VERTEX; stype <= ShaderType::FRAGMENT; ++stype)
		{
			if(!shaders[stype].isCreated())
			{
				continue;
			}

			VkPipelineShaderStageCreateInfo& inf = m_shaderCreateInfos[m_shaderCreateInfoCount++];
			inf = {};
			inf.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			inf.stage = convertShaderTypeBit(static_cast<ShaderTypeBit>(1 << stype));
			inf.pName = "main";
			inf.module = shaders[stype]->m_impl->m_handle;
		}
	}

	// Create the factory
	//
	if(graphicsProg)
	{
		m_pplineFactory = getAllocator().newInstance<PipelineFactory>();
		m_pplineFactory->init(
			getGrManagerImpl().getAllocator(), getGrManagerImpl().getDevice(), getGrManagerImpl().getPipelineCache());
	}

	// Create the pipeline if compute
	//
	if(!graphicsProg)
	{
		VkComputePipelineCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		ci.layout = m_pplineLayout.getHandle();

		ci.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		ci.stage.pName = "main";
		ci.stage.module = shaders[ShaderType::COMPUTE]->m_impl->m_handle;

		ANKI_VK_CHECK(vkCreateComputePipelines(
			getDevice(), getGrManagerImpl().getPipelineCache(), 1, &ci, nullptr, &m_computePpline));
	}

	return ErrorCode::NONE;
}

} // end namespace anki

// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/PipelineImpl.h>
#include <anki/gr/Pipeline.h>
#include <anki/gr/vulkan/ShaderImpl.h>

namespace anki
{

//==============================================================================
// Misc                                                                        =
//==============================================================================

/// Pre-filled VkGraphicsPipelineCreateInfo
class FilledGraphicsPipelineCreateInfo : public VkGraphicsPipelineCreateInfo
{
public:
	Array<VkPipelineShaderStageCreateInfo, U(ShaderType::COUNT) - 1> m_stages;
	VkPipelineVertexInputStateCreateInfo m_vertex;
	Array<VkVertexInputBindingDescription, MAX_VERTEX_ATTRIBUTES> m_bindings;
	Array<VkVertexInputAttributeDescription, MAX_VERTEX_ATTRIBUTES> m_attribs;
	VkPipelineInputAssemblyStateCreateInfo m_ia;
	VkPipelineTessellationStateCreateInfo m_tess;
	VkPipelineViewportStateCreateInfo m_vp;
	VkPipelineRasterizationStateCreateInfo m_rast;
	VkPipelineMultisampleStateCreateInfo m_ms;
	VkPipelineDepthStencilStateCreateInfo m_ds;
	VkPipelineColorBlendStateCreateInfo m_color;
	VkPipelineDynamicStateCreateInfo m_dyn;

	FilledGraphicsPipelineCreateInfo()
	{
		memset(this, 0, sizeof(*this));

		sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		for(VkPipelineShaderStageCreateInfo& stage : m_stages)
		{
			stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		}

		m_vertex.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		m_ia.sType =
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

		m_tess.sType =
			VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

		m_vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

		m_rast.sType =
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

		m_ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

		m_ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

		m_color.sType =
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

		m_dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	}
};

static const FilledGraphicsPipelineCreateInfo FILLED;

//==============================================================================
// PipelineImpl                                                                =
//==============================================================================

//==============================================================================
Error PipelineImpl::initGraphics(const PipelineInitInfo& init)
{
	FilledGraphicsPipelineCreateInfo ci = FILLED;

	ci.pStages = &ci.m_stages[0];
	initShaders(init, ci);

	ci.pVertexInputState = initVertexStage(init.m_vertex, ci.m_vertex);

	ci.layout = 0; // XXX
	ci.renderPass = 0; // XXX
	ci.basePipelineHandle = VK_NULL_HANDLE;

	return ErrorCode::NONE;
}

//==============================================================================
void PipelineImpl::initShaders(
	const PipelineInitInfo& init, VkGraphicsPipelineCreateInfo& ci)
{
	ci.stageCount = 0;

	for(ShaderType type = ShaderType::FIRST_GRAPHICS;
		type <= ShaderType::LAST_GRAPHICS;
		++type)
	{
		if(!init.m_shaders[type].isCreated())
		{
			continue;
		}

		ANKI_ASSERT(
			init.m_shaders[type]->getImplementation().m_shaderType == type);
		ANKI_ASSERT(init.m_shaders[type]->getImplementation().m_handle);

		VkPipelineShaderStageCreateInfo& stage =
			const_cast<VkPipelineShaderStageCreateInfo&>(
				ci.pStages[ci.stageCount]);
		ANKI_VK_MEMSET_DBG(stage);
		stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage.pNext = nullptr;
		stage.flags = 0;
		stage.stage = VkShaderStageFlagBits(1u << U(type));
		stage.module = init.m_shaders[type]->getImplementation().m_handle;
		stage.pName = "main";
		stage.pSpecializationInfo = nullptr;

		++ci.stageCount;
	}
}

//==============================================================================
VkPipelineVertexInputStateCreateInfo* PipelineImpl::initVertexStage(
	const VertexStateInfo& vertex, VkPipelineVertexInputStateCreateInfo& ci)
{
	if(vertex.m_bindingCount == 0 && vertex.m_attributeCount == 0)
	{
		// Early out
		return nullptr;
	}

	// First the bindings
	ci.vertexBindingDescriptionCount = vertex.m_bindingCount;
	for(U i = 0; i < ci.vertexBindingDescriptionCount; ++i)
	{
		VkVertexInputBindingDescription& vkBinding =
			const_cast<VkVertexInputBindingDescription&>(
				ci.pVertexBindingDescriptions[i]);

		vkBinding.binding = i;
		vkBinding.stride = vertex.m_bindings[i].m_stride;

		switch(vertex.m_bindings[i].m_stepRate)
		{
		case VertexStepRate::VERTEX:
			vkBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			break;
		case VertexStepRate::INSTANCE:
			vkBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
			break;
		default:
			ANKI_ASSERT(0);
		}
	}

	// Then the attributes
	ci.vertexAttributeDescriptionCount = vertex.m_attributeCount;
	for(U i = 0; i < ci.vertexAttributeDescriptionCount; ++i)
	{
		VkVertexInputAttributeDescription& vkAttrib =
			const_cast<VkVertexInputAttributeDescription&>(
				ci.pVertexAttributeDescriptions[i]);

		vkAttrib.location = 0;
		vkAttrib.binding = vertex.m_attributes[i].m_binding;
		vkAttrib.format = convertFormat(vertex.m_attributes[i].m_format);
		vkAttrib.offset = vertex.m_attributes[i].m_offset;
	}

	return &ci;
}

//==============================================================================
VkPipelineInputAssemblyStateCreateInfo* PipelineImpl::initInputAssemblyState(
	const InputAssemblerStateInfo& ia,
	VkPipelineInputAssemblyStateCreateInfo& ci)
{
	// TODO
	return &ci;
}

//==============================================================================
Error PipelineImpl::init(const PipelineInitInfo& init)
{

	return ErrorCode::NONE;
}

} // end namespace anki

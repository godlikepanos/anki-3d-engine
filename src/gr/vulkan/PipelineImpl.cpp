// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/PipelineImpl.h>
#include <anki/gr/Pipeline.h>
#include <anki/gr/vulkan/ShaderImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

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
	Array<VkPipelineColorBlendAttachmentState, MAX_COLOR_ATTACHMENTS>
		m_attachments;
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
		m_color.pAttachments = &m_attachments[0];

		m_dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	}
};

static const FilledGraphicsPipelineCreateInfo FILLED;

//==============================================================================
// PipelineImpl                                                                =
//==============================================================================

//==============================================================================
PipelineImpl::~PipelineImpl()
{
	if(m_handle)
	{
		vkDestroyPipeline(getDevice(), m_handle, nullptr);
	}
}

//==============================================================================
void PipelineImpl::initGraphics(const PipelineInitInfo& init)
{
	FilledGraphicsPipelineCreateInfo ci = FILLED;

	ci.pStages = &ci.m_stages[0];
	initShaders(init, ci);

	// Init sub-states
	ci.pVertexInputState = initVertexStage(init.m_vertex, ci.m_vertex);
	ci.pInputAssemblyState =
		initInputAssemblyState(init.m_inputAssembler, ci.m_ia);
	ci.pTessellationState =
		initTessellationState(init.m_tessellation, ci.m_tess);
	ci.pViewportState = initViewportState(ci.m_vp);
	ci.pRasterizationState = initRasterizerState(init.m_rasterizer, ci.m_rast);
	ci.pMultisampleState = initMsState(ci.m_ms);
	ci.pDepthStencilState = initDsState(init.m_depthStencil, ci.m_ds);
	ci.pColorBlendState = initColorState(init.m_color, ci.m_color);
	ci.pDynamicState = nullptr; // No dynamic state as static at the moment

	// Finalize
	ci.layout = getGrManagerImpl().getGlobalPipelineLayout();
	ci.renderPass = getGrManagerImpl().getOrCreateCompatibleRenderPass(init);
	ci.basePipelineHandle = VK_NULL_HANDLE;

	ANKI_VK_CHECKF(vkCreateGraphicsPipelines(
		getDevice(), nullptr, 1, &ci, nullptr, &m_handle));
}

//==============================================================================
void PipelineImpl::initCompute(const PipelineInitInfo& init)
{
	VkComputePipelineCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	ci.pNext = nullptr;
	ci.layout = getGrManagerImpl().getGlobalPipelineLayout();
	ci.basePipelineHandle = VK_NULL_HANDLE;

	VkPipelineShaderStageCreateInfo& stage = ci.stage;
	stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage.pNext = nullptr;
	stage.flags = 0;
	stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stage.module =
		init.m_shaders[ShaderType::COMPUTE]->getImplementation().m_handle;
	stage.pName = "main";
	stage.pSpecializationInfo = nullptr;

	ANKI_VK_CHECKF(vkCreateComputePipelines(
		getDevice(), nullptr, 1, &ci, nullptr, &m_handle));
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
	ci.topology = convertTopology(ia.m_topology);
	ci.primitiveRestartEnable = ia.m_primitiveRestartEnabled;

	return &ci;
}

//==============================================================================
VkPipelineTessellationStateCreateInfo* PipelineImpl::initTessellationState(
	const TessellationStateInfo& t, VkPipelineTessellationStateCreateInfo& ci)
{
	ci.patchControlPoints = t.m_patchControlPointCount;
	return &ci;
}

//==============================================================================
VkPipelineViewportStateCreateInfo* PipelineImpl::initViewportState(
	VkPipelineViewportStateCreateInfo& ci)
{
	// Viewport is dynamic
	return nullptr;
}

//==============================================================================
VkPipelineRasterizationStateCreateInfo* PipelineImpl::initRasterizerState(
	const RasterizerStateInfo& r, VkPipelineRasterizationStateCreateInfo& ci)
{
	ci.depthClampEnable = VK_FALSE;
	ci.rasterizerDiscardEnable = VK_FALSE;
	ci.polygonMode = convertFillMode(r.m_fillMode);
	ci.cullMode = convertCullMode(r.m_cullMode);
	ci.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	ci.depthBiasEnable = VK_FALSE; // Dynamic

	return &ci;
}

//==============================================================================
VkPipelineMultisampleStateCreateInfo* PipelineImpl::initMsState(
	VkPipelineMultisampleStateCreateInfo& ci)
{
	// No MS for now
	return nullptr;
}

//==============================================================================
VkPipelineDepthStencilStateCreateInfo* PipelineImpl::initDsState(
	const DepthStencilStateInfo& ds, VkPipelineDepthStencilStateCreateInfo& ci)
{
	ci.depthTestEnable = ds.m_depthCompareFunction != CompareOperation::ALWAYS
		|| ds.m_depthWriteEnabled;
	ci.depthWriteEnable = ds.m_depthWriteEnabled;
	ci.depthCompareOp = convertCompareOp(ds.m_depthCompareFunction);
	ci.depthBoundsTestEnable = VK_FALSE;
	ci.stencilTestEnable = VK_FALSE; // For now no stencil

	return &ci;
}

//==============================================================================
VkPipelineColorBlendStateCreateInfo* PipelineImpl::initColorState(
	const ColorStateInfo& c, VkPipelineColorBlendStateCreateInfo& ci)
{
	ci.logicOpEnable = VK_FALSE;
	ci.attachmentCount = c.m_attachmentCount;

	for(U i = 0; i < ci.attachmentCount; ++i)
	{
		VkPipelineColorBlendAttachmentState& out =
			const_cast<VkPipelineColorBlendAttachmentState&>(
				ci.pAttachments[i]);
		const ColorAttachmentStateInfo& in = c.m_attachments[i];
		out.blendEnable = !(in.m_srcBlendMethod == BlendMethod::ONE
			&& in.m_dstBlendMethod == BlendMethod::ZERO);
		out.srcColorBlendFactor = convertBlendMethod(in.m_srcBlendMethod);
		out.dstColorBlendFactor = convertBlendMethod(in.m_dstBlendMethod);
		out.colorBlendOp = convertBlendFunc(in.m_blendFunction);
		out.srcAlphaBlendFactor = out.srcColorBlendFactor;
		out.dstAlphaBlendFactor = out.dstColorBlendFactor;
		out.alphaBlendOp = out.colorBlendOp;

		out.colorWriteMask = convertColorWriteMask(in.m_channelWriteMask);
	}

	return &ci;
}

//==============================================================================
void PipelineImpl::init(const PipelineInitInfo& init)
{
	if(init.m_shaders[ShaderType::COMPUTE].isCreated())
	{
		initCompute(init);
	}
	else
	{
		initGraphics(init);
	}
}

} // end namespace anki

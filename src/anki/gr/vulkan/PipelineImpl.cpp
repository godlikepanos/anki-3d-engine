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
	Array<VkPipelineColorBlendAttachmentState, MAX_COLOR_ATTACHMENTS> m_attachments;
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

		m_vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		m_ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		m_tess.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		m_vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		m_rast.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		m_ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		m_ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		m_color.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		m_dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	}
};

static const FilledGraphicsPipelineCreateInfo FILLED;

PipelineImpl::~PipelineImpl()
{
	if(m_handle)
	{
		vkDestroyPipeline(getDevice(), m_handle, nullptr);
	}
}

Error PipelineImpl::initGraphics(const PipelineInitInfo& init)
{
	FilledGraphicsPipelineCreateInfo ci = FILLED;
	ci.m_vertex.pVertexBindingDescriptions = &ci.m_bindings[0];
	ci.m_vertex.pVertexAttributeDescriptions = &ci.m_attribs[0];
	ci.m_color.pAttachments = &ci.m_attachments[0];
	ci.pStages = &ci.m_stages[0];

	initShaders(init, ci);

	// Init sub-states
	ci.pVertexInputState = initVertexStage(init.m_vertex, ci.m_vertex);
	ci.pInputAssemblyState = initInputAssemblyState(init.m_inputAssembler, ci.m_ia);
	ci.pTessellationState = initTessellationState(init.m_tessellation, ci.m_tess);
	ci.pViewportState = initViewportState(ci.m_vp);
	ci.pRasterizationState = initRasterizerState(init.m_rasterizer, ci.m_rast);
	ci.pMultisampleState = initMsState(ci.m_ms);
	ci.pDepthStencilState = initDsState(init.m_depthStencil, ci.m_ds);
	ci.pColorBlendState = initColorState(init.m_color, ci.m_color);
	ci.pDynamicState = initDynamicState(ci.m_dyn);

	// Finalize
	ci.layout = getGrManagerImpl().getGlobalPipelineLayout();
	ci.renderPass = getGrManagerImpl().getCompatibleRenderPassCreator().getOrCreateCompatibleRenderPass(init);
	ci.basePipelineHandle = VK_NULL_HANDLE;

	ANKI_VK_CHECK(
		vkCreateGraphicsPipelines(getDevice(), getGrManagerImpl().getPipelineCache(), 1, &ci, nullptr, &m_handle));

	return ErrorCode::NONE;
}

Error PipelineImpl::initCompute(const PipelineInitInfo& init)
{
	VkComputePipelineCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	ci.layout = getGrManagerImpl().getGlobalPipelineLayout();
	ci.basePipelineHandle = VK_NULL_HANDLE;

	VkPipelineShaderStageCreateInfo& stage = ci.stage;
	stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stage.module = init.m_shaders[ShaderType::COMPUTE]->getImplementation().m_handle;
	stage.pName = "main";
	stage.pSpecializationInfo = nullptr;

	ANKI_VK_CHECK(
		vkCreateComputePipelines(getDevice(), getGrManagerImpl().getPipelineCache(), 1, &ci, nullptr, &m_handle));

	return ErrorCode::NONE;
}

void PipelineImpl::initShaders(const PipelineInitInfo& init, VkGraphicsPipelineCreateInfo& ci)
{
	ci.stageCount = 0;

	for(ShaderType type = ShaderType::FIRST_GRAPHICS; type <= ShaderType::LAST_GRAPHICS; ++type)
	{
		if(!init.m_shaders[type].isCreated())
		{
			continue;
		}

		ANKI_ASSERT(init.m_shaders[type]->getImplementation().m_shaderType == type);
		ANKI_ASSERT(init.m_shaders[type]->getImplementation().m_handle);

		VkPipelineShaderStageCreateInfo& stage =
			const_cast<VkPipelineShaderStageCreateInfo&>(ci.pStages[ci.stageCount]);

		stage.stage = VkShaderStageFlagBits(1u << U(type));
		stage.module = init.m_shaders[type]->getImplementation().m_handle;
		stage.pName = "main";

		++ci.stageCount;
	}
}

VkPipelineVertexInputStateCreateInfo* PipelineImpl::initVertexStage(
	const VertexStateInfo& vertex, VkPipelineVertexInputStateCreateInfo& ci)
{
	// First the bindings
	ci.vertexBindingDescriptionCount = vertex.m_bindingCount;
	for(U i = 0; i < ci.vertexBindingDescriptionCount; ++i)
	{
		VkVertexInputBindingDescription& vkBinding =
			const_cast<VkVertexInputBindingDescription&>(ci.pVertexBindingDescriptions[i]);

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
			const_cast<VkVertexInputAttributeDescription&>(ci.pVertexAttributeDescriptions[i]);

		vkAttrib.location = i;
		vkAttrib.binding = vertex.m_attributes[i].m_binding;
		vkAttrib.format = convertFormat(vertex.m_attributes[i].m_format);
		vkAttrib.offset = vertex.m_attributes[i].m_offset;
	}

	return &ci;
}

VkPipelineInputAssemblyStateCreateInfo* PipelineImpl::initInputAssemblyState(
	const InputAssemblerStateInfo& ia, VkPipelineInputAssemblyStateCreateInfo& ci)
{
	ci.topology = convertTopology(ia.m_topology);
	ci.primitiveRestartEnable = ia.m_primitiveRestartEnabled;

	return &ci;
}

VkPipelineTessellationStateCreateInfo* PipelineImpl::initTessellationState(
	const TessellationStateInfo& t, VkPipelineTessellationStateCreateInfo& ci)
{
	ci.patchControlPoints = t.m_patchControlPointCount;
	return &ci;
}

VkPipelineViewportStateCreateInfo* PipelineImpl::initViewportState(VkPipelineViewportStateCreateInfo& ci)
{
	// Viewport an scissor is dynamic
	ci.viewportCount = 1;
	ci.scissorCount = 1;

	return &ci;
}

VkPipelineRasterizationStateCreateInfo* PipelineImpl::initRasterizerState(
	const RasterizerStateInfo& r, VkPipelineRasterizationStateCreateInfo& ci)
{
	ci.depthClampEnable = VK_FALSE;
	ci.rasterizerDiscardEnable = VK_FALSE;
	ci.polygonMode = convertFillMode(r.m_fillMode);
	ci.cullMode = convertCullMode(r.m_cullMode);
	ci.frontFace = VK_FRONT_FACE_CLOCKWISE; // Use CW to workaround Vulkan's y flip
	ci.depthBiasEnable = VK_TRUE;
	ci.lineWidth = 1.0;

	return &ci;
}

VkPipelineMultisampleStateCreateInfo* PipelineImpl::initMsState(VkPipelineMultisampleStateCreateInfo& ci)
{
	ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	return &ci;
}

VkPipelineDepthStencilStateCreateInfo* PipelineImpl::initDsState(
	const DepthStencilStateInfo& ds, VkPipelineDepthStencilStateCreateInfo& ci)
{
	VkPipelineDepthStencilStateCreateInfo* out = nullptr;

	if(ds.isInUse())
	{
		ci.depthTestEnable = (ds.m_depthCompareFunction != CompareOperation::ALWAYS) || ds.m_depthWriteEnabled;
		ci.depthWriteEnable = ds.m_depthWriteEnabled;
		ci.depthCompareOp = convertCompareOp(ds.m_depthCompareFunction);
		ci.depthBoundsTestEnable = VK_FALSE;

		ci.stencilTestEnable = !(stencilTestDisabled(ds.m_stencilFront) && stencilTestDisabled(ds.m_stencilBack));
		ci.front.failOp = convertStencilOp(ds.m_stencilFront.m_stencilFailOperation);
		ci.front.passOp = convertStencilOp(ds.m_stencilFront.m_stencilPassDepthPassOperation);
		ci.front.depthFailOp = convertStencilOp(ds.m_stencilFront.m_stencilPassDepthFailOperation);
		ci.front.compareOp = convertCompareOp(ds.m_stencilFront.m_compareFunction);
		ci.back.failOp = convertStencilOp(ds.m_stencilBack.m_stencilFailOperation);
		ci.back.passOp = convertStencilOp(ds.m_stencilBack.m_stencilPassDepthPassOperation);
		ci.back.depthFailOp = convertStencilOp(ds.m_stencilBack.m_stencilPassDepthFailOperation);
		ci.back.compareOp = convertCompareOp(ds.m_stencilBack.m_compareFunction);

		out = &ci;
	}

	return out;
}

VkPipelineColorBlendStateCreateInfo* PipelineImpl::initColorState(
	const ColorStateInfo& c, VkPipelineColorBlendStateCreateInfo& ci)
{
	ci.logicOpEnable = VK_FALSE;
	ci.attachmentCount = c.m_attachmentCount;

	for(U i = 0; i < ci.attachmentCount; ++i)
	{
		VkPipelineColorBlendAttachmentState& out = const_cast<VkPipelineColorBlendAttachmentState&>(ci.pAttachments[i]);
		const ColorAttachmentStateInfo& in = c.m_attachments[i];

		out.blendEnable = !(in.m_srcBlendMethod == BlendMethod::ONE && in.m_dstBlendMethod == BlendMethod::ZERO);
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

VkPipelineDynamicStateCreateInfo* PipelineImpl::initDynamicState(VkPipelineDynamicStateCreateInfo& ci)
{
	// Almost all state is dynamic
	static const Array<VkDynamicState, 8> DYN = {{VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_DEPTH_BIAS,
		VK_DYNAMIC_STATE_BLEND_CONSTANTS,
		VK_DYNAMIC_STATE_DEPTH_BOUNDS,
		VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
		VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
		VK_DYNAMIC_STATE_STENCIL_REFERENCE}};

	ci.dynamicStateCount = DYN.getSize();
	ci.pDynamicStates = &DYN[0];

	return &ci;
}

Error PipelineImpl::init(const PipelineInitInfo& init)
{
	if(init.m_shaders[ShaderType::COMPUTE].isCreated())
	{
		m_bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
		return initCompute(init);
	}
	else
	{
		m_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		return initGraphics(init);
	}

	return ErrorCode::NONE;
}

} // end namespace anki

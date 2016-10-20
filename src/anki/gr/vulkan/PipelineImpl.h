// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/ResourceGroupExtra.h>

namespace anki
{

// Forward
class VertexStateInfo;
class InputAssemblerStateInfo;
class TessellationStateInfo;
class ViewportStateInfo;
class RasterizerStateInfo;
class DepthStencilStateInfo;
class ColorStateInfo;

/// @addtogroup vulkan
/// @{

/// Program pipeline.
class PipelineImpl : public VulkanObject
{
public:
	VkPipeline m_handle = VK_NULL_HANDLE;
	VkPipelineBindPoint m_bindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	Array<DescriptorSetLayoutInfo, MAX_BOUND_RESOURCE_GROUPS> m_descriptorSetLayoutInfos;
	U8 m_descriptorSetMask = 0;

	PipelineImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~PipelineImpl();

	ANKI_USE_RESULT Error init(const PipelineInitInfo& init);

private:
	ANKI_USE_RESULT Error initGraphics(const PipelineInitInfo& init);

	ANKI_USE_RESULT Error initCompute(const PipelineInitInfo& init);

	void initShaders(const PipelineInitInfo& init, VkGraphicsPipelineCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineVertexInputStateCreateInfo* initVertexStage(
		const VertexStateInfo& vertex, VkPipelineVertexInputStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineInputAssemblyStateCreateInfo* initInputAssemblyState(
		const InputAssemblerStateInfo& ia, VkPipelineInputAssemblyStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineTessellationStateCreateInfo* initTessellationState(
		const TessellationStateInfo& t, VkPipelineTessellationStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineViewportStateCreateInfo* initViewportState(VkPipelineViewportStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineRasterizationStateCreateInfo* initRasterizerState(
		const RasterizerStateInfo& r, VkPipelineRasterizationStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineMultisampleStateCreateInfo* initMsState(VkPipelineMultisampleStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineDepthStencilStateCreateInfo* initDsState(
		const DepthStencilStateInfo& ds, VkPipelineDepthStencilStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineColorBlendStateCreateInfo* initColorState(
		const ColorStateInfo& c, VkPipelineColorBlendStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineDynamicStateCreateInfo* initDynamicState(VkPipelineDynamicStateCreateInfo& ci);

	ANKI_USE_RESULT Error createPplineLayout(const PipelineInitInfo& init);
};
/// @}

} // end namespace anki

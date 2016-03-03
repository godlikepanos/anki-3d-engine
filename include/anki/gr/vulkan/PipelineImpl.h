// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>

namespace anki
{

// Forward
class VertexStateInfo;
class InputAssemblerStateInfo;

/// @addtogroup vulkan
/// @{

/// Program pipeline.
class PipelineImpl : public VulkanObject
{
public:
	VkPipeline m_handle = VK_NULL_HANDLE;

	PipelineImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~PipelineImpl();

	ANKI_USE_RESULT Error init(const PipelineInitInfo& init);

private:
	ANKI_USE_RESULT Error initGraphics(const PipelineInitInfo& init);

	void initShaders(
		const PipelineInitInfo& init, VkGraphicsPipelineCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineVertexInputStateCreateInfo* initVertexStage(
		const VertexStateInfo& vertex,
		VkPipelineVertexInputStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineInputAssemblyStateCreateInfo*
	initInputAssemblyState(const InputAssemblerStateInfo& ia,
		VkPipelineInputAssemblyStateCreateInfo& ci);
};
/// @}

} // end namespace anki

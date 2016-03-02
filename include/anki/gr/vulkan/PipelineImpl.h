// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Program pipeline.
class PipelineImpl : public VulkanObject
{
public:
	VkPipeline m_ppline = VK_NULL_HANDLE;

	PipelineImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~PipelineImpl();

	ANKI_USE_RESULT Error init(const PipelineInitInfo& init);
};
/// @}

} // end namespace anki

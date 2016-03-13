// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Vulkan implementation of GrManager.
class GrManagerImpl
{
public:
	VkInstance m_instance = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	VkQueue m_queue = VK_NULL_HANDLE;

	GrManagerImpl(GrManager* manager)
		: m_manager(manager)
	{
		ANKI_ASSERT(manager);
	}

	~GrManagerImpl();

	/// Get or create a compatible render pass for a pipeline.
	VkRenderPass getOrCreateCompatibleRenderPass(const PipelineInitInfo& init);

private:
	GrManager* m_manager = nullptr;
	GrAllocator<U8> m_alloc;

	/// Map for compatible render passes.
	class CompatibleRenderPassHashMap;
	CompatibleRenderPassHashMap* m_renderPasses = nullptr;
};
/// @}

} // end namespace anki

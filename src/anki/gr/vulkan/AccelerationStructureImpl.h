// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/AccelerationStructure.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/GpuMemoryManager.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// AccelerationStructure implementation.
class AccelerationStructureImpl final : public AccelerationStructure,
										public VulkanObject<AccelerationStructure, AccelerationStructureImpl>
{
public:
	AccelerationStructureImpl(GrManager* manager, CString name)
		: AccelerationStructure(manager, name)
	{
	}

	~AccelerationStructureImpl();

	ANKI_USE_RESULT Error init(const AccelerationStructureInitInfo& inf);

	VkAccelerationStructureKHR getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

private:
	VkAccelerationStructureKHR m_handle = VK_NULL_HANDLE;
	GpuMemoryHandle m_memHandle;
};
/// @}

} // end namespace anki

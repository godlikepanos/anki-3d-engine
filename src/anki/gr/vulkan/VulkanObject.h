// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// The base class for all Vulkan object implementations.
class VulkanObject
{
public:
	VulkanObject(GrManager* manager)
		: m_manager(manager)
	{
		ANKI_ASSERT(manager);
	}

	virtual ~VulkanObject()
	{
	}

	VkDevice getDevice() const;

	GrAllocator<U8> getAllocator() const;

	GrManagerImpl& getGrManagerImpl();

	const GrManagerImpl& getGrManagerImpl() const;

	GrManager& getGrManager()
	{
		ANKI_ASSERT(m_manager);
		return *m_manager;
	}

protected:
	GrManager* m_manager = nullptr;
};
/// @}

} // end namespace anki

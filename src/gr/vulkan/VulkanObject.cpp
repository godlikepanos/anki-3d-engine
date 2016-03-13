// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

//==============================================================================
VkDevice VulkanObject::getDevice() const
{
	return m_manager->getImplementation().m_device;
}

} // end namespace anki

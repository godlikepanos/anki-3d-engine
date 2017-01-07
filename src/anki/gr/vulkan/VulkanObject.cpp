// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

VkDevice VulkanObject::getDevice() const
{
	return m_manager->getImplementation().getDevice();
}

GrAllocator<U8> VulkanObject::getAllocator() const
{
	return m_manager->getAllocator();
}

GrManagerImpl& VulkanObject::getGrManagerImpl()
{
	return m_manager->getImplementation();
}

const GrManagerImpl& VulkanObject::getGrManagerImpl() const
{
	return m_manager->getImplementation();
}

} // end namespace anki

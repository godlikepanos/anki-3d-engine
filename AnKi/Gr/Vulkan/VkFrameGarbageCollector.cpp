// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkFrameGarbageCollector.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Gr/Vulkan/VkDescriptor.h>

namespace anki {

TextureGarbage::~TextureGarbage()
{
	const VkDevice dev = getVkDevice();

	for(VkImageView viewHandle : m_viewHandles)
	{
		vkDestroyImageView(dev, viewHandle, nullptr);
	}

	for(U32 bindlessIndex : m_bindlessIndices)
	{
		BindlessDescriptorSet::getSingleton().unbindTexture(bindlessIndex);
	}

	if(m_imageHandle)
	{
		vkDestroyImage(dev, m_imageHandle, nullptr);
	}

	if(m_memoryHandle)
	{
		GpuMemoryManager::getSingleton().freeMemory(m_memoryHandle);
	}
}

BufferGarbage::~BufferGarbage()
{
	const VkDevice dev = getVkDevice();

	for(VkBufferView view : m_viewHandles)
	{
		vkDestroyBufferView(dev, view, nullptr);
	}

	if(m_bufferHandle)
	{
		vkDestroyBuffer(dev, m_bufferHandle, nullptr);
	}

	if(m_memoryHandle)
	{
		GpuMemoryManager::getSingleton().freeMemory(m_memoryHandle);
	}
}

ASGarbage::~ASGarbage()
{
	vkDestroyAccelerationStructureKHR(getGrManagerImpl().getDevice(), m_asHandle, nullptr);
}

} // end namespace anki

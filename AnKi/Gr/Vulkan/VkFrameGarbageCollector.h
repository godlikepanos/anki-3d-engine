// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkCommon.h>
#include <AnKi/Gr/Vulkan/VkGpuMemoryManager.h>
#include <AnKi/Gr/BackendCommon/FrameGarbageCollector.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// @memberof FrameGarbageCollector
class TextureGarbage : public IntrusiveListEnabled<TextureGarbage>
{
public:
	GrDynamicArray<VkImageView> m_viewHandles;
	GrDynamicArray<U32> m_bindlessIndices;
	VkImage m_imageHandle = VK_NULL_HANDLE;
	GpuMemoryHandle m_memoryHandle;

	~TextureGarbage();
};

/// @memberof FrameGarbageCollector
class BufferGarbage : public IntrusiveListEnabled<BufferGarbage>
{
public:
	VkBuffer m_bufferHandle = VK_NULL_HANDLE;
	GrDynamicArray<VkBufferView> m_viewHandles;
	GpuMemoryHandle m_memoryHandle;

	~BufferGarbage();
};

/// AS have more data (buffers) that build them but don't bother storing them since buffers will be automatically garbage collected as well.
/// @memberof FrameGarbageCollector
class ASGarbage : public IntrusiveListEnabled<ASGarbage>
{
public:
	VkAccelerationStructureKHR m_asHandle = VK_NULL_HANDLE;

	~ASGarbage();
};

class VulkanFrameGarbageCollector :
	public FrameGarbageCollector<TextureGarbage, BufferGarbage, ASGarbage>,
	public MakeSingleton<VulkanFrameGarbageCollector>
{
};
/// @}

} // end namespace anki

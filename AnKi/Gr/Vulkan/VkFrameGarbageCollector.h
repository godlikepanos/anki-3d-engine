// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkCommon.h>
#include <AnKi/Gr/Vulkan/VkGpuMemoryManager.h>
#include <AnKi/Gr/Vulkan/VkFenceFactory.h>
#include <AnKi/Util/DynamicArray.h>
#include <AnKi/Util/List.h>

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
};

/// @memberof FrameGarbageCollector
class BufferGarbage : public IntrusiveListEnabled<BufferGarbage>
{
public:
	VkBuffer m_bufferHandle = VK_NULL_HANDLE;
	GrDynamicArray<VkBufferView> m_viewHandles;
	GpuMemoryHandle m_memoryHandle;
};

/// AS have more data (buffers) that build them but don't bother storing them since buffers will be automatically garbage collected as well.
/// @memberof FrameGarbageCollector
class ASGarbage : public IntrusiveListEnabled<ASGarbage>
{
public:
	VkAccelerationStructureKHR m_asHandle = VK_NULL_HANDLE;
};

/// This class gathers various garbages and disposes them when in some later frame where it is safe to do so. This is used on bindless textures and
/// buffers where we have to wait until the frame where they were deleted is done.
class FrameGarbageCollector
{
public:
	FrameGarbageCollector() = default;

	~FrameGarbageCollector();

	void init()
	{
#if ANKI_EXTRA_CHECKS
		m_initialized = true;
#endif
	}

	void destroy();

	/// Sets a new frame and collects garbage as well.
	/// @note It's thread-safe.
	void setNewFrame(MicroFencePtr frameFence);

	/// @note It's thread-safe.
	void newTextureGarbage(TextureGarbage* textureGarbage);

	/// @note It's thread-safe.
	void newBufferGarbage(BufferGarbage* bufferGarbage);

	/// @note It's thread-safe.
	void newASGarbage(ASGarbage* garbage);

private:
	class FrameGarbage : public IntrusiveListEnabled<FrameGarbage>
	{
	public:
		IntrusiveList<TextureGarbage> m_textureGarbage;
		IntrusiveList<BufferGarbage> m_bufferGarbage;
		IntrusiveList<ASGarbage> m_asGarbage;
		MicroFencePtr m_fence;
	};

	Mutex m_mtx;
	IntrusiveList<FrameGarbage> m_frames;

#if ANKI_EXTRA_CHECKS
	Bool m_initialized = false;
#endif

	void collectGarbage();

	FrameGarbage& getFrame();
};
/// @}

} // end namespace anki

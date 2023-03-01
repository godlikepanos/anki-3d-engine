// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/Common.h>
#include <AnKi/Gr/Vulkan/GpuMemoryManager.h>
#include <AnKi/Gr/Vulkan/FenceFactory.h>
#include <AnKi/Util/DynamicArray.h>
#include <AnKi/Util/List.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// @memberof FrameGarbageCollector
class TextureGarbage : public IntrusiveListEnabled<TextureGarbage>
{
public:
	DynamicArray<VkImageView> m_viewHandles;
	DynamicArray<U32> m_bindlessIndices;
	VkImage m_imageHandle = VK_NULL_HANDLE;
	GpuMemoryHandle m_memoryHandle;
};

/// @memberof FrameGarbageCollector
class BufferGarbage : public IntrusiveListEnabled<BufferGarbage>
{
public:
	VkBuffer m_bufferHandle = VK_NULL_HANDLE;
	DynamicArray<VkBufferView> m_viewHandles;
	GpuMemoryHandle m_memoryHandle;
};

/// This class gathers various garbages and disposes them when in some later frame where it is safe to do so. This is
/// used on bindless textures and buffers where we have to wait until the frame where they were deleted is done.
class FrameGarbageCollector
{
public:
	FrameGarbageCollector() = default;

	~FrameGarbageCollector();

	void init(GrManagerImpl* gr)
	{
		m_gr = gr;
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

private:
	class FrameGarbage : public IntrusiveListEnabled<FrameGarbage>
	{
	public:
		IntrusiveList<TextureGarbage> m_textureGarbage;
		IntrusiveList<BufferGarbage> m_bufferGarbage;
		MicroFencePtr m_fence;
	};

	GrManagerImpl* m_gr = nullptr;

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

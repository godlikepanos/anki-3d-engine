// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/FrameGarbageCollector.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Gr/Fence.h>

namespace anki {

FrameGarbageCollector::~FrameGarbageCollector()
{
	destroy();
}

void FrameGarbageCollector::collectGarbage()
{
	if(ANKI_LIKELY(m_frames.isEmpty()))
	{
		return;
	}

	const VkDevice dev = m_gr->getDevice();
	GrAllocator<U8> alloc = m_gr->getAllocator();

	IntrusiveList<FrameGarbage> newFrames;
	while(!m_frames.isEmpty())
	{
		FrameGarbage& frame = *m_frames.popFront();

		if(frame.m_fence.isCreated() && !frame.m_fence->done())
		{
			ANKI_ASSERT(!frame.m_textureGarbage.isEmpty() || !frame.m_bufferGarbage.isEmpty());
			newFrames.pushBack(&frame);
			continue;
		}

		// Frame is done, dispose garbage and destroy it

		// Dispose texture garbage
		while(!frame.m_textureGarbage.isEmpty())
		{
			TextureGarbage* textureGarbage = frame.m_textureGarbage.popBack();

			for(VkImageView viewHandle : textureGarbage->m_viewHandles)
			{
				vkDestroyImageView(dev, viewHandle, nullptr);
			}
			textureGarbage->m_viewHandles.destroy(alloc);

			for(U32 bindlessIndex : textureGarbage->m_bindlessIndices)
			{
				m_gr->getDescriptorSetFactory().unbindBindlessTexture(bindlessIndex);
			}
			textureGarbage->m_bindlessIndices.destroy(alloc);

			if(textureGarbage->m_imageHandle)
			{
				vkDestroyImage(dev, textureGarbage->m_imageHandle, nullptr);
			}

			if(textureGarbage->m_memoryHandle)
			{
				m_gr->getGpuMemoryManager().freeMemory(textureGarbage->m_memoryHandle);
			}

			alloc.deleteInstance(textureGarbage);
		}

		// Dispose buffer garbage
		while(!frame.m_bufferGarbage.isEmpty())
		{
			BufferGarbage* bufferGarbage = frame.m_bufferGarbage.popBack();

			for(VkBufferView view : bufferGarbage->m_viewHandles)
			{
				vkDestroyBufferView(dev, view, nullptr);
			}
			bufferGarbage->m_viewHandles.destroy(alloc);

			if(bufferGarbage->m_bufferHandle)
			{
				vkDestroyBuffer(dev, bufferGarbage->m_bufferHandle, nullptr);
			}

			if(bufferGarbage->m_memoryHandle)
			{
				m_gr->getGpuMemoryManager().freeMemory(bufferGarbage->m_memoryHandle);
			}

			alloc.deleteInstance(bufferGarbage);
		}

		alloc.deleteInstance(&frame);
	}

	m_frames = std::move(newFrames);
}

FrameGarbageCollector::FrameGarbage& FrameGarbageCollector::getFrame()
{
	if(!m_frames.isEmpty() && !m_frames.getBack().m_fence.isCreated())
	{
		// Do nothing
	}
	else
	{
		FrameGarbage* newGarbage = m_gr->getAllocator().newInstance<FrameGarbage>();
		m_frames.pushBack(newGarbage);
	}

	return m_frames.getBack();
}

void FrameGarbageCollector::setNewFrame(MicroFencePtr frameFence)
{
	ANKI_ASSERT(frameFence.isCreated());

	LockGuard<Mutex> lock(m_mtx);
	ANKI_ASSERT(m_initialized);

	if(!m_frames.isEmpty() && !m_frames.getBack().m_fence.isCreated())
	{
		// Last frame is without a fence, asign the fence to not not have it garbage collected
		m_frames.getBack().m_fence = std::move(frameFence);
	}

	collectGarbage();
}

void FrameGarbageCollector::newTextureGarbage(TextureGarbage* textureGarbage)
{
	ANKI_ASSERT(textureGarbage);
	LockGuard<Mutex> lock(m_mtx);
	ANKI_ASSERT(m_initialized);
	FrameGarbage& frame = getFrame();
	frame.m_textureGarbage.pushBack(textureGarbage);
}

void FrameGarbageCollector::newBufferGarbage(BufferGarbage* bufferGarbage)
{
	ANKI_ASSERT(bufferGarbage);
	LockGuard<Mutex> lock(m_mtx);
	ANKI_ASSERT(m_initialized);
	FrameGarbage& frame = getFrame();
	frame.m_bufferGarbage.pushBack(bufferGarbage);
}

void FrameGarbageCollector::destroy()
{
	LockGuard<Mutex> lock(m_mtx);

	collectGarbage();
	ANKI_ASSERT(m_frames.isEmpty());

#if ANKI_EXTRA_CHECKS
	m_initialized = false;
#endif
}

} // end namespace anki

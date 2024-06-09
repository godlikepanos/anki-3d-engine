// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DFrameGarbageCollector.h>

namespace anki {

FrameGarbageCollector::~FrameGarbageCollector()
{
	collectGarbage();
	ANKI_ASSERT(m_frames.isEmpty());
}

void FrameGarbageCollector::endFrame(MicroFence* frameFence)
{
	LockGuard lock(m_mtx);

	if(!m_frames.isEmpty() && !m_frames.getBack().m_fence.isCreated())
	{
		// Last frame is without a fence, asign the fence to not have it garbage collected
		m_frames.getBack().m_fence.reset(frameFence);
	}

	collectGarbage();
}

FrameGarbageCollector::FrameGarbage& FrameGarbageCollector::getFrame()
{
	if(!m_frames.isEmpty() && !m_frames.getBack().m_fence.isCreated())
	{
		// Do nothing
	}
	else
	{
		FrameGarbage* newGarbage = newInstance<FrameGarbage>(GrMemoryPool::getSingleton());
		m_frames.pushBack(newGarbage);
	}

	return m_frames.getBack();
}

void FrameGarbageCollector::newTextureGarbage(TextureGarbage* textureGarbage)
{
	LockGuard<Mutex> lock(m_mtx);
	FrameGarbage& frame = getFrame();
	frame.m_textureGarbage.pushBack(textureGarbage);
}

void FrameGarbageCollector::newBufferGarbage(BufferGarbage* garbage)
{
	LockGuard<Mutex> lock(m_mtx);
	FrameGarbage& frame = getFrame();
	frame.m_bufferGarbage.pushBack(garbage);
}

void FrameGarbageCollector::collectGarbage()
{
	if(m_frames.isEmpty()) [[likely]]
	{
		return;
	}

	IntrusiveList<FrameGarbage> newFrames;
	while(!m_frames.isEmpty())
	{
		FrameGarbage& frame = *m_frames.popFront();

		if(frame.m_fence.isCreated() && !frame.m_fence->done())
		{
			ANKI_ASSERT(!frame.m_textureGarbage.isEmpty());
			newFrames.pushBack(&frame);
			continue;
		}

		// Frame is done, dispose garbage and destroy it

		// Dispose texture garbage
		while(!frame.m_textureGarbage.isEmpty())
		{
			TextureGarbage* textureGarbage = frame.m_textureGarbage.popBack();

			for(U32 i = 0; i < textureGarbage->m_descriptorHeapHandles.getSize(); ++i)
			{
				DescriptorFactory::getSingleton().freePersistent(textureGarbage->m_descriptorHeapHandles[i]);
			}

			safeRelease(textureGarbage->m_resource);

			deleteInstance(GrMemoryPool::getSingleton(), textureGarbage);
		}

		// Dispose buffer garbage
		while(!frame.m_bufferGarbage.isEmpty())
		{
			BufferGarbage* garbage = frame.m_bufferGarbage.popBack();

			safeRelease(garbage->m_resource);

			deleteInstance(GrMemoryPool::getSingleton(), garbage);
		}

		deleteInstance(GrMemoryPool::getSingleton(), &frame);
	}

	m_frames = std::move(newFrames);
}

} // end namespace anki

// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/BackendCommon/Common.h>
#include <AnKi/Util/Thread.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// This class gathers various garbages and disposes them when in some later frame where it is safe to do so. This is used on bindless textures and
/// buffers where we have to wait until the frame where they were deleted is done.
template<typename TTextureGarbage, typename TBufferGarbage, typename TASGarbage>
class FrameGarbageCollector
{
public:
	FrameGarbageCollector() = default;

	~FrameGarbageCollector()
	{
		destroy();
	}

	void destroy()
	{
		for(FrameGarbage& frame : m_frames)
		{
			collectGarbage(frame);
		}
	}

	/// Sets a new frame and collects garbage as well.
	/// @note It's thread-safe.
	void beginFrameAndCollectGarbage()
	{
		LockGuard lock(m_mtx);
		m_frame = (m_frame + 1) % m_frames.getSize();
		FrameGarbage& frame = m_frames[m_frame];
		collectGarbage(frame);
	}

	/// @note It's thread-safe.
	void newTextureGarbage(TTextureGarbage* textureGarbage)
	{
		ANKI_ASSERT(textureGarbage);
		LockGuard lock(m_mtx);
		FrameGarbage& frame = m_frames[m_frame];
		frame.m_textureGarbage.pushBack(textureGarbage);
	}

	/// @note It's thread-safe.
	void newBufferGarbage(TBufferGarbage* bufferGarbage)
	{
		ANKI_ASSERT(bufferGarbage);
		LockGuard lock(m_mtx);
		FrameGarbage& frame = m_frames[m_frame];
		frame.m_bufferGarbage.pushBack(bufferGarbage);
	}

	/// @note It's thread-safe.
	void newASGarbage(TASGarbage* garbage)
	{
		ANKI_ASSERT(garbage);
		LockGuard lock(m_mtx);
		FrameGarbage& frame = m_frames[m_frame];
		frame.m_asGarbage.pushBack(garbage);
	}

private:
	class FrameGarbage
	{
	public:
		IntrusiveList<TTextureGarbage> m_textureGarbage;
		IntrusiveList<TBufferGarbage> m_bufferGarbage;
		IntrusiveList<TASGarbage> m_asGarbage;
	};

	Mutex m_mtx;
	Array<FrameGarbage, kMaxFramesInFlight> m_frames;
	U32 m_frame = 0;

	static void collectGarbage(FrameGarbage& frame)
	{
		while(!frame.m_textureGarbage.isEmpty())
		{
			deleteInstance(GrMemoryPool::getSingleton(), frame.m_textureGarbage.popBack());
		}

		while(!frame.m_bufferGarbage.isEmpty())
		{
			deleteInstance(GrMemoryPool::getSingleton(), frame.m_bufferGarbage.popBack());
		}

		while(!frame.m_asGarbage.isEmpty())
		{
			deleteInstance(GrMemoryPool::getSingleton(), frame.m_asGarbage.popBack());
		}
	}
};
/// @}

} // end namespace anki

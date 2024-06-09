// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/D3D/D3DDescriptor.h>
#include <AnKi/Gr/D3D/D3DFence.h>
#include <AnKi/Util/List.h>

namespace anki {

/// @addtogroup directx
/// @{

/// @memberof FrameGarbageCollector
class TextureGarbage : public IntrusiveListEnabled<TextureGarbage>
{
public:
	GrDynamicArray<DescriptorHeapHandle> m_descriptorHeapHandles;
	ID3D12Resource* m_resource = nullptr;
};

/// @memberof FrameGarbageCollector
class BufferGarbage : public IntrusiveListEnabled<BufferGarbage>
{
public:
	ID3D12Resource* m_resource = nullptr;
};

/// This class gathers various garbages and disposes them when in some later frame where it is safe to do so. This is used on bindless textures and
/// buffers where we have to wait until the frame where they were deleted is done.
class FrameGarbageCollector : public MakeSingleton<FrameGarbageCollector>
{
public:
	FrameGarbageCollector() = default;

	FrameGarbageCollector(const FrameGarbageCollector&) = delete;

	~FrameGarbageCollector();

	FrameGarbageCollector& operator=(const FrameGarbageCollector&) = delete;

	/// @note It's thread-safe.
	void newTextureGarbage(TextureGarbage* textureGarbage);

	/// @note It's thread-safe.
	void newBufferGarbage(BufferGarbage* garbage);

	/// Finalizes a frame.
	/// @note It's thread-safe.
	void endFrame(MicroFence* frameFence);

private:
	class FrameGarbage : public IntrusiveListEnabled<FrameGarbage>
	{
	public:
		IntrusiveList<TextureGarbage> m_textureGarbage;
		IntrusiveList<BufferGarbage> m_bufferGarbage;
		MicroFencePtr m_fence;
	};

	Mutex m_mtx;
	IntrusiveList<FrameGarbage> m_frames;

	void collectGarbage();

	FrameGarbage& getFrame();
};
/// @}

} // end namespace anki

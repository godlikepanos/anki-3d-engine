// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

// Copies the swapchain to a buffer
class ScreenshotPass : public RendererObject
{
public:
	~ScreenshotPass();

	Error init();

	void populateRenderGraph();

	void setFramesToCapture(U32 framesToCaptureCount)
	{
		m_framesToCaptureCount = framesToCaptureCount;
	}

	ANKI_INTERNAL void setFence(Fence* fence);

private:
	static constexpr U32 kFramesInFlightCount = 4;

	RendererShaderProgram m_prog;

	U32 m_framesToCaptureCount = 0;

	Array<BufferPtr, kFramesInFlightCount> m_buffers;
	Array<void*, kFramesInFlightCount> m_mappedMemories = {};
	Array<FencePtr, kFramesInFlightCount> m_fences;
	Array<U64, kFramesInFlightCount> m_rendererFrames = {};
	U32 m_localFrame = 0;

	Error saveTexture(U32 localFrame) const;
};

} // end namespace anki

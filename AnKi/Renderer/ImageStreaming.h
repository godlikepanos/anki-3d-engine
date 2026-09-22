// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/Readback.h>

namespace anki {

// Handles image streaming requests
class ImageStreaming : public RendererObject
{
public:
	Error init();

	void populateRenderGraphPreRendering();

	void populateRenderGraphPostRendering();

	BufferHandle getFeedbackBufferHandle() const
	{
		return m_runCtx.m_feedbackBuffHandle;
	}

	BufferView getFeedbackBuffer() const
	{
		return m_runCtx.m_feedbackBuff;
	}

private:
	RendererShaderProgram m_clearFeedbackBufferProg;
	RendererShaderProgram m_storeRequestsProg;

	MultiframeReadbackToken m_readback;

	class
	{
	public:
		BufferHandle m_feedbackBuffHandle;
		BufferView m_feedbackBuff;
	} m_runCtx;
};

} // namespace anki

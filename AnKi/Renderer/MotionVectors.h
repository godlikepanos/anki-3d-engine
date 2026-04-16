// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr.h>
#include <AnKi/Renderer/RendererObject.h>

namespace anki {

// Motion vector calculation.
class MotionVectors : public RendererObject
{
public:
	MotionVectors()
	{
		registerDebugRenderTarget("MotionVectors");
		registerDebugRenderTarget("AdjMotionVectors&Disocclusion");
	}

	Error init();

	void populateRenderGraph();

	RenderTargetHandle getMotionVectorsRt() const
	{
		return m_runCtx.m_motionVectorsRtHandle;
	}

	RenderTargetHandle getAdjustedMotionVectorsRt() const
	{
		return m_runCtx.m_adjustedMotionVectorsRtHandle;
	}

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  [[maybe_unused]] DebugRenderTargetDrawStyle& drawStyle) const override
	{
		handles[0] = (rtName == "MotionVectors") ? m_runCtx.m_motionVectorsRtHandle : m_runCtx.m_adjustedMotionVectorsRtHandle;
	}

private:
	RendererShaderProgram m_grProg;
	RenderTargetDesc m_motionVectorsRtDesc;
	RenderTargetDesc m_adjustedMotionVectorsRtDesc;

	class
	{
	public:
		RenderTargetHandle m_motionVectorsRtHandle;
		RenderTargetHandle m_adjustedMotionVectorsRtHandle;
	} m_runCtx;
};

} // end namespace anki

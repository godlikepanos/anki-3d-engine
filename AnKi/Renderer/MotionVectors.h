// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr.h>
#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Motion vector calculation.
class MotionVectors : public RendererObject
{
public:
	MotionVectors()
	{
		registerDebugRenderTarget("MotionVectors");
	}

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getMotionVectorsRt() const
	{
		return m_runCtx.m_motionVectorsRtHandle;
	}

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] Array<DebugRenderTargetDrawStyle, kMaxDebugRenderTargets>& drawStyles) const override
	{
		handles[0] = m_runCtx.m_motionVectorsRtHandle;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	RenderTargetDesc m_motionVectorsRtDescr;

	class
	{
	public:
		RenderTargetHandle m_motionVectorsRtHandle;
	} m_runCtx;
};
/// @}

} // end namespace anki

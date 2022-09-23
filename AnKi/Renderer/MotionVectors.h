// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
	MotionVectors(Renderer* renderer)
		: RendererObject(renderer)
	{
		registerDebugRenderTarget("MotionVectors");
		registerDebugRenderTarget("MotionVectorsHistoryLength");
	}

	~MotionVectors();

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getMotionVectorsRt() const
	{
		return m_runCtx.m_motionVectorsRtHandle;
	}

	RenderTargetHandle getHistoryLengthRt() const
	{
		return m_runCtx.m_historyLengthWriteRtHandle;
	}

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const override
	{
		if(rtName == "MotionVectors")
		{
			handles[0] = m_runCtx.m_motionVectorsRtHandle;
		}
		else
		{
			ANKI_ASSERT(rtName == "MotionVectorsHistoryLength");
			handles[0] = m_runCtx.m_historyLengthWriteRtHandle;
		}
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	RenderTargetDescription m_motionVectorsRtDescr;
	FramebufferDescription m_fbDescr;

	Array<TexturePtr, 2> m_historyLengthTextures;
	Bool m_historyLengthTexturesImportedOnce = false;

	class
	{
	public:
		RenderTargetHandle m_motionVectorsRtHandle;
		RenderTargetHandle m_historyLengthReadRtHandle;
		RenderTargetHandle m_historyLengthWriteRtHandle;
	} m_runCtx;

	void run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);

	Error initInternal();
};
/// @}

} // end namespace anki

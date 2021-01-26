// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Gr.h>
#include <anki/renderer/RendererObject.h>

namespace anki
{

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
		registerDebugRenderTarget("MotionVectorsRejection");
	}

	~MotionVectors();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getMotionVectorsRt() const
	{
		return m_runCtx.m_motionVectorsRtHandle;
	}

	RenderTargetHandle getRejectionFactorRt() const
	{
		return m_runCtx.m_rejectionFactorRtHandle;
	}

	void getDebugRenderTarget(CString rtName, RenderTargetHandle& handle) const override
	{
		if(rtName == "MotionVectors")
		{
			handle = m_runCtx.m_motionVectorsRtHandle;
		}
		else
		{
			ANKI_ASSERT(rtName == "MotionVectorsRejection");
			handle = m_runCtx.m_rejectionFactorRtHandle;
		}
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	RenderTargetDescription m_motionVectorsRtDescr;
	RenderTargetDescription m_rejectionFactorRtDescr;

	class
	{
	public:
		RenderingContext* m_ctx = nullptr;
		RenderTargetHandle m_motionVectorsRtHandle;
		RenderTargetHandle m_rejectionFactorRtHandle;
	} m_runCtx;

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki

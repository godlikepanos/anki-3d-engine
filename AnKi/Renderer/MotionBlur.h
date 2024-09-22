// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline NumericCVar<U32> g_motionBlurTileSizeCVar("R", "MotionBlurTileSize", 32, 8, 64, "Motion blur tile size");
inline NumericCVar<U32> g_motionBlurSampleCountCVar("R", "MotionBlurSampleCount", 16, 0, 64, "Motion blur sample count");

/// Motion blur.
class MotionBlur : public RendererObject
{
public:
	MotionBlur()
	{
		registerDebugRenderTarget("MotionBlur");
	}

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const override
	{
		ANKI_ASSERT(rtName == "MotionBlur");
		handles[0] = m_runCtx.m_rt;
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_maxVelocityGrProg;
	ShaderProgramPtr m_maxNeightbourVelocityGrProg;
	ShaderProgramPtr m_reconstructGrProg;

	RenderTargetDesc m_maxVelocityRtDesc;
	RenderTargetDesc m_maxNeighbourVelocityRtDesc;
	RenderTargetDesc m_finalRtDesc;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;
};
/// @}

} // end namespace anki

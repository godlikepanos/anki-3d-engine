// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Gr.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline NumericCVar<U32> g_ssrStepIncrementCVar("R", "SsrStepIncrement", 32, 1, 256, "The number of steps for each loop");
inline NumericCVar<U32> g_ssrMaxIterationsCVar("R", "SsrMaxIterations", 64, 1, 256, "Max SSR raymarching loop iterations");
inline NumericCVar<F32> g_ssrRoughnessCutoffCVar("R", "SsrRoughnessCutoff", (ANKI_PLATFORM_MOBILE) ? 0.7f : 1.0f, 0.0f, 1.0f,
												 "Materials with roughness higher that this value will fallback to probe reflections");
inline BoolCVar g_ssrQuarterResolutionCVar("R", "SsrQuarterResolution", ANKI_PLATFORM_MOBILE);

/// Screen space reflections.
class Ssr : public RendererObject
{
public:
	Ssr()
	{
		registerDebugRenderTarget("Ssr");
	}

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const override
	{
		ANKI_ASSERT(rtName == "Ssr");
		handles[0] = m_runCtx.m_ssrRt;
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_ssrRt;
	}

public:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_ssrGrProg;

	RenderTargetDesc m_ssrRtDescr;

	class
	{
	public:
		RenderTargetHandle m_ssrRt;
	} m_runCtx;

	Error initInternal();
};
/// @}

} // end namespace anki

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

ANKI_CVAR(BoolCVar, Render, ShadowmapResolveQuarterRez, ANKI_PLATFORM_MOBILE, "Shadowmapping resolve quality")

/// Resolves shadowmaps into a single texture.
class ShadowmapsResolve : public RendererObject
{
public:
	ShadowmapsResolve()
	{
		registerDebugRenderTarget("ResolvedShadows");
	}

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
							  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const override
	{
		ANKI_ASSERT(rtName == "ResolvedShadows");
		handles[0] = m_runCtx.m_rt;
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

public:
	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 3> m_grProgs;
	RenderTargetDesc m_rtDescr;
	Bool m_quarterRez = false;
	ImageResourcePtr m_noiseImage;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;

	void run(RenderPassWorkContext& rgraphCtx, RenderingContext& ctx);
};
/// @}

} // namespace anki

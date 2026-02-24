// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

ANKI_CVAR2(NumericCVar<U32>, Render, VolumetricLightingAccumulation, SubdivisionXY, 2u, 1u, 16u,
		   "The original clusters will be split using this CVar")
ANKI_CVAR2(NumericCVar<U32>, Render, VolumetricLightingAccumulation, SubdivisionZ, 2u, 1u, 16u, "The original clusters will be split using this CVar")
ANKI_CVAR2(NumericCVar<U32>, Render, VolumetricLightingAccumulation, FinalZSplit, 26, 1, 256,
		   "Final cluster split that will recieve volumetric lights")
ANKI_CVAR2(BoolCVar, Render, VolumetricLightingAccumulation, Debug, false, "Enable debugging of volumetrics")

// Volumetric lighting. It accumulates lighting in a volume texture.
class VolumetricLightingAccumulation : public RendererObject
{
public:
	VolumetricLightingAccumulation()
	{
		registerDebugRenderTarget("Volumetric Lighting");
	}

	Error init();

	void populateRenderGraph();

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rts[1];
	}

	void fillClustererConstants(ClustererConstants& consts);

	void setEnableDebuggingView(Bool enable)
	{
		m_debugResult = enable;
	}

	Bool getDebuggingView() const
	{
		return m_debugResult;
	}

	void getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  DebugRenderTargetDrawStyle& drawStyle) const override;

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	ShaderProgramPtr m_debugGrProg;

	Array<RendererTexture, 2> m_rtTextures;
	ImageResourcePtr m_noiseImage;

	RenderTargetDesc m_debugRtDesc;

	UVec3 m_volumeSize;
	Bool m_debugResult = false;

	class
	{
	public:
		Array<RenderTargetHandle, 2> m_rts;
		RenderTargetHandle m_debugRt;
	} m_runCtx; // Runtime context.
};

} // end namespace anki

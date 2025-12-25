// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/TraditionalDeferredShading.h>
#include <AnKi/Collision/Forward.h>

namespace anki {

ANKI_CVAR2(NumericCVar<U32>, Render, Idp, TileResolution, (ANKI_PLATFORM_MOBILE) ? 16 : 32, 8, 32, "GI tile resolution")
ANKI_CVAR2(NumericCVar<U32>, Render, Idp, ShadowMapResolution, 128, 4, 2048, "GI shadowmap resolution")

// Ambient global illumination passes. It builds a volume clipmap with ambient GI information.
class IndirectDiffuseProbes : public RendererObject
{
public:
	Error init();

	void populateRenderGraph();

	RenderTargetHandle getCurrentlyRefreshedVolumeRt() const
	{
		ANKI_ASSERT(m_runCtx.m_probeVolumeHandle.isValid());
		return m_runCtx.m_probeVolumeHandle;
	}

	Bool hasCurrentlyRefreshedVolumeRt() const
	{
		return m_runCtx.m_probeVolumeHandle.isValid();
	}

private:
	class
	{
	public:
		Array<RenderTargetDesc, kGBufferColorRenderTargetCount> m_colorRtDescrs;
		RenderTargetDesc m_depthRtDescr;
	} m_gbuffer; // G-buffer pass.

	class
	{
	public:
		RenderTargetDesc m_rtDescr;
	} m_shadowMapping;

	class LS
	{
	public:
		RenderTargetDesc m_rtDescr;
		TraditionalDeferredLightShading m_deferred;
	} m_lightShading; // Light shading.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_irradiance; // Irradiance.

	static constexpr U32 kProbeCellRefreshesPerFrame = 2;

	U32 m_tileSize = 0;

	class
	{
	public:
		RenderTargetHandle m_probeVolumeHandle;
	} m_runCtx;

	Error initInternal();
	Error initGBuffer();
	Error initShadowMapping();
	Error initLightShading();
	Error initIrradiance();
};

} // end namespace anki

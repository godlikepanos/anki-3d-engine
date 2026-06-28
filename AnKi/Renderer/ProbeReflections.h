// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/TraditionalDeferredShading.h>

namespace anki {

ANKI_CVAR2(NumericCVar<U32>, Render, ProbeReflections, ShadowMapResolution, 64, 4, 2048, "Reflection probe shadow resolution")

// Probe reflections.
class ProbeReflections : public RendererObject
{
	friend class IrTask;

public:
	Error init();

	void populateRenderGraph();

	U getReflectionTextureMipmapCount() const
	{
		return m_lightShading.m_mipCount;
	}

	RenderTargetHandle getCurrentlyRefreshedReflectionRt() const
	{
		ANKI_ASSERT(m_runCtx.m_probeTex.isValid());
		return m_runCtx.m_probeTex;
	}

	Bool getHasCurrentlyRefreshedReflectionRt() const
	{
		return m_runCtx.m_probeTex.isValid();
	}

	// The pre-integrated specular environment BRDF (DFG) LUT
	Texture& getIntegrationLut() const
	{
		return *m_integrationLut;
	}

private:
	class
	{
	public:
		U32 m_tileSize = 0;
		Array<RenderTargetDesc, kGBufferColorRenderTargetCount> m_colorRtDescrs;
		RenderTargetDesc m_depthRtDescr;
	} m_gbuffer; // G-buffer pass.

	class LS
	{
	public:
		U32 m_tileSize = 0;
		U8 m_mipCount = 0;
		TraditionalDeferredLightShading m_deferred;
	} m_lightShading; // Light shading.

	class
	{
	public:
		RenderTargetDesc m_rtDescr;
	} m_shadowMapping;

	class
	{
	public:
		RenderTargetHandle m_probeTex;
	} m_runCtx;

	// Pre-integrated specular environment BRDF (DFG) LUT.
	RendererTexture m_integrationLut;

	Error initIntegrationLut();
};

} // end namespace anki

// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/TraditionalDeferredShading.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline NumericCVar<U32> g_probeReflectionIrradianceResolutionCVar("R", "ProbeReflectionIrradianceResolution", 16, 4, 2048,
																  "Reflection probe irradiance resolution");
inline NumericCVar<U32> g_probeReflectionShadowMapResolutionCVar("R", "ProbeReflectionShadowMapResolution", 64, 4, 2048,
																 "Reflection probe shadow resolution");

/// Probe reflections.
class ProbeReflections : public RendererObject
{
	friend class IrTask;

public:
	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	U getReflectionTextureMipmapCount() const
	{
		return m_lightShading.m_mipCount;
	}

	Texture& getIntegrationLut() const
	{
		return m_integrationLut->getTexture();
	}

	SamplerPtr getIntegrationLutSampler() const
	{
		return m_integrationLutSampler;
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

private:
	class
	{
	public:
		U32 m_tileSize = 0;
		Array<RenderTargetDesc, kGBufferColorRenderTargetCount> m_colorRtDescrs;
		RenderTargetDesc m_depthRtDescr;
	} m_gbuffer; ///< G-buffer pass.

	class LS
	{
	public:
		U32 m_tileSize = 0;
		U8 m_mipCount = 0;
		TraditionalDeferredLightShading m_deferred;
	} m_lightShading; ///< Light shading.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		BufferPtr m_diceValuesBuff;
		U32 m_workgroupSize = 16;
	} m_irradiance; ///< Irradiance.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_irradianceToRefl; ///< Apply irradiance back to the reflection.

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

	// Other
	ImageResourcePtr m_integrationLut;
	SamplerPtr m_integrationLutSampler;

	Error initInternal();
	Error initGBuffer();
	Error initLightShading();
	Error initIrradiance();
	Error initIrradianceToRefl();
	Error initShadowMapping();
};
/// @}

} // end namespace anki

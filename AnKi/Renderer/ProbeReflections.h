// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/TraditionalDeferredShading.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

// Forward
class GpuVisibilityOutput;

/// @addtogroup renderer
/// @{

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

	TextureView& getIntegrationLut() const
	{
		return m_integrationLut->getTextureView();
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
		Array<RenderTargetDescription, kGBufferColorRenderTargetCount> m_colorRtDescrs;
		RenderTargetDescription m_depthRtDescr;
	} m_gbuffer; ///< G-buffer pass.

	class LS
	{
	public:
		U32 m_tileSize = 0;
		U32 m_mipCount = 0;
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
		RenderTargetDescription m_rtDescr;
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

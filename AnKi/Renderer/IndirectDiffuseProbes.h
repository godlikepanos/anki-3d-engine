// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/TraditionalDeferredShading.h>
#include <AnKi/Collision/Forward.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Ambient global illumination passes.
///
/// It builds a volume clipmap with ambient GI information.
class IndirectDiffuseProbes : public RendererObject
{
public:
	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

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
		Array<RenderTargetDescription, kGBufferColorRenderTargetCount> m_colorRtDescrs;
		RenderTargetDescription m_depthRtDescr;
	} m_gbuffer; ///< G-buffer pass.

	class
	{
	public:
		RenderTargetDescription m_rtDescr;
	} m_shadowMapping;

	class LS
	{
	public:
		RenderTargetDescription m_rtDescr;
		TraditionalDeferredLightShading m_deferred;
	} m_lightShading; ///< Light shading.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_irradiance; ///< Irradiance.

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
/// @}

} // end namespace anki

// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/TraditionalDeferredShading.h>
#include <AnKi/Renderer/RenderQueue.h>
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
	IndirectDiffuseProbes(Renderer* r)
		: RendererObject(r)
		, m_lightShading(r)
	{
	}

	~IndirectDiffuseProbes();

	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getCurrentlyRefreshedVolumeRt() const;

	Bool hasCurrentlyRefreshedVolumeRt() const;

private:
	class InternalContext;

	class
	{
	public:
		Array<RenderTargetDescription, kGBufferColorRenderTargetCount> m_colorRtDescrs;
		RenderTargetDescription m_depthRtDescr;
		FramebufferDescription m_fbDescr;
	} m_gbuffer; ///< G-buffer pass.

	class
	{
	public:
		RenderTargetDescription m_rtDescr;
		FramebufferDescription m_fbDescr;
	} m_shadowMapping;

	class LS
	{
	public:
		RenderTargetDescription m_rtDescr;
		FramebufferDescription m_fbDescr;
		TraditionalDeferredLightShading m_deferred;

		LS(Renderer* r)
			: m_deferred(r)
		{
		}
	} m_lightShading; ///< Light shading.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_irradiance; ///< Irradiance.

	InternalContext* m_giCtx = nullptr;
	U32 m_tileSize = 0;

	Error initInternal();
	Error initGBuffer();
	Error initShadowMapping();
	Error initLightShading();
	Error initIrradiance();

	void runGBufferInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const;
	void runShadowmappingInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const;
	void runLightShading(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx);
	void runIrradiance(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx);
};
/// @}

} // end namespace anki

// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/TraditionalDeferredShading.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Probe reflections.
class ProbeReflections : public RendererObject
{
	friend class IrTask;

public:
	ProbeReflections(Renderer* r);

	~ProbeReflections();

	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	U getReflectionTextureMipmapCount() const
	{
		return m_lightShading.m_mipCount;
	}

	TextureViewPtr getIntegrationLut() const
	{
		return m_integrationLut->getTextureView();
	}

	SamplerPtr getIntegrationLutSampler() const
	{
		return m_integrationLutSampler;
	}

	RenderTargetHandle getReflectionRt() const
	{
		return m_ctx.m_lightShadingRt;
	}

private:
	class
	{
	public:
		U32 m_tileSize = 0;
		Array<RenderTargetDescription, kGBufferColorRenderTargetCount> m_colorRtDescrs;
		RenderTargetDescription m_depthRtDescr;
		FramebufferDescription m_fbDescr;
	} m_gbuffer; ///< G-buffer pass.

	class LS
	{
	public:
		U32 m_tileSize = 0;
		U32 m_mipCount = 0;
		TexturePtr m_cubeArr;

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
		FramebufferDescription m_fbDescr;
	} m_shadowMapping;

	class CacheEntry
	{
	public:
		U64 m_uuid; ///< Probe UUID.
		Timestamp m_lastUsedTimestamp = 0; ///< When it was last seen by the renderer.

		Array<FramebufferDescription, 6> m_lightShadingFbDescrs;
	};

	DynamicArray<CacheEntry> m_cacheEntries;
	HashMap<U64, U32> m_probeUuidToCacheEntryIdx;

	// Other
	ImageResourcePtr m_integrationLut;
	SamplerPtr m_integrationLutSampler;

	class
	{
	public:
		const ReflectionProbeQueueElement* m_probe = nullptr;
		U32 m_cacheEntryIdx = kMaxU32;

		Array<RenderTargetHandle, kGBufferColorRenderTargetCount> m_gbufferColorRts;
		RenderTargetHandle m_gbufferDepthRt;
		RenderTargetHandle m_lightShadingRt;
		BufferHandle m_irradianceDiceValuesBuffHandle;
		RenderTargetHandle m_shadowMapRt;

		U32 m_gbufferRenderableCount = 0;
		U32 m_shadowRenderableCount = 0;
	} m_ctx; ///< Runtime context.

	Error initInternal();
	Error initGBuffer();
	Error initLightShading();
	Error initIrradiance();
	Error initIrradianceToRefl();
	Error initShadowMapping();

	/// Lazily init the cache entry
	void initCacheEntry(U32 cacheEntryIdx);

	void prepareProbes(RenderingContext& ctx, ReflectionProbeQueueElement*& probeToUpdate,
					   U32& probeToUpdateCacheEntryIdx);

	void runGBuffer(RenderPassWorkContext& rgraphCtx);
	void runShadowMapping(RenderPassWorkContext& rgraphCtx);
	void runLightShading(U32 faceIdx, const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
	void runMipmappingOfLightShading(U32 faceIdx, RenderPassWorkContext& rgraphCtx);
	void runIrradiance(RenderPassWorkContext& rgraphCtx);
	void runIrradianceToRefl(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki

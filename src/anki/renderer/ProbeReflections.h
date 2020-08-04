// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Renderer.h>
#include <anki/renderer/RendererObject.h>
#include <anki/renderer/TraditionalDeferredShading.h>
#include <anki/renderer/ClusterBin.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Probe reflections.
class ProbeReflections : public RendererObject
{
	friend class IrTask;

public:
	ProbeReflections(Renderer* r);

	~ProbeReflections();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	U getReflectionTextureMipmapCount() const
	{
		return m_lightShading.m_mipCount;
	}

	TextureViewPtr getIntegrationLut() const
	{
		return m_integrationLut->getGrTextureView();
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
		Array<RenderTargetDescription, GBUFFER_COLOR_ATTACHMENT_COUNT> m_colorRtDescrs;
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
		SamplerPtr m_shadowSampler;
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
	TextureResourcePtr m_integrationLut;
	SamplerPtr m_integrationLutSampler;

	class
	{
	public:
		const ReflectionProbeQueueElement* m_probe = nullptr;
		U32 m_cacheEntryIdx = MAX_U32;

		Array<RenderTargetHandle, GBUFFER_COLOR_ATTACHMENT_COUNT> m_gbufferColorRts;
		RenderTargetHandle m_gbufferDepthRt;
		RenderTargetHandle m_lightShadingRt;
		BufferHandle m_irradianceDiceValuesBuffHandle;
		RenderTargetHandle m_shadowMapRt;

		U32 m_gbufferRenderableCount = 0;
		U32 m_shadowRenderableCount = 0;
	} m_ctx; ///< Runtime context.

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initGBuffer(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initLightShading(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initIrradiance(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initIrradianceToRefl(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initShadowMapping(const ConfigSet& cfg);

	/// Lazily init the cache entry
	void initCacheEntry(U32 cacheEntryIdx);

	void prepareProbes(RenderingContext& ctx, ReflectionProbeQueueElement*& probeToUpdate,
					   U32& probeToUpdateCacheEntryIdx);

	void runGBuffer(RenderPassWorkContext& rgraphCtx);
	void runShadowMapping(RenderPassWorkContext& rgraphCtx);
	void runLightShading(U32 faceIdx, RenderPassWorkContext& rgraphCtx);
	void runMipmappingOfLightShading(U32 faceIdx, RenderPassWorkContext& rgraphCtx);
	void runIrradiance(RenderPassWorkContext& rgraphCtx);
	void runIrradianceToRefl(RenderPassWorkContext& rgraphCtx);

	// A RenderPassWorkCallback for the light shading pass into a single face.
	template<U faceIdx>
	static void runLightShadingCallback(RenderPassWorkContext& rgraphCtx)
	{
		ProbeReflections* const self = static_cast<ProbeReflections*>(rgraphCtx.m_userData);
		self->runLightShading(faceIdx, rgraphCtx);
	}

	// A RenderPassWorkCallback for the mipmapping of light shading result.
	template<U faceIdx>
	static void runMipmappingOfLightShadingCallback(RenderPassWorkContext& rgraphCtx)
	{
		ProbeReflections* const self = static_cast<ProbeReflections*>(rgraphCtx.m_userData);
		self->runMipmappingOfLightShading(faceIdx, rgraphCtx);
	}
};
/// @}

} // end namespace anki

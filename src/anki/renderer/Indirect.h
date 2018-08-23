// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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

/// Probe reflections and irradiance.
class Indirect : public RendererObject
{
	friend class IrTask;

anki_internal:
	Indirect(Renderer* r);

	~Indirect();

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

	RenderTargetHandle getIrradianceRt() const
	{
		return m_ctx.m_irradianceRt;
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
		U32 m_tileSize = 8;
		U32 m_envMapReadSize = 16; ///< This controls the iterations that will be used to calculate the irradiance.
		TexturePtr m_cubeArr;

		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_irradiance; ///< Irradiance.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_irradianceToRefl; ///< Apply irradiance back to the reflection.

	class CacheEntry
	{
	public:
		U64 m_probeUuid;
		Timestamp m_lastUsedTimestamp = 0; ///< When it was rendered.

		Array<FramebufferDescription, 6> m_lightShadingFbDescrs;
		Array<FramebufferDescription, 6> m_irradianceFbDescrs;
		Array<FramebufferDescription, 6> m_irradianceToReflFbDescrs;
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
		RenderTargetHandle m_irradianceRt;
	} m_ctx; ///< Runtime context.

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initGBuffer(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initLightShading(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initIrradiance(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initIrradianceToRefl(const ConfigSet& cfg);

	/// Lazily init the cache entry
	void initCacheEntry(U32 cacheEntryIdx);

	void prepareProbes(
		RenderingContext& ctx, ReflectionProbeQueueElement*& probeToUpdate, U32& probeToUpdateCacheEntryIdx);

	/// Find or allocate a new cache entry.
	Bool findBestCacheEntry(U64 probeUuid, U32& cacheEntryIdx, Bool& cacheEntryFound);

	void runGBuffer(CommandBufferPtr& cmdb);
	void runLightShading(U32 faceIdx, RenderPassWorkContext& rgraphCtx);
	void runMipmappingOfLightShading(U32 faceIdx, RenderPassWorkContext& rgraphCtx);
	void runIrradiance(U32 faceIdx, RenderPassWorkContext& rgraphCtx);
	void runIrradianceToRefl(U32 faceIdx, RenderPassWorkContext& rgraphCtx);

	// A RenderPassWorkCallback for G-buffer pass
	static void runGBufferCallback(RenderPassWorkContext& rgraphCtx)
	{
		Indirect* const self = static_cast<Indirect*>(rgraphCtx.m_userData);
		self->runGBuffer(rgraphCtx.m_commandBuffer);
	}

	// A RenderPassWorkCallback for the light shading pass into a single face.
	template<U faceIdx>
	static void runLightShadingCallback(RenderPassWorkContext& rgraphCtx)
	{
		Indirect* const self = static_cast<Indirect*>(rgraphCtx.m_userData);
		self->runLightShading(faceIdx, rgraphCtx);
	}

	// A RenderPassWorkCallback for the mipmapping of light shading result.
	template<U faceIdx>
	static void runMipmappingOfLightShadingCallback(RenderPassWorkContext& rgraphCtx)
	{
		Indirect* const self = static_cast<Indirect*>(rgraphCtx.m_userData);
		self->runMipmappingOfLightShading(faceIdx, rgraphCtx);
	}

	// A RenderPassWorkCallback for the irradiance calculation of a single cube face.
	template<U faceIdx>
	static void runIrradianceCallback(RenderPassWorkContext& rgraphCtx)
	{
		Indirect* const self = static_cast<Indirect*>(rgraphCtx.m_userData);
		self->runIrradiance(faceIdx, rgraphCtx);
	}

	// A RenderPassWorkCallback to apply the irradiance back to the reflection.
	template<U faceIdx>
	static void runIrradianceToReflCallback(RenderPassWorkContext& rgraphCtx)
	{
		Indirect* const self = static_cast<Indirect*>(rgraphCtx.m_userData);
		self->runIrradianceToRefl(faceIdx, rgraphCtx);
	}
};
/// @}

} // end namespace anki

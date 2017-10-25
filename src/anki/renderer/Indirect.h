// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Renderer.h>
#include <anki/renderer/RendererObject.h>
#include <anki/renderer/Clusterer.h>
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

	TexturePtr getIntegrationLut() const
	{
		return m_integrationLut->getGrTexture();
	}

	SamplerPtr getIntegrationLutSampler() const
	{
		return m_integrationLutSampler;
	}

private:
	struct LightPassVertexUniforms;
	struct LightPassPointLightUniforms;
	struct LightPassSpotLightUniforms;

	class
	{
	public:
		U32 m_tileSize = 0;
		Array<RenderTargetDescription, GBUFFER_COLOR_ATTACHMENT_COUNT> m_colorRtDescrs;
		RenderTargetDescription m_depthRtDescr;
		GraphicsRenderPassFramebufferDescription m_fbDescr;
	} m_gbuffer; ///< G-buffer pass.

	class
	{
	public:
		U32 m_tileSize = 0;
		U32 m_mipCount = 0;
		TexturePtr m_cubeArr;

		ShaderProgramResourcePtr m_lightProg;
		ShaderProgramPtr m_plightGrProg;
		ShaderProgramPtr m_slightGrProg;

		/// @name Vertex & index buffer of light volumes.
		/// @{
		BufferPtr m_plightPositions;
		BufferPtr m_plightIndices;
		U32 m_plightIdxCount;
		BufferPtr m_slightPositions;
		BufferPtr m_slightIndices;
		U32 m_slightIdxCount;
		/// @}
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

	class CacheEntry
	{
	public:
		U64 m_probeUuid;
		Timestamp m_lastUsedTimestamp = 0; ///< When it was rendered.

		Array<GraphicsRenderPassFramebufferDescription, 6> m_lightShadingFbDescrs;
		Array<GraphicsRenderPassFramebufferDescription, 6> m_irradianceFbDescrs;
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
	ANKI_USE_RESULT Error loadMesh(CString fname, BufferPtr& vert, BufferPtr& idx, U32& idxCount);

	/// Lazily init the cache entry
	void initCacheEntry(U32 cacheEntryIdx);

	void prepareProbes(
		RenderingContext& ctx, ReflectionProbeQueueElement*& probeToUpdate, U32& probeToUpdateCacheEntryIdx);

	/// Find or allocate a new cache entry.
	Bool findBestCacheEntry(U64 probeUuid, U32& cacheEntryIdx, Bool& cacheEntryFound);

	void runGBuffer(CommandBufferPtr& cmdb);
	void runLightShading(CommandBufferPtr& cmdb, const RenderGraph& rgraph, U32 faceIdx);
	void runMipmappingOfLightShading(CommandBufferPtr& cmdb, const RenderGraph& rgraph, U32 faceIdx);
	void runIrradiance(CommandBufferPtr& cmdb, const RenderGraph& rgraph, U32 faceIdx);

	// A RenderPassWorkCallback for G-buffer pass
	static void runGBufferCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		Indirect* self = static_cast<Indirect*>(userData);
		self->runGBuffer(cmdb);
	}

	// A RenderPassWorkCallback for the light shading pass into a single face.
	template<U faceIdx>
	static void runLightShadingCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		Indirect* self = static_cast<Indirect*>(userData);
		self->runLightShading(cmdb, rgraph, faceIdx);
	}

	// A RenderPassWorkCallback for the mipmapping of light shading result.
	template<U faceIdx>
	static void runMipmappingOfLightShadingCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		Indirect* self = static_cast<Indirect*>(userData);
		self->runMipmappingOfLightShading(cmdb, rgraph, faceIdx);
	}

	// A RenderPassWorkCallback for the irradiance calculation of a single cube face.
	template<U faceIdx>
	static void runIrradianceCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		Indirect* self = static_cast<Indirect*>(userData);
		self->runIrradiance(cmdb, rgraph, faceIdx);
	}
};
/// @}

} // end namespace anki

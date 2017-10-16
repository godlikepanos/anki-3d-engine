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

	void run(RenderingContext& ctx);

	U getReflectionTextureMipmapCount() const
	{
		return m_lightShading.m_mipCount;
	}

	TexturePtr getIrradianceTexture() const
	{
		return m_irradiance.m_cubeArr;
	}

	TexturePtr getReflectionTexture() const
	{
		return m_lightShading.m_cubeArr;
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
		Array<TexturePtr, GBUFFER_COLOR_ATTACHMENT_COUNT> m_colorRts;
		TexturePtr m_depthRt;
		FramebufferPtr m_fb;
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

		Array<FramebufferPtr, 6> m_lightShadingFbs;
		Array<FramebufferPtr, 6> m_irradianceFbs;
	};

	DynamicArray<CacheEntry> m_cacheEntries;
	HashMap<U64, U32> m_probeUuidToCacheEntryIdx;

	// Other
	TextureResourcePtr m_integrationLut;
	SamplerPtr m_integrationLutSampler;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initGBuffer(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initLightShading(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initIrradiance(const ConfigSet& cfg);
	ANKI_USE_RESULT Error loadMesh(CString fname, BufferPtr& vert, BufferPtr& idx, U32& idxCount);

	/// Lazily init the cache entry
	void initCacheEntry(U32 cacheEntryIdx);

	void prepareProbes(
		RenderingContext& ctx, ReflectionProbeQueueElement*& probeToUpdate, U32& probeToUpdateCacheEntryIdx);
	void runGBuffer(RenderingContext& rctx, const ReflectionProbeQueueElement& probe);
	void runLightShading(RenderingContext& rctx, const ReflectionProbeQueueElement& probe, CacheEntry& cacheEntry);
	void runIrradiance(RenderingContext& rctx, U32 cacheEntryIdx);

	/// Find or allocate a new cache entry.
	Bool findBestCacheEntry(U64 probeUuid, U32& cacheEntryIdx, Bool& cacheEntryFound);
};
/// @}

} // end namespace anki

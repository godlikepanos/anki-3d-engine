// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Shadowmapping pass
class ShadowMapping : public RendererObject
{
anki_internal:
	ShadowMapping(Renderer* r)
		: RendererObject(r)
	{
	}

	~ShadowMapping();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getShadowmapRt() const
	{
		return m_esmRt;
	}

private:
	/// @name ESM stuff
	/// @{

	/// The ESM map consists of tiles.
	class Tile
	{
	public:
		U64 m_lastUsedTimestamp = 0;
		U64 m_lightUuid = 0;
		U32 m_drawcallCount = 0;
		U8 m_face = 0;
		Bool8 m_pinned = false; ///< If true we cannot allocate from it.

		Vec4 m_uv;
		Array<U32, 4> m_viewport;
	};

	/// A HashMap key.
	class TileKey
	{
	public:
		U64 m_lightUuid;
		U64 m_face;

		U64 computeHash() const
		{
			return anki::computeHash(this, sizeof(*this), 693);
		}
	};

	FramebufferDescription m_esmFbDescr; ///< The FB for ESM
	TexturePtr m_esmAtlas; ///< ESM texture atlas.
	RenderTargetHandle m_esmRt;

	U32 m_tileResolution = 0; ///< Tile resolution.
	U32 m_atlasResolution = 0; ///< Atlas size is (m_atlasResolution, m_atlasResolution)
	U32 m_tileCountPerRowOrColumn = 0;
	DynamicArray<Tile> m_tiles;

	ShaderProgramResourcePtr m_esmResolveProg;
	ShaderProgramPtr m_esmResolveGrProg;

	HashMap<TileKey, U32> m_lightUuidToTileIdx;

	Bool allocateTile(U64 lightTimestamp, U64 lightUuid, U32 face, U32& tileAllocated, Bool& inTheCache);
	static Bool shouldRenderTile(U64 lightTimestamp, U64 lightUuid, U32 face, const Tile& tileIdx, U32 drawcallCount);

	class EsmResolveWorkItem
	{
	public:
		Vec4 m_uvIn; ///< UV + size that point to the scratch buffer.
		Array<U32, 4> m_viewportOut; ///< Viewport in the ESM RT.
		F32 m_cameraNear;
		F32 m_cameraFar;
	};
	WeakArray<EsmResolveWorkItem> m_esmResolveWorkItems;

	ANKI_USE_RESULT Error initEsm(const ConfigSet& cfg);

	static Mat4 createSpotLightTextureMatrix(const Tile& tile);

	/// A RenderPassWorkCallback for ESM
	static void runEsmCallback(RenderPassWorkContext& rgraphCtx)
	{
		scast<ShadowMapping*>(rgraphCtx.m_userData)->runEsm(rgraphCtx);
	}

	void runEsm(RenderPassWorkContext& rgraphCtx);
	/// @}

	/// @name Scratch buffer stuff
	/// @{
	RenderTargetHandle m_scratchRt; ///< Size of the RT is (m_scratchTileSize * m_scratchTileCount, m_scratchTileSize).
	FramebufferDescription m_scratchFbDescr; ///< FB info.
	RenderTargetDescription m_scratchRtDescr; ///< Render target.

	U32 m_scratchTileCount = 0;
	U32 m_scratchTileResolution = 0;
	U32 m_freeScratchTiles = 0;

	class ScratchBufferWorkItem
	{
	public:
		Array<U32, 4> m_viewport;
		RenderQueue* m_renderQueue;
		U32 m_firstRenderableElement;
		U32 m_renderableElementCount;
		U32 m_threadPoolTaskIdx;
	};

	struct LightToRenderToScratchInfo;

	WeakArray<ScratchBufferWorkItem> m_scratchWorkItems;

	ANKI_USE_RESULT Error initScratch(const ConfigSet& cfg);

	/// A RenderPassWorkCallback for shadow passes.
	static void runShadowmappingCallback(RenderPassWorkContext& rgraphCtx)
	{
		scast<ShadowMapping*>(rgraphCtx.m_userData)->runShadowMapping(rgraphCtx);
	}

	void runShadowMapping(RenderPassWorkContext& rgraphCtx);
	/// @}

	/// @name Misc & common
	/// @{

	/// Try to allocate a number of scratch tiles and regular tiles.
	Bool allocateTilesAndScratchTiles(U64 lightUuid,
		U32 faceCount,
		const U64* faceTimestamps,
		const U32* faceIndices,
		const U32* drawcallsCount,
		U32* tileIndices,
		U32* scratchTileIndices);

	/// Add new work to render to scratch buffer and ESM buffer.
	void newScratchAndEsmResloveRenderWorkItems(U32 tileIdx,
		U32 scratchTileIdx,
		RenderQueue* lightRenderQueue,
		DynamicArrayAuto<LightToRenderToScratchInfo>& scratchWorkItem,
		DynamicArrayAuto<EsmResolveWorkItem>& esmResolveWorkItem,
		U32& drawcallCount) const;

	/// Iterate lights and create work items.
	void processLights(RenderingContext& ctx, U32& threadCountForScratchPass);

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
	/// @}
};
/// @}

} // end namespace anki

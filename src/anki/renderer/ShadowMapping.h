// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>
#include <anki/renderer/TileAllocator.h>

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
	using Viewport = Array<U32, 4>;

	/// @name ESM stuff
	/// @{

	TileAllocator m_esmTileAlloc;

	FramebufferDescription m_esmFbDescr; ///< The FB for ESM
	TexturePtr m_esmAtlas; ///< ESM texture atlas. Size (m_esmTileResolution*m_esmTileCountBothAxis)^2
	RenderTargetHandle m_esmRt;

	U32 m_esmTileResolution = 0; ///< Tile resolution.
	U32 m_esmTileCountBothAxis = 0;

	ShaderProgramResourcePtr m_esmResolveProg;
	ShaderProgramPtr m_esmResolveGrProg;

	class EsmResolveWorkItem;
	WeakArray<EsmResolveWorkItem> m_esmResolveWorkItems;

	ANKI_USE_RESULT Error initEsm(const ConfigSet& cfg);

	inline Mat4 createSpotLightTextureMatrix(const Viewport& viewport) const;

	void runEsm(RenderPassWorkContext& rgraphCtx);
	/// @}

	/// @name Scratch buffer stuff
	/// @{
	TileAllocator m_scratchTileAlloc;

	RenderTargetHandle m_scratchRt; ///< Size of the RT is (m_scratchTileSize * m_scratchTileCount, m_scratchTileSize).
	FramebufferDescription m_scratchFbDescr; ///< FB info.
	RenderTargetDescription m_scratchRtDescr; ///< Render target.

	U32 m_scratchTileCountX = 0;
	U32 m_scratchTileCountY = 0;
	U32 m_scratchTileResolution = 0;

	class ScratchBufferWorkItem;
	class LightToRenderToScratchInfo;

	WeakArray<ScratchBufferWorkItem> m_scratchWorkItems;
	U32 m_scratchMaxViewportWidth = 0;
	U32 m_scratchMaxViewportHeight = 0;

	ANKI_USE_RESULT Error initScratch(const ConfigSet& cfg);

	void runShadowMapping(RenderPassWorkContext& rgraphCtx);
	/// @}

	/// @name Misc & common
	/// @{

	static const U m_lodCount = 3;
	static const U m_pointLightsMaxLod = 1;

	Array<F32, m_lodCount - 1> m_lodDistances;

	/// Find the lod of the light
	U choseLod(const Vec4& cameraOrigin, const PointLightQueueElement& light, Bool& blurEsm) const;
	/// Find the lod of the light
	U choseLod(const Vec4& cameraOrigin, const SpotLightQueueElement& light, Bool& blurEsm) const;

	/// Try to allocate a number of scratch tiles and regular tiles.
	TileAllocatorResult allocateTilesAndScratchTiles(U64 lightUuid,
		U32 faceCount,
		const U64* faceTimestamps,
		const U32* faceIndices,
		const U32* drawcallsCount,
		const U32* lods,
		Viewport* esmTileViewports,
		Viewport* scratchTileViewports,
		TileAllocatorResult* subResults);

	/// Add new work to render to scratch buffer and ESM buffer.
	void newScratchAndEsmResloveRenderWorkItems(const Viewport& esmViewport,
		const Viewport& scratchVewport,
		Bool blurEsm,
		Bool perspectiveProjection,
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

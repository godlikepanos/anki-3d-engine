// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
public:
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
		return m_atlas.m_rt;
	}

private:
	using Viewport = Array<U32, 4>;

	/// @name Atlas stuff
	/// @{

	class Atlas
	{
	public:
		class ResolveWorkItem;

		TileAllocator m_tileAlloc;

		TexturePtr m_tex; ///<  Size (m_tileResolution*m_tileCountBothAxis)^2
		RenderTargetHandle m_rt;

		U32 m_tileResolution = 0; ///< Tile resolution.
		U32 m_tileCountBothAxis = 0;

		ShaderProgramResourcePtr m_resolveProg;
		ShaderProgramPtr m_resolveGrProg;

		WeakArray<ResolveWorkItem> m_resolveWorkItems;
	} m_atlas;

	ANKI_USE_RESULT Error initAtlas(const ConfigSet& cfg);

	inline Mat4 createSpotLightTextureMatrix(const Viewport& viewport) const;

	void runAtlas(RenderPassWorkContext& rgraphCtx);
	/// @}

	/// @name Scratch buffer stuff
	/// @{

	class Scratch
	{
	public:
		class WorkItem;
		class LightToRenderToScratchInfo;

		TileAllocator m_tileAlloc;

		RenderTargetHandle m_rt; ///< Size of the RT is (m_tileSize * m_tileCount, m_tileSize).
		FramebufferDescription m_fbDescr; ///< FB info.
		RenderTargetDescription m_rtDescr; ///< Render target.

		U32 m_tileCountX = 0;
		U32 m_tileCountY = 0;
		U32 m_tileResolution = 0;

		WeakArray<WorkItem> m_workItems;
		U32 m_maxViewportWidth = 0;
		U32 m_maxViewportHeight = 0;
	} m_scratch;

	ANKI_USE_RESULT Error initScratch(const ConfigSet& cfg);

	void runShadowMapping(RenderPassWorkContext& rgraphCtx);
	/// @}

	/// @name Misc & common
	/// @{

	static const U32 m_lodCount = 3;
	static const U32 m_pointLightsMaxLod = 1;

	Array<F32, m_lodCount - 1> m_lodDistances;

	/// Find the lod of the light
	U32 choseLod(const Vec4& cameraOrigin, const PointLightQueueElement& light, Bool& blurAtlas) const;
	/// Find the lod of the light
	U32 choseLod(const Vec4& cameraOrigin, const SpotLightQueueElement& light, Bool& blurAtlas) const;

	/// Try to allocate a number of scratch tiles and regular tiles.
	TileAllocatorResult allocateTilesAndScratchTiles(U64 lightUuid, U32 faceCount, const U64* faceTimestamps,
													 const U32* faceIndices, const U32* drawcallsCount, const U32* lods,
													 Viewport* atlasTileViewports, Viewport* scratchTileViewports,
													 TileAllocatorResult* subResults);

	/// Add new work to render to scratch buffer and atlas buffer.
	void newScratchAndAtlasResloveRenderWorkItems(
		const Viewport& atlasViewport, const Viewport& scratchVewport, Bool blurAtlas, RenderQueue* lightRenderQueue,
		DynamicArrayAuto<Scratch::LightToRenderToScratchInfo>& scratchWorkItem,
		DynamicArrayAuto<Atlas::ResolveWorkItem>& atlasResolveWorkItem, U32& drawcallCount) const;

	/// Iterate lights and create work items.
	void processLights(RenderingContext& ctx, U32& threadCountForScratchPass);

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config);
	/// @}
};
/// @}

} // end namespace anki

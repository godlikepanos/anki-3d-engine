// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Renderer/TileAllocator.h>

namespace anki {

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

	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getShadowmapRt() const
	{
		return m_atlas.m_rt;
	}

private:
	/// @name Atlas stuff
	/// @{

	class Atlas
	{
	public:
		class ResolveWorkItem;

		TileAllocator m_tileAlloc;

		TexturePtr m_tex; ///<  Size (m_tileResolution*m_tileCountBothAxis)^2
		RenderTargetHandle m_rt;
		Bool m_rtImportedOnce = false;

		U32 m_tileResolution = 0; ///< Tile resolution.
		U32 m_tileCountBothAxis = 0;

		ShaderProgramResourcePtr m_resolveProg;
		ShaderProgramPtr m_resolveGrProg;

		FramebufferDescription m_fbDescr;

		WeakArray<ResolveWorkItem> m_resolveWorkItems;
	} m_atlas;

	Error initAtlas();

	inline Mat4 createSpotLightTextureMatrix(const UVec4& viewport) const;

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

	Error initScratch();

	void runShadowMapping(RenderPassWorkContext& rgraphCtx);
	/// @}

	/// @name Misc & common
	/// @{

	static constexpr U32 m_pointLightsMaxLod = 1;

	/// Find the lod of the light
	void chooseLod(const Vec4& cameraOrigin, const PointLightQueueElement& light, Bool& blurAtlas, U32& tileBufferLod,
				   U32& renderQueueElementsLod) const;
	/// Find the lod of the light
	void chooseLod(const Vec4& cameraOrigin, const SpotLightQueueElement& light, Bool& blurAtlas, U32& tileBufferLod,
				   U32& renderQueueElementsLod) const;

	/// Try to allocate a number of scratch tiles and regular tiles.
	TileAllocatorResult allocateTilesAndScratchTiles(U64 lightUuid, U32 faceCount, const U64* faceTimestamps,
													 const U32* faceIndices, const U32* drawcallsCount, const U32* lods,
													 UVec4* atlasTileViewports, UVec4* scratchTileViewports,
													 TileAllocatorResult* subResults);

	/// Add new work to render to scratch buffer and atlas buffer.
	void newScratchAndAtlasResloveRenderWorkItems(
		const UVec4& atlasViewport, const UVec4& scratchVewport, Bool blurAtlas, RenderQueue* lightRenderQueue,
		U32 renderQueueElementsLod, DynamicArrayRaii<Scratch::LightToRenderToScratchInfo>& scratchWorkItem,
		DynamicArrayRaii<Atlas::ResolveWorkItem>& atlasResolveWorkItem, U32& drawcallCount) const;

	/// Iterate lights and create work items.
	void processLights(RenderingContext& ctx, U32& threadCountForScratchPass);

	Error initInternal();
	/// @}
};
/// @}

} // end namespace anki

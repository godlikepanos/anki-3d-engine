// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Renderer/TileAllocator.h>

namespace anki {

// Forward
class PointLightQueueElement;
class SpotLightQueueElement;

/// @addtogroup renderer
/// @{

/// Shadowmapping pass
class ShadowMapping : public RendererObject
{
public:
	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getShadowmapRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	class LightToRenderTempInfo;
	class ThreadWorkItem;

	TileAllocator m_tileAlloc;
	static constexpr U32 kTileAllocHierarchyCount = 4;
	static constexpr U32 kPointLightMaxTileAllocHierarchy = 1;
	static constexpr U32 kSpotLightMaxTileAllocHierarchy = 1;

	TexturePtr m_atlasTex; ///<  Size (m_tileResolution*m_tileCountBothAxis)^2
	Bool m_rtImportedOnce = false;

	U32 m_tileResolution = 0; ///< Tile resolution.
	U32 m_tileCountBothAxis = 0;

	FramebufferDescription m_fbDescr;

	ShaderProgramResourcePtr m_clearDepthProg;
	ShaderProgramPtr m_clearDepthGrProg;

	class
	{
	public:
		RenderTargetHandle m_rt;
		WeakArray<ThreadWorkItem> m_workItems;
		UVec4 m_fullViewport; ///< Calculate the viewport that contains all of the work items. Mobile optimization.
	} m_runCtx;

	Error initInternal();

	void processLights(RenderingContext& ctx, U32& threadCountForScratchPass);

	Bool allocateAtlasTiles(U64 lightUuid, U32 faceCount, const U64* faceTimestamps, const U32* faceIndices,
							const U32* drawcallsCount, const U32* hierarchies, UVec4* atlasTileViewports,
							TileAllocatorResult* subResults);

	Mat4 createSpotLightTextureMatrix(const UVec4& viewport) const;

	/// Find the detail of the light
	void chooseDetail(const Vec4& cameraOrigin, const PointLightQueueElement& light, U32& tileAllocatorHierarchy,
					  U32& renderQueueElementsLod) const;
	/// Find the detail of the light
	void chooseDetail(const Vec4& cameraOrigin, const SpotLightQueueElement& light, U32& tileAllocatorHierarchy,
					  U32& renderQueueElementsLod) const;

	template<typename TMemoryPool>
	void newWorkItems(const UVec4& atlasViewport, RenderQueue* lightRenderQueue, U32 renderQueueElementsLod,
					  DynamicArray<LightToRenderTempInfo, TMemoryPool>& workItems, U32& drawcallCount) const;

	void runShadowMapping(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki

// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Renderer/Utils/TileAllocator2.h>

namespace anki {

// Forward
extern NumericCVar<U32> g_shadowMappingPcfCVar;

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
	class ViewportWorkItem;

	TileAllocator2 m_tileAlloc;
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

	Array<RenderTargetDescription, kMaxShadowCascades> m_cascadeHzbRtDescrs;

	class
	{
	public:
		RenderTargetHandle m_rt;
		WeakArray<ViewportWorkItem> m_workItems;

		UVec2 m_renderAreaMin; ///< Calculate the viewport that contains all of the work items. Mobile optimization.
		UVec2 m_renderAreaMax;
	} m_runCtx;

	Error initInternal();

	void processLights(RenderingContext& ctx);

	TileAllocatorResult2 allocateAtlasTiles(U32 lightUuid, U32 componentIndex, U32 faceCount, const U32* hierarchies, UVec4* atlasTileViewports);

	Mat4 createSpotLightTextureMatrix(const UVec4& viewport) const;

	void chooseDetail(const Vec3& cameraOrigin, const LightComponent& lightc, U32& tileAllocatorHierarchy) const;

	void runShadowMapping(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki

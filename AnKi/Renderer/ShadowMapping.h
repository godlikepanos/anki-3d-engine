// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Renderer/Utils/TileAllocator.h>

namespace anki {

// Forward
class GpuVisibilityOutput;

/// @addtogroup renderer
/// @{

inline NumericCVar<U32> g_shadowMappingTileResolutionCVar("R", "ShadowMappingTileResolution", (ANKI_PLATFORM_MOBILE) ? 128 : 256, 16, 2048,
														  "Shadowmapping tile resolution");
inline NumericCVar<U32> g_shadowMappingTileCountPerRowOrColumnCVar("R", "ShadowMappingTileCountPerRowOrColumn", 32, 1, 256,
																   "Shadowmapping atlas will have this number squared number of tiles");
inline BoolCVar g_shadowMappingPcfCVar("R", "ShadowMappingPcf", (ANKI_PLATFORM_MOBILE) ? false : true, "Shadow PCF");
inline BoolCVar g_shadowMappingPcssCVar("R", "ShadowMappingPcss", (ANKI_PLATFORM_MOBILE) ? false : true, "Shadow PCSS");

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
	class ShadowSubpassInfo
	{
	public:
		UVec4 m_viewport;
		Mat4 m_viewProjMat;
		Mat3x4 m_viewMat;
		BufferView m_clearTileIndirectArgs;
		RenderTargetHandle m_hzbRt;
	};

	TileAllocator m_tileAlloc;
	static constexpr U32 kTileAllocHierarchyCount = 4;
	static constexpr U32 kPointLightMaxTileAllocHierarchy = 1;
	static constexpr U32 kSpotLightMaxTileAllocHierarchy = 1;

	TexturePtr m_atlasTex; ///<  Size (m_tileResolution*m_tileCountBothAxis)^2
	Bool m_rtImportedOnce = false;

	U32 m_tileResolution = 0; ///< Tile resolution.
	U32 m_tileCountBothAxis = 0;

	ShaderProgramResourcePtr m_clearDepthProg;
	ShaderProgramPtr m_clearDepthGrProg;

	ShaderProgramResourcePtr m_vetVisibilityProg;
	ShaderProgramPtr m_vetVisibilityGrProg;

	Array<RenderTargetDesc, kMaxShadowCascades> m_cascadeHzbRtDescrs;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;

	void processLights(RenderingContext& ctx);

	TileAllocatorResult2 allocateAtlasTiles(U32 lightUuid, U32 componentIndex, U32 faceCount, const U32* hierarchies, UVec4* atlasTileViewports);

	Mat4 createSpotLightTextureMatrix(const UVec4& viewport) const;

	void chooseDetail(const Vec3& cameraOrigin, const LightComponent& lightc, Vec2 lodDistances, U32& tileAllocatorHierarchy) const;

	BufferView createVetVisibilityPass(CString passName, const LightComponent& lightc, const GpuVisibilityOutput& visOut,
									   RenderGraphBuilder& rgraph) const;

	void createDrawShadowsPass(const UVec4& viewport, const Mat4& viewProjMat, const Mat3x4& viewMat, const GpuVisibilityOutput& visOut,
							   const BufferView& clearTileIndirectArgs, const RenderTargetHandle hzbRt, CString passName, RenderGraphBuilder& rgraph);

	void createDrawShadowsPass(ConstWeakArray<ShadowSubpassInfo> subPasses, const GpuVisibilityOutput& visOut, CString passName,
							   RenderGraphBuilder& rgraph);
};
/// @}

} // end namespace anki

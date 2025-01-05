// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>
#include <AnKi/Math.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Collision/Forward.h>

namespace anki {

/// @addtogroup renderer
/// @{

inline NumericCVar<F32> g_internalRenderScalingCVar("R", "InternalRenderScaling", 1.0f, 0.5f, 1.0f,
													"A factor over the requested swapchain resolution. Applies to all passes up to TAA");
inline NumericCVar<F32> g_renderScalingCVar("R", "RenderScaling", 1.0f, 0.5f, 8.0f,
											"A factor over the requested swapchain resolution. Applies to post-processing and UI");
inline NumericCVar<U32> g_zSplitCountCVar("R", "ZSplitCount", 64, 8, kMaxZsplitCount, "Clusterer number of Z splits");
inline NumericCVar<U8> g_textureAnisotropyCVar("R", "TextureAnisotropy", (ANKI_PLATFORM_MOBILE) ? 1 : 16, 1, 16,
											   "Texture anisotropy for the main passes");
inline BoolCVar g_preferComputeCVar("R", "PreferCompute", !ANKI_PLATFORM_MOBILE, "Prefer compute shaders");
inline BoolCVar g_highQualityHdrCVar("R", "HighQualityHdr", !ANKI_PLATFORM_MOBILE,
									 "If true use R16G16B16 for HDR images. Alternatively use B10G11R11");
inline BoolCVar g_vrsLimitTo2x2CVar("R", "VrsLimitTo2x2", false, "If true the max rate will be 2x2");
inline NumericCVar<U8> g_shadowCascadeCountCVar("R", "ShadowCascadeCount", (ANKI_PLATFORM_MOBILE) ? 3 : kMaxShadowCascades, 1, kMaxShadowCascades,
												"Max number of shadow cascades for directional lights");
inline NumericCVar<F32> g_shadowCascade0DistanceCVar("R", "ShadowCascade0Distance", 18.0, 1.0, kMaxF32, "The distance of the 1st cascade");
inline NumericCVar<F32> g_shadowCascade1DistanceCVar("R", "ShadowCascade1Distance", (ANKI_PLATFORM_MOBILE) ? 80.0f : 40.0, 1.0, kMaxF32,
													 "The distance of the 2nd cascade");
inline NumericCVar<F32> g_shadowCascade2DistanceCVar("R", "ShadowCascade2Distance", (ANKI_PLATFORM_MOBILE) ? 150.0f : 80.0, 1.0, kMaxF32,
													 "The distance of the 3rd cascade");
inline NumericCVar<F32> g_shadowCascade3DistanceCVar("R", "ShadowCascade3Distance", 200.0, 1.0, kMaxF32, "The distance of the 4th cascade");
inline NumericCVar<F32> g_lod0MaxDistanceCVar("R", "Lod0MaxDistance", 20.0f, 1.0f, kMaxF32, "Distance that will be used to calculate the LOD 0");
inline NumericCVar<F32> g_lod1MaxDistanceCVar("R", "Lod1MaxDistance", 40.0f, 2.0f, kMaxF32, "Distance that will be used to calculate the LOD 1");

inline StatCounter g_rendererGpuTimeStatVar(StatCategory::kTime, "GPU frame",
											StatFlag::kMilisecond | StatFlag::kShowAverage | StatFlag::kMainThreadUpdates);

/// Renderer statistics.
class RendererPrecreatedSamplers
{
public:
	SamplerPtr m_nearestNearestClamp;
	SamplerPtr m_trilinearClamp;
	SamplerPtr m_trilinearRepeat;
	SamplerPtr m_trilinearRepeatAniso;
	SamplerPtr m_trilinearRepeatAnisoResolutionScalingBias;
	SamplerPtr m_trilinearClampShadow;
};

enum class MeshletRenderingType
{
	kNone,
	kMeshShaders,
	kSoftware
};

class RendererInitInfo
{
public:
	UVec2 m_swapchainSize = UVec2(0u);

	AllocAlignedCallback m_allocCallback = nullptr;
	void* m_allocCallbackUserData = nullptr;
};

/// Offscreen renderer.
class Renderer : public MakeSingleton<Renderer>
{
public:
	Renderer();

	~Renderer();

	Error init(const RendererInitInfo& inf);

	Error render(Texture* presentTex);

#define ANKI_RENDERER_OBJECT_DEF(name, name2, initCondition) \
	name& get##name() \
	{ \
		ANKI_ASSERT(m_##name2); \
		return *m_##name2; \
	}
#include <AnKi/Renderer/RendererObject.def.h>

	Bool getRtShadowsEnabled() const
	{
		return m_rtShadows != nullptr;
	}

	const UVec2& getInternalResolution() const
	{
		return m_internalResolution;
	}

	const UVec2& getPostProcessResolution() const
	{
		return m_postProcessResolution;
	}

	const UVec2& getSwapchainResolution() const
	{
		return m_swapchainResolution;
	}

	F32 getAspectRatio() const
	{
		return F32(m_internalResolution.x()) / F32(m_internalResolution.y());
	}

	U64 getFrameCount() const
	{
		return m_frameCount;
	}

	MeshletRenderingType getMeshletRenderingType() const
	{
		return m_meshletRenderingType;
	}

	/// Create the init info for a 2D texture that will be used as a render target.
	[[nodiscard]] TextureInitInfo create2DRenderTargetInitInfo(U32 w, U32 h, Format format, TextureUsageBit usage, CString name = {});

	/// Create the init info for a 2D texture that will be used as a render target.
	[[nodiscard]] RenderTargetDesc create2DRenderTargetDescription(U32 w, U32 h, Format format, CString name = {});

	[[nodiscard]] TexturePtr createAndClearRenderTarget(const TextureInitInfo& inf, TextureUsageBit initialUsage,
														const ClearValue& clearVal = ClearValue());

	Texture& getDummyTexture2d() const
	{
		return *m_dummyTex2d;
	}

	Texture& getDummyTexture3d() const
	{
		return *m_dummyTex3d;
	}

	Buffer& getDummyBuffer() const
	{
		return *m_dummyBuff;
	}

	const RendererPrecreatedSamplers& getSamplers() const
	{
		return m_samplers;
	}

	const UVec2& getTileCounts() const
	{
		return m_tileCounts;
	}

	U32 getZSplitCount() const
	{
		return m_zSplitCount;
	}

	Format getHdrFormat() const;
	Format getDepthNoStencilFormat() const;

	BufferHandle getGpuSceneBufferHandle() const
	{
		return m_runCtx.m_gpuSceneHandle;
	}

	/// @name Debug render targets
	/// @{

	/// Register a debug render target.
	void registerDebugRenderTarget(RendererObject* obj, CString rtName);

	/// Set the render target you want to show.
	void setCurrentDebugRenderTarget(CString rtName);

	/// Get the render target currently showing.
	CString getCurrentDebugRenderTarget() const
	{
		return m_currentDebugRtName;
	}

	// Need to call it after the handle is set by the RenderGraph.
	Bool getCurrentDebugRenderTarget(Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles, ShaderProgramPtr& optionalShaderProgram);
	/// @}

	StackMemoryPool& getFrameMemoryPool()
	{
		return m_framePool;
	}

	ShaderProgram& getFillBufferProgram() const
	{
		return *m_fillBufferGrProg;
	}

#if ANKI_STATS_ENABLED
	void appendPipelineQuery(PipelineQuery* q)
	{
		ANKI_ASSERT(q);
		LockGuard lock(m_pipelineQueriesMtx);
		m_pipelineQueries[m_frameCount % kMaxFramesInFlight].emplaceBack(q);
	}
#endif

private:
	class Cleanup
	{
	public:
		~Cleanup()
		{
			RendererMemoryPool::freeSingleton();
		}
	} m_cleanup; // First so it will be called last in the renderer's destructor

	/// @name Rendering stages
	/// @{
#define ANKI_RENDERER_OBJECT_DEF(name, name2, initCondition) name* m_##name2 = nullptr;
#include <AnKi/Renderer/RendererObject.def.h>
	/// @}

	StackMemoryPool m_framePool;

	UVec2 m_internalResolution = UVec2(0u); ///< The resolution of all passes up until TAA.
	UVec2 m_postProcessResolution = UVec2(0u); ///< The resolution of post processing and following passes.
	UVec2 m_swapchainResolution = UVec2(0u);

	U64 m_frameCount; ///< Frame number

	CommonMatrices m_prevMatrices;

	Array<Vec2, 64> m_jitterOffsets;

	TexturePtr m_dummyTex2d;
	TexturePtr m_dummyTex3d;
	BufferPtr m_dummyBuff;

	RendererPrecreatedSamplers m_samplers;

	ShaderProgramResourcePtr m_clearTexComputeProg;

	class DebugRtInfo
	{
	public:
		RendererObject* m_obj;
		RendererString m_rtName;
	};
	RendererDynamicArray<DebugRtInfo> m_debugRts;
	RendererString m_currentDebugRtName;

	ShaderProgramResourcePtr m_blitProg;
	ShaderProgramPtr m_blitGrProg;

	ShaderProgramResourcePtr m_fillBufferProg;
	ShaderProgramPtr m_fillBufferGrProg;

	RenderGraphPtr m_rgraph;

	UVec2 m_tileCounts = UVec2(0u);
	U32 m_zSplitCount = 0;

	class
	{
	public:
		BufferHandle m_gpuSceneHandle;
	} m_runCtx;

#if ANKI_STATS_ENABLED
	Array<RendererDynamicArray<PipelineQueryPtr>, kMaxFramesInFlight> m_pipelineQueries;
	Mutex m_pipelineQueriesMtx;
#endif

	MeshletRenderingType m_meshletRenderingType = MeshletRenderingType::kNone;

	Error initInternal(const RendererInitInfo& inf);

	void gpuSceneCopy(RenderingContext& ctx);

#if ANKI_STATS_ENABLED
	void updatePipelineStats();
#endif

	void writeGlobalRendererConstants(RenderingContext& ctx, GlobalRendererConstants& consts);

	/// This function does all the rendering stages and produces a final result.
	Error populateRenderGraph(RenderingContext& ctx);
};
/// @}

} // end namespace anki

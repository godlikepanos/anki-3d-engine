// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>
#include <AnKi/Renderer/Utils/Drawer.h>
#include <AnKi/Renderer/Utils/GpuVisibility.h>
#include <AnKi/Renderer/Utils/HzbGenerator.h>
#include <AnKi/Math.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Collision/Forward.h>
#include <AnKi/Renderer/Utils/Readback.h>

namespace anki {

// Forward
extern BoolCVar g_vrsCVar;
extern BoolCVar g_vrsLimitTo2x2CVar;
extern BoolCVar g_preferComputeCVar;
extern NumericCVar<F32> g_renderScalingCVar;
extern BoolCVar g_rayTracedShadowsCVar;
extern BoolCVar g_meshletRenderingCVar;
extern NumericCVar<U8> g_shadowCascadeCountCVar;
extern NumericCVar<F32> g_shadowCascade0DistanceCVar;
extern NumericCVar<F32> g_shadowCascade1DistanceCVar;
extern NumericCVar<F32> g_shadowCascade2DistanceCVar;
extern NumericCVar<F32> g_shadowCascade3DistanceCVar;
extern NumericCVar<F32> g_lod0MaxDistanceCVar;
extern NumericCVar<F32> g_lod1MaxDistanceCVar;

/// @addtogroup renderer
/// @{

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

/// Offscreen renderer.
class Renderer
{
public:
	Renderer();

	~Renderer();

#define ANKI_RENDERER_OBJECT_DEF(name, name2, initCondition) \
	name& get##name() \
	{ \
		return *m_##name2; \
	}
#include <AnKi/Renderer/RendererObject.def.h>

	Bool getRtShadowsEnabled() const
	{
		return m_rtShadows.isCreated();
	}

	const UVec2& getInternalResolution() const
	{
		return m_internalResolution;
	}

	const UVec2& getPostProcessResolution() const
	{
		return m_postProcessResolution;
	}

	F32 getAspectRatio() const
	{
		return F32(m_internalResolution.x()) / F32(m_internalResolution.y());
	}

	/// Init the renderer.
	Error init(UVec2 swapchainSize, StackMemoryPool* framePool);

	/// This function does all the rendering stages and produces a final result.
	Error populateRenderGraph(RenderingContext& ctx);

	void finalize(const RenderingContext& ctx, Fence* fence);

	U64 getFrameCount() const
	{
		return m_frameCount;
	}

	const RenderableDrawer& getSceneDrawer() const
	{
		return m_sceneDrawer;
	}

	RenderableDrawer& getSceneDrawer()
	{
		return m_sceneDrawer;
	}

	GpuVisibility& getGpuVisibility()
	{
		return m_visibility;
	}

	Bool runSoftwareMeshletRendering() const
	{
		return g_meshletRenderingCVar.get() && !GrManager::getSingleton().getDeviceCapabilities().m_meshShaders;
	}

	GpuVisibilityNonRenderables& getGpuVisibilityNonRenderables()
	{
		return m_nonRenderablesVisibility;
	}

	GpuVisibilityAccelerationStructures& getGpuVisibilityAccelerationStructures()
	{
		return m_asVisibility;
	}

	const HzbGenerator& getHzbGenerator() const
	{
		return m_hzbGenerator;
	}

	ReadbackManager& getReadbackManager()
	{
		return m_readbaks;
	}

	/// Create the init info for a 2D texture that will be used as a render target.
	[[nodiscard]] TextureInitInfo create2DRenderTargetInitInfo(U32 w, U32 h, Format format, TextureUsageBit usage, CString name = {});

	/// Create the init info for a 2D texture that will be used as a render target.
	[[nodiscard]] RenderTargetDescription create2DRenderTargetDescription(U32 w, U32 h, Format format, CString name = {});

	[[nodiscard]] TexturePtr createAndClearRenderTarget(const TextureInitInfo& inf, TextureUsageBit initialUsage,
														const ClearValue& clearVal = ClearValue());

	TextureView& getDummyTextureView2d() const
	{
		return *m_dummyTexView2d;
	}

	TextureView& getDummyTextureView3d() const
	{
		return *m_dummyTexView3d;
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

	StackMemoryPool& getFrameMemoryPool() const
	{
		ANKI_ASSERT(m_framePool);
		return *m_framePool;
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
	/// @name Rendering stages
	/// @{
#define ANKI_RENDERER_OBJECT_DEF(name, name2, initCondition) UniquePtr<name, SingletonMemoryPoolDeleter<RendererMemoryPool>> m_##name2;
#include <AnKi/Renderer/RendererObject.def.h>
	/// @}

	UVec2 m_tileCounts = UVec2(0u);
	U32 m_zSplitCount = 0;

	UVec2 m_internalResolution = UVec2(0u); ///< The resolution of all passes up until TAA.
	UVec2 m_postProcessResolution = UVec2(0u); ///< The resolution of post processing and following passes.

	RenderableDrawer m_sceneDrawer;
	GpuVisibility m_visibility;
	GpuVisibilityNonRenderables m_nonRenderablesVisibility;
	GpuVisibilityAccelerationStructures m_asVisibility;
	HzbGenerator m_hzbGenerator;
	ReadbackManager m_readbaks;

	U64 m_frameCount; ///< Frame number

	CommonMatrices m_prevMatrices;

	Array<Vec2, 64> m_jitterOffsets;

	TextureViewPtr m_dummyTexView2d;
	TextureViewPtr m_dummyTexView3d;
	BufferPtr m_dummyBuff;

	RendererPrecreatedSamplers m_samplers;

	ShaderProgramResourcePtr m_clearTexComputeProg;

	StackMemoryPool* m_framePool = nullptr;

	class DebugRtInfo
	{
	public:
		RendererObject* m_obj;
		RendererString m_rtName;
	};
	RendererDynamicArray<DebugRtInfo> m_debugRts;
	RendererString m_currentDebugRtName;

	class
	{
	public:
		BufferHandle m_gpuSceneHandle;
	} m_runCtx;

#if ANKI_STATS_ENABLED
	Array<RendererDynamicArray<PipelineQueryPtr>, kMaxFramesInFlight> m_pipelineQueries;
	Mutex m_pipelineQueriesMtx;
#endif

	Error initInternal(UVec2 swapchainSize);

	void gpuSceneCopy(RenderingContext& ctx);

#if ANKI_STATS_ENABLED
	void updatePipelineStats();
#endif
};
/// @}

} // end namespace anki

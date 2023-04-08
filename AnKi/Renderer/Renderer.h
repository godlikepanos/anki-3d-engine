// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>
#include <AnKi/Renderer/Drawer.h>
#include <AnKi/Math.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Core/GpuMemoryPools.h>
#include <AnKi/Collision/Forward.h>

namespace anki {

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

#define ANKI_RENDERER_OBJECT_DEF(a, b) \
	a& get##a() \
	{ \
		return *m_##b; \
	}
#include <AnKi/Renderer/RendererObject.defs.h>
#undef ANKI_RENDERER_OBJECT_DEF

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
	Error init(UVec2 swapchainSize);

	/// This function does all the rendering stages and produces a final result.
	Error populateRenderGraph(RenderingContext& ctx);

	void finalize(const RenderingContext& ctx);

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

	/// Create the init info for a 2D texture that will be used as a render target.
	[[nodiscard]] TextureInitInfo create2DRenderTargetInitInfo(U32 w, U32 h, Format format, TextureUsageBit usage,
															   CString name = {});

	/// Create the init info for a 2D texture that will be used as a render target.
	[[nodiscard]] RenderTargetDescription create2DRenderTargetDescription(U32 w, U32 h, Format format,
																		  CString name = {});

	[[nodiscard]] TexturePtr createAndClearRenderTarget(const TextureInitInfo& inf, TextureUsageBit initialUsage,
														const ClearValue& clearVal = ClearValue());

	/// Returns true if there were resources loaded or loading async tasks that got completed.
	Bool resourcesLoaded() const
	{
		return m_resourcesDirty;
	}

	TextureViewPtr getDummyTextureView2d() const
	{
		return m_dummyTexView2d;
	}

	TextureViewPtr getDummyTextureView3d() const
	{
		return m_dummyTexView3d;
	}

	BufferPtr getDummyBuffer() const
	{
		return m_dummyBuff;
	}

	const RendererPrecreatedSamplers& getSamplers() const
	{
		return m_samplers;
	}

	U32 getTileSize() const
	{
		return m_tileSize;
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
	Bool getCurrentDebugRenderTarget(Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
									 ShaderProgramPtr& optionalShaderProgram);
	/// @}

private:
	/// @name Rendering stages
	/// @{
#define ANKI_RENDERER_OBJECT_DEF(a, b) UniquePtr<a, SingletonMemoryPoolDeleter<RendererMemoryPool>> m_##b;
#include <AnKi/Renderer/RendererObject.defs.h>
#undef ANKI_RENDERER_OBJECT_DEF
	/// @}

	U32 m_tileSize = 0;
	UVec2 m_tileCounts = UVec2(0u);
	U32 m_zSplitCount = 0;

	UVec2 m_internalResolution = UVec2(0u); ///< The resolution of all passes up until TAA.
	UVec2 m_postProcessResolution = UVec2(0u); ///< The resolution of post processing and following passes.

	RenderableDrawer m_sceneDrawer;

	U64 m_frameCount; ///< Frame number

	U64 m_prevLoadRequestCount = 0;
	U64 m_prevAsyncTasksCompleted = 0;
	Bool m_resourcesDirty = true;

	CommonMatrices m_prevMatrices;

	Array<Vec2, 64> m_jitterOffsets;

	TextureViewPtr m_dummyTexView2d;
	TextureViewPtr m_dummyTexView3d;
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

	class
	{
	public:
		BufferHandle m_gpuSceneHandle;
	} m_runCtx;

	Error initInternal(UVec2 swapchainSize);

	void gpuSceneCopy(RenderingContext& ctx);
};
/// @}

} // end namespace anki

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

ANKI_CVAR(
	NumericCVar<F32>, Render, InternalRenderScaling, 1.0f,
	[](F32 value) {
		return (value > 0.1f && value <= 8.0f) || value == 540.0f || value == 720.0f || value == 1080.0f || value == 1440.0f || value == 2160.0f;
	},
	"A factor over the requested swapchain resolution or some common resolution values (eg 1080, 720 etc). Applies to all passes up to TAA")
ANKI_CVAR(
	NumericCVar<F32>, Render, RenderScaling, 1.0f,
	[](F32 value) {
		return (value > 0.1f && value <= 8.0f) || value == 540.0f || value == 720.0f || value == 1080.0f || value == 1440.0f || value == 2160.0f;
	},
	"A factor over the requested swapchain resolution. Applies to post-processing and UI")
ANKI_CVAR(NumericCVar<U8>, Render, TextureAnisotropy, (ANKI_PLATFORM_MOBILE) ? 1 : 16, 1, 16, "Texture anisotropy for the main passes")
ANKI_CVAR(BoolCVar, Render, PreferCompute, !ANKI_PLATFORM_MOBILE, "Prefer compute shaders")
ANKI_CVAR(BoolCVar, Render, HighQualityHdr, !ANKI_PLATFORM_MOBILE, "If true use R16G16B16 for HDR images. Alternatively use B10G11R11")
ANKI_CVAR(BoolCVar, Render, VrsLimitTo2x2, false, "If true the max rate will be 2x2")
ANKI_CVAR(NumericCVar<U8>, Render, ShadowCascadeCount, (ANKI_PLATFORM_MOBILE) ? 3 : kMaxShadowCascades, 1, kMaxShadowCascades,
		  "Max number of shadow cascades for directional lights")
ANKI_CVAR(NumericCVar<F32>, Render, ShadowCascade0Distance, 18.0, 1.0, kMaxF32, "The distance of the 1st cascade")
ANKI_CVAR(NumericCVar<F32>, Render, ShadowCascade1Distance, (ANKI_PLATFORM_MOBILE) ? 80.0f : 40.0, 1.0, kMaxF32, "The distance of the 2nd cascade")
ANKI_CVAR(NumericCVar<F32>, Render, ShadowCascade2Distance, (ANKI_PLATFORM_MOBILE) ? 150.0f : 80.0, 1.0, kMaxF32, "The distance of the 3rd cascade")
ANKI_CVAR(NumericCVar<F32>, Render, ShadowCascade3Distance, 200.0, 1.0, kMaxF32, "The distance of the 4th cascade")
ANKI_CVAR(NumericCVar<F32>, Render, Lod0MaxDistance, 20.0f, 1.0f, kMaxF32, "Distance that will be used to calculate the LOD 0")
ANKI_CVAR(NumericCVar<F32>, Render, Lod1MaxDistance, 40.0f, 2.0f, kMaxF32, "Distance that will be used to calculate the LOD 1")

ANKI_SVAR(RendererGpuTime, StatCategory::kTime, "GPU frame", StatFlag::kMilisecond | StatFlag::kShowAverage | StatFlag::kMainThreadUpdates)

// Renderer statistics.
class RendererPrecreatedSamplers
{
public:
	SamplerPtr m_nearestNearestClamp;
	SamplerPtr m_nearestNearestRepeat;
	SamplerPtr m_trilinearClamp;
	SamplerPtr m_trilinearRepeat;
	SamplerPtr m_trilinearRepeatAniso;
	SamplerPtr m_trilinearRepeatAnisoResolutionScalingBias;
	SamplerPtr m_trilinearClampShadow;
};

// Some dummy resources to fill the slots.
class DummyGpuResources
{
public:
	TexturePtr m_texture2DSrv;
	TexturePtr m_texture2DUintSrv;
	TexturePtr m_texture3DSrv;
	TexturePtr m_texture2DUav;
	TexturePtr m_texture3DUav;
	BufferPtr m_buffer; // It's 1024 bytes
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

// The container of all rendering
class Renderer : public MakeSingleton<Renderer>
{
	friend class RendererObject;

public:
	Renderer();

	~Renderer();

	Error init(const RendererInitInfo& inf);

	Error render();

#define ANKI_RENDERER_OBJECT_DEF(type, name, initCondition) \
	type& get##type() \
	{ \
		ANKI_ASSERT(m_##name != nullptr && m_##name != numberToPtr<type*>(kMaxPtrSize)); \
		return *m_##name; \
	} \
	Bool is##type##Enabled() const \
	{ \
		ANKI_ASSERT(m_##name != numberToPtr<type*>(kMaxPtrSize) && "Calling this before the renderer had a chance to decide if to initialize it"); \
		return m_##name != nullptr; \
	}
#include <AnKi/Renderer/RendererObject.def.h>

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
		return F32(m_internalResolution.x) / F32(m_internalResolution.y);
	}

	U64 getFrameCount() const
	{
		return m_frameCount;
	}

	MeshletRenderingType getMeshletRenderingType() const
	{
		return m_meshletRenderingType;
	}

	// Create the init info for a 2D texture that will be used as a render target.
	[[nodiscard]] TextureInitInfo create2DRenderTargetInitInfo(U32 w, U32 h, Format format, TextureUsageBit usage, CString name = {});

	// Create the init info for a 2D texture that will be used as a render target.
	[[nodiscard]] RenderTargetDesc create2DRenderTargetDescription(U32 w, U32 h, Format format, CString name = {});

	[[nodiscard]] TexturePtr createAndClearRenderTarget(const TextureInitInfo& inf, TextureUsageBit initialUsage,
														const ClearValue& clearVal = ClearValue());

	const RendererPrecreatedSamplers& getSamplers() const
	{
		return m_samplers;
	}

	Format getHdrFormat() const;
	Format getDepthNoStencilFormat() const;

	BufferHandle getGpuSceneBufferHandle() const
	{
		return m_runCtx.m_gpuSceneHandle;
	}

	// Register a debug render target.
	void registerDebugRenderTarget(RendererObject* obj, CString rtName);

	// Set the render target you want to show.
	void setCurrentDebugRenderTarget(CString rtName, Bool disableTonemapping = false);

	// Get the render target currently showing.
	CString getCurrentDebugRenderTarget() const
	{
		return m_currentDebugRtName;
	}

	// Need to call it after the handle is set by the RenderGraph.
	Bool getCurrentDebugRenderTarget(Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
									 DebugRenderTargetDrawStyle& drawStyle);

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

	template<typename TFunc>
	void iterateDebugRenderTargetNames(TFunc func) const
	{
		for(const auto& x : m_debugRts)
		{
			if(func(x.m_rtName) == FunctorContinue::kStop)
			{
				break;
			}
		}
	}

	// Return the current rendering context. It's valid only inside render()
	ANKI_INTERNAL RenderingContext& getRenderingContext() const
	{
		ANKI_ASSERT(m_runCtx.m_currentCtx);
		return *m_runCtx.m_currentCtx;
	}

private:
	class Cleanup
	{
	public:
		~Cleanup()
		{
			RendererMemoryPool::freeSingleton();
		}
	} m_cleanup; // First so it will be called last in the renderer's destructor

	// Rendering stages
#define ANKI_RENDERER_OBJECT_DEF(name, name2, initCondition) name* m_##name2 = numberToPtr<name*>(kMaxPtrSize);
#include <AnKi/Renderer/RendererObject.def.h>

	StackMemoryPool m_framePool;

	UVec2 m_internalResolution = UVec2(0u); // The resolution of all passes up until TAA.
	UVec2 m_postProcessResolution = UVec2(0u); // The resolution of post processing and following passes.
	UVec2 m_swapchainResolution = UVec2(0u);

	U64 m_frameCount; // Frame number

	CommonMatrices m_prevMatrices;

	Array<Vec2, 64> m_jitterOffsets;

	DummyGpuResources m_dummyResources;

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
	Bool m_disableDebugRtTonemapping = false;

	ShaderProgramResourcePtr m_blitProg;
	ShaderProgramPtr m_blitGrProg;

	ShaderProgramResourcePtr m_fillBufferProg;
	ShaderProgramPtr m_fillBufferGrProg;

	RenderGraphPtr m_rgraph;

	class
	{
	public:
		BufferHandle m_gpuSceneHandle;
		RenderingContext* m_currentCtx = nullptr;
	} m_runCtx;

#if ANKI_STATS_ENABLED
	Array<RendererDynamicArray<PipelineQueryPtr>, kMaxFramesInFlight> m_pipelineQueries;
	Mutex m_pipelineQueriesMtx;
#endif

	MeshletRenderingType m_meshletRenderingType = MeshletRenderingType::kNone;

	Error initInternal(const RendererInitInfo& inf);

	void gpuSceneCopy();

#if ANKI_STATS_ENABLED
	void updatePipelineStats();
#endif

	void writeGlobalRendererConstants(GlobalRendererConstants& consts);

	// This function does all the rendering stages and produces a final result.
	Error populateRenderGraph();
};

} // end namespace anki

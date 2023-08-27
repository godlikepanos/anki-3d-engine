// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr.h>
#include <AnKi/Util/Ptr.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>
#include <AnKi/Scene/GpuSceneArray.h>

namespace anki {

// Forward
#define ANKI_RENDERER_OBJECT_DEF(a, b) class a;
#include <AnKi/Renderer/RendererObject.def.h>
#undef ANKI_RENDERER_OBJECT_DEF

class Renderer;
class RendererObject;
class RenderQueue;

/// @addtogroup renderer
/// @{

#define ANKI_R_LOGI(...) ANKI_LOG("REND", kNormal, __VA_ARGS__)
#define ANKI_R_LOGE(...) ANKI_LOG("REND", kError, __VA_ARGS__)
#define ANKI_R_LOGW(...) ANKI_LOG("REND", kWarning, __VA_ARGS__)
#define ANKI_R_LOGF(...) ANKI_LOG("REND", kFatal, __VA_ARGS__)
#define ANKI_R_LOGV(...) ANKI_LOG("REND", kVerbose, __VA_ARGS__)

class RendererMemoryPool : public HeapMemoryPool, public MakeSingleton<RendererMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

private:
	RendererMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData)
		: HeapMemoryPool(allocCb, allocCbUserData, "RendererMemPool")
	{
	}

	~RendererMemoryPool() = default;
};

ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(Renderer, RendererMemoryPool)

/// Don't create second level command buffers if they contain more drawcalls than this constant.
constexpr U32 kMinDrawcallsPerSecondaryCommandBuffer = 16;

/// Bloom size is rendererSize/kBloomFraction.
constexpr U32 kBloomFraction = 4;

constexpr U32 kMaxDebugRenderTargets = 2;

/// Computes the 'a' and 'b' numbers for linearizeDepthOptimal (see shaders)
inline void computeLinearizeDepthOptimal(F32 near, F32 far, F32& a, F32& b)
{
	a = (near - far) / near;
	b = far / near;
}

constexpr U32 kGBufferColorRenderTargetCount = 4;

/// Downsample and blur down to a texture with size kDownscaleBurDownTo
constexpr U32 kDownscaleBurDownTo = 32;

inline constexpr Array<Format, kGBufferColorRenderTargetCount> kGBufferColorRenderTargetFormats = {
	{Format::kR8G8B8A8_Unorm, Format::kR8G8B8A8_Unorm, Format::kA2B10G10R10_Unorm_Pack32, Format::kR16G16_Snorm}};

/// Rendering context.
class RenderingContext
{
public:
	RenderGraphDescription m_renderGraphDescr;

	CommonMatrices m_matrices;
	CommonMatrices m_prevMatrices;

	F32 m_cameraNear = 0.0f;
	F32 m_cameraFar = 0.0f;

	/// The render target that the Renderer will populate.
	RenderTargetHandle m_outRenderTarget;

	Array<Mat4, kMaxShadowCascades> m_dirLightTextureMatrices;

	RenderingContext(StackMemoryPool* pool)
		: m_renderGraphDescr(pool)
	{
	}

	RenderingContext(const RenderingContext&) = delete;

	RenderingContext& operator=(const RenderingContext&) = delete;
};

/// Choose the detail of a shadow cascade. 0 means high detail and >0 is progressively lower.
inline U32 chooseDirectionalLightShadowCascadeDetail(U32 cascade)
{
	return (cascade <= 1) ? 0 : 1;
}
/// @}

} // end namespace anki

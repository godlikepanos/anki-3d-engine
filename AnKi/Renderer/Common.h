// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr.h>
#include <AnKi/Core/GpuMemoryPools.h>
#include <AnKi/Util/Ptr.h>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>

namespace anki {

#define ANKI_R_LOGI(...) ANKI_LOG("REND", kNormal, __VA_ARGS__)
#define ANKI_R_LOGE(...) ANKI_LOG("REND", kError, __VA_ARGS__)
#define ANKI_R_LOGW(...) ANKI_LOG("REND", kWarning, __VA_ARGS__)
#define ANKI_R_LOGF(...) ANKI_LOG("REND", kFatal, __VA_ARGS__)
#define ANKI_R_LOGV(...) ANKI_LOG("REND", kVerbose, __VA_ARGS__)

// Forward
#define ANKI_RENDERER_OBJECT_DEF(a, b) class a;
#include <AnKi/Renderer/RendererObject.defs.h>
#undef ANKI_RENDERER_OBJECT_DEF

class Renderer;
class RendererObject;
class DebugDrawer;

class RenderQueue;
class RenderableQueueElement;
class PointLightQueueElement;
class DirectionalLightQueueElement;
class SpotLightQueueElement;
class ReflectionProbeQueueElement;
class DecalQueueElement;

class ShaderProgramResourceVariant;

/// @addtogroup renderer
/// @{

/// Don't create second level command buffers if they contain more drawcalls than this constant.
constexpr U32 kMinDrawcallsPerSecondaryCommandBuffer = 16;

/// Bloom size is rendererSize/kBloomFraction.
constexpr U32 kBloomFraction = 4;

/// Used to calculate the mipmap count of the HiZ map.
constexpr U32 hHierachicalZMinHeight = 80;

constexpr TextureSubresourceInfo kHiZHalfSurface(TextureSurfaceInfo(0, 0, 0, 0));
constexpr TextureSubresourceInfo kHiZQuarterSurface(TextureSurfaceInfo(1, 0, 0, 0));

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

/// GPU buffers and textures that the clusterer refers to.
class ClusteredShadingContext
{
public:
	StagingGpuMemoryToken m_pointLightsToken;
	void* m_pointLightsAddress = nullptr;
	StagingGpuMemoryToken m_spotLightsToken;
	void* m_spotLightsAddress = nullptr;
	StagingGpuMemoryToken m_reflectionProbesToken;
	void* m_reflectionProbesAddress = nullptr;
	StagingGpuMemoryToken m_decalsToken;
	void* m_decalsAddress = nullptr;
	StagingGpuMemoryToken m_fogDensityVolumesToken;
	void* m_fogDensityVolumesAddress = nullptr;
	StagingGpuMemoryToken m_globalIlluminationProbesToken;
	void* m_globalIlluminationProbesAddress = nullptr;
	StagingGpuMemoryToken m_clusteredShadingUniformsToken;
	void* m_clusteredShadingUniformsAddress = nullptr;
	StagingGpuMemoryToken m_clustersToken;
	void* m_clustersAddress = nullptr;

	BufferHandle m_clustersBufferHandle; ///< To track dependencies. Don't track all tokens, not worth it.

	TextureViewPtr m_diffuseDecalTextureView;
	TextureViewPtr m_specularRoughnessDecalTextureView;
};

/// Rendering context.
class RenderingContext
{
public:
	StackMemoryPool* m_tempPool = nullptr;
	RenderQueue* m_renderQueue = nullptr;

	RenderGraphDescription m_renderGraphDescr;

	CommonMatrices m_matrices;
	CommonMatrices m_prevMatrices;

	/// The render target that the Renderer will populate.
	RenderTargetHandle m_outRenderTarget;

	ClusteredShadingContext m_clusteredShading;

	RenderingContext(StackMemoryPool* pool)
		: m_tempPool(pool)
		, m_renderGraphDescr(pool)
	{
	}

	RenderingContext(const RenderingContext&) = delete;

	RenderingContext& operator=(const RenderingContext&) = delete;
};

/// A convenience function to find empty cache entries. Used for various probes.
template<typename THashMap, typename TCacheEntryArray, typename TMemPool>
U32 findBestCacheEntry(U64 uuid, Timestamp crntTimestamp, const TCacheEntryArray& entries, THashMap& map,
					   TMemPool& pool)
{
	ANKI_ASSERT(uuid > 0);

	// First, try to see if the UUID is in the cache
	auto it = map.find(uuid);
	if(ANKI_LIKELY(it != map.getEnd()))
	{
		const U32 cacheEntryIdx = *it;
		if(ANKI_LIKELY(entries[cacheEntryIdx].m_uuid == uuid))
		{
			// Found it
			return cacheEntryIdx;
		}
		else
		{
			// Cache entry is wrong, remove it
			map.erase(pool, it);
		}
	}

	// 2nd and 3rd choice, find an empty entry or some entry to re-use
	U32 emptyCacheEntryIdx = kMaxU32;
	U32 cacheEntryIdxToKick = kMaxU32;
	Timestamp cacheEntryIdxToKickMinTimestamp = kMaxTimestamp;
	for(U32 cacheEntryIdx = 0; cacheEntryIdx < entries.getSize(); ++cacheEntryIdx)
	{
		if(entries[cacheEntryIdx].m_uuid == 0)
		{
			// Found an empty
			emptyCacheEntryIdx = cacheEntryIdx;
			break;
		}
		else if(entries[cacheEntryIdx].m_lastUsedTimestamp != crntTimestamp
				&& entries[cacheEntryIdx].m_lastUsedTimestamp < cacheEntryIdxToKickMinTimestamp)
		{
			// Found some with low timestamp
			cacheEntryIdxToKick = cacheEntryIdx;
			cacheEntryIdxToKickMinTimestamp = entries[cacheEntryIdx].m_lastUsedTimestamp;
		}
	}

	U32 outCacheEntryIdx;
	if(emptyCacheEntryIdx != kMaxU32)
	{
		outCacheEntryIdx = emptyCacheEntryIdx;
	}
	else if(cacheEntryIdxToKick != kMaxU32)
	{
		outCacheEntryIdx = cacheEntryIdxToKick;
	}
	else
	{
		// We are out of cache entries. Return OOM
		outCacheEntryIdx = kMaxU32;
	}

	return outCacheEntryIdx;
}
/// @}

} // end namespace anki

// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr.h>
#include <AnKi/Core/StagingGpuMemoryManager.h>
#include <AnKi/Util/Ptr.h>
#include <AnKi/Shaders/Include/Evsm.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>

namespace anki {

#define ANKI_R_LOGI(...) ANKI_LOG("R   ", NORMAL, __VA_ARGS__)
#define ANKI_R_LOGE(...) ANKI_LOG("R   ", ERROR, __VA_ARGS__)
#define ANKI_R_LOGW(...) ANKI_LOG("R   ", WARNING, __VA_ARGS__)
#define ANKI_R_LOGF(...) ANKI_LOG("R   ", FATAL, __VA_ARGS__)

// Forward
#define ANKI_RENDERER_OBJECT_DEF(a, b) class a;
#include <AnKi/Renderer/RendererObjectDefs.h>
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
constexpr U32 MIN_DRAWCALLS_PER_2ND_LEVEL_COMMAND_BUFFER = 16;

/// SSAO size is rendererSize/SSAO_FRACTION.
constexpr U32 SSAO_FRACTION = 2;

/// Bloom size is rendererSize/BLOOM_FRACTION.
constexpr U32 BLOOM_FRACTION = 4;

/// Volumetric size is rendererSize/VOLUMETRIC_FRACTION.
constexpr U32 VOLUMETRIC_FRACTION = 4;

/// Used to calculate the mipmap count of the HiZ map.
constexpr U32 HIERARCHICAL_Z_MIN_HEIGHT = 80;

const TextureSubresourceInfo HIZ_HALF_DEPTH(TextureSurfaceInfo(0, 0, 0, 0));
const TextureSubresourceInfo HIZ_QUARTER_DEPTH(TextureSurfaceInfo(1, 0, 0, 0));

/// Computes the 'a' and 'b' numbers for linearizeDepthOptimal (see shaders)
inline void computeLinearizeDepthOptimal(F32 near, F32 far, F32& a, F32& b)
{
	a = (near - far) / near;
	b = far / near;
}

constexpr U32 GBUFFER_COLOR_ATTACHMENT_COUNT = 4;

/// Downsample and blur down to a texture with size DOWNSCALE_BLUR_DOWN_TO
constexpr U32 DOWNSCALE_BLUR_DOWN_TO = 32;

/// Use this size of render target for the avg lum calculation.
constexpr U32 AVERAGE_LUMINANCE_RENDER_TARGET_SIZE = 128;

extern const Array<Format, GBUFFER_COLOR_ATTACHMENT_COUNT> GBUFFER_COLOR_ATTACHMENT_PIXEL_FORMATS;

constexpr Format GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT = Format::D32_SFLOAT;

constexpr Format LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT = Format::B10G11R11_UFLOAT_PACK32;

constexpr Format FORWARD_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT = Format::R16G16B16A16_SFLOAT;

constexpr Format DBG_COLOR_ATTACHMENT_PIXEL_FORMAT = Format::R8G8B8A8_UNORM;

constexpr Format SHADOW_DEPTH_PIXEL_FORMAT = Format::D32_SFLOAT;
#if ANKI_EVSM4
constexpr Format SHADOW_COLOR_PIXEL_FORMAT = Format::R32G32B32A32_SFLOAT;
#else
constexpr Format SHADOW_COLOR_PIXEL_FORMAT = Format::R32G32_SFLOAT;
#endif

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
	StackAllocator<U8> m_tempAllocator;
	RenderQueue* m_renderQueue = nullptr;

	RenderGraphDescription m_renderGraphDescr;

	CommonMatrices m_matrices;
	CommonMatrices m_prevMatrices;

	/// The render target that the Renderer will populate.
	RenderTargetHandle m_outRenderTarget;
	U32 m_outRenderTargetWidth = 0;
	U32 m_outRenderTargetHeight = 0;

	ClusteredShadingContext m_clusteredShading;

	RenderingContext(const StackAllocator<U8>& alloc)
		: m_tempAllocator(alloc)
		, m_renderGraphDescr(alloc)
	{
	}

	RenderingContext(const RenderingContext&) = delete;

	RenderingContext& operator=(const RenderingContext&) = delete;
};

/// A convenience function to find empty cache entries. Used for various probes.
template<typename THashMap, typename TCacheEntryArray, typename TAlloc>
U32 findBestCacheEntry(U64 uuid, Timestamp crntTimestamp, const TCacheEntryArray& entries, THashMap& map, TAlloc alloc)
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
			map.erase(alloc, it);
		}
	}

	// 2nd and 3rd choice, find an empty entry or some entry to re-use
	U32 emptyCacheEntryIdx = MAX_U32;
	U32 cacheEntryIdxToKick = MAX_U32;
	Timestamp cacheEntryIdxToKickMinTimestamp = MAX_TIMESTAMP;
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
	if(emptyCacheEntryIdx != MAX_U32)
	{
		outCacheEntryIdx = emptyCacheEntryIdx;
	}
	else if(cacheEntryIdxToKick != MAX_U32)
	{
		outCacheEntryIdx = cacheEntryIdxToKick;
	}
	else
	{
		// We are out of cache entries. Return OOM
		outCacheEntryIdx = MAX_U32;
	}

	return outCacheEntryIdx;
}
/// @}

} // end namespace anki

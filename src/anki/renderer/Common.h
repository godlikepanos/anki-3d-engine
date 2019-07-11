// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Gr.h>
#include <anki/core/StagingGpuMemoryManager.h>
#include <anki/util/Ptr.h>

namespace anki
{

#define ANKI_R_LOGI(...) ANKI_LOG("R   ", NORMAL, __VA_ARGS__)
#define ANKI_R_LOGE(...) ANKI_LOG("R   ", ERROR, __VA_ARGS__)
#define ANKI_R_LOGW(...) ANKI_LOG("R   ", WARNING, __VA_ARGS__)
#define ANKI_R_LOGF(...) ANKI_LOG("R   ", FATAL, __VA_ARGS__)

// Forward
class Renderer;
class GBuffer;
class GBufferPost;
class ShadowMapping;
class LightShading;
class ForwardShading;
class LensFlare;
class Ssao;
class Tonemapping;
class Bloom;
class FinalComposite;
class Dbg;
class ProbeReflections;
class DownscaleBlur;
class VolumetricFog;
class DepthDownscale;
class TemporalAA;
class UiStage;
class Ssr;
class VolumetricLightingAccumulation;
class GlobalIllumination;
class GenericCompute;

class RenderingContext;
class DebugDrawer;

class RenderQueue;
class RenderableQueueElement;
class PointLightQueueElement;
class DirectionalLightQueueElement;
class SpotLightQueueElement;
class ReflectionProbeQueueElement;
class DecalQueueElement;

class ShaderProgramResourceVariant;
class ClusterBin;

/// @addtogroup renderer
/// @{

/// Don't create second level command buffers if they contain more drawcalls than this constant.
const U MIN_DRAWCALLS_PER_2ND_LEVEL_COMMAND_BUFFER = 16;

/// FS size is rendererSize/FS_FRACTION.
const U FS_FRACTION = 2;

/// SSAO size is rendererSize/SSAO_FRACTION.
const U SSAO_FRACTION = 2;

/// Bloom size is rendererSize/BLOOM_FRACTION.
const U BLOOM_FRACTION = 4;

/// Volumetric size is rendererSize/VOLUMETRIC_FRACTION.
const U VOLUMETRIC_FRACTION = 4;

/// SSR size is rendererSize/SSR_FRACTION.
const U SSR_FRACTION = 2;

/// Used to calculate the mipmap count of the HiZ map.
const U HIERARCHICAL_Z_MIN_HEIGHT = 80;

const TextureSubresourceInfo HIZ_HALF_DEPTH(TextureSurfaceInfo(0, 0, 0, 0));
const TextureSubresourceInfo HIZ_QUARTER_DEPTH(TextureSurfaceInfo(1, 0, 0, 0));

/// Computes the 'a' and 'b' numbers for linearizeDepthOptimal (see shaders)
inline void computeLinearizeDepthOptimal(F32 near, F32 far, F32& a, F32& b)
{
	a = (near - far) / near;
	b = far / near;
}

const U GBUFFER_COLOR_ATTACHMENT_COUNT = 4;

/// Downsample and blur down to a texture with size DOWNSCALE_BLUR_DOWN_TO
const U DOWNSCALE_BLUR_DOWN_TO = 32;

/// Use this size of render target for the avg lum calculation.
const U AVERAGE_LUMINANCE_RENDER_TARGET_SIZE = 128;

extern const Array<Format, GBUFFER_COLOR_ATTACHMENT_COUNT> GBUFFER_COLOR_ATTACHMENT_PIXEL_FORMATS;

const Format GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT = Format::D32_SFLOAT;

const Format LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT = Format::B10G11R11_UFLOAT_PACK32;

const Format FORWARD_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT = Format::R16G16B16A16_SFLOAT;

const Format DBG_COLOR_ATTACHMENT_PIXEL_FORMAT = Format::R8G8B8A8_UNORM;

const Format SHADOW_DEPTH_PIXEL_FORMAT = Format::D32_SFLOAT;
const Format SHADOW_COLOR_PIXEL_FORMAT = Format::R16_UNORM;

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

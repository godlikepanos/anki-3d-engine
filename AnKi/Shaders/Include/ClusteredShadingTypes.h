// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/GpuSceneTypes.h>

ANKI_BEGIN_NAMESPACE

// Limits
constexpr U32 kMaxVisibleLights = 32u * 4u;
constexpr U32 kMaxVisibleDecals = 32u * 4u;
constexpr U32 kMaxVisibleFogDensityVolumes = 16u;
constexpr U32 kMaxVisibleReflectionProbes = 16u;
constexpr U32 kMaxVisibleGlobalIlluminationProbes = 8u;

// Other consts
constexpr F32 kClusterObjectFrustumNearPlane = 0.1f / 4.0f; ///< Near plane of all clusterer object frustums.
constexpr F32 kSubsurfaceMin = 0.01f;
constexpr U32 kMaxZsplitCount = 128u;
constexpr U32 kClusteredShadingTileSize = 64; ///< The size of the tile in clustered shading.

// Information that a tile or a Z-split will contain.
struct Cluster
{
	U32 m_pointLightsMask[kMaxVisibleLights / 32];
	U32 m_spotLightsMask[kMaxVisibleLights / 32];
	U32 m_decalsMask[kMaxVisibleDecals / 32];
	U32 m_fogDensityVolumesMask;
	U32 m_reflectionProbesMask;
	U32 m_giProbesMask;
};

#if defined(__cplusplus)
constexpr Array<U32, U32(GpuSceneNonRenderableObjectType::kCount)> kClusteredObjectSizes
#else
constexpr U32 kClusteredObjectSizes[(U32)GpuSceneNonRenderableObjectType::kCount]
#endif
	= {sizeof(GpuSceneLight), sizeof(GpuSceneDecal), sizeof(GpuSceneFogDensityVolume), sizeof(GpuSceneReflectionProbe),
	   sizeof(GpuSceneGlobalIlluminationProbe)};

#if defined(__cplusplus)
constexpr Array<U32, U32(GpuSceneNonRenderableObjectType::kCount)> kMaxVisibleClusteredObjects
#else
constexpr U32 kMaxVisibleClusteredObjects[(U32)GpuSceneNonRenderableObjectType::kCount]
#endif
	= {kMaxVisibleLights, kMaxVisibleDecals, kMaxVisibleFogDensityVolumes, kMaxVisibleReflectionProbes, kMaxVisibleGlobalIlluminationProbes};

ANKI_END_NAMESPACE

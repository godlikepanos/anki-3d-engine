// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/LightFunctions.hlsl>

//
// Common uniforms
//
#if defined(CLUSTERED_SHADING_UNIFORMS_BINDING)
[[vk::binding(CLUSTERED_SHADING_UNIFORMS_BINDING, CLUSTERED_SHADING_SET)]] ConstantBuffer<ClusteredShadingUniforms>
	u_clusteredShading;
#endif

//
// Light uniforms (3)
//
#if defined(CLUSTERED_SHADING_LIGHTS_BINDING)
[[vk::binding(CLUSTERED_SHADING_LIGHTS_BINDING, CLUSTERED_SHADING_SET)]] cbuffer b_pointLights
{
	PointLight u_pointLights2[kMaxVisiblePointLights];
};

[[vk::binding(CLUSTERED_SHADING_LIGHTS_BINDING + 1u, CLUSTERED_SHADING_SET)]] cbuffer b_spotLights
{
	SpotLight u_spotLights[kMaxVisibleSpotLights];
};

[[vk::binding(CLUSTERED_SHADING_LIGHTS_BINDING + 2u, CLUSTERED_SHADING_SET)]] Texture2D u_shadowAtlasTex;
#endif

//
// Reflection probes (3)
//
#if defined(CLUSTERED_SHADING_REFLECTIONS_BINDING)
[[vk::binding(CLUSTERED_SHADING_REFLECTIONS_BINDING, CLUSTERED_SHADING_SET)]] cbuffer b_reflectionProbes
{
	ReflectionProbe u_reflectionProbes[kMaxVisibleReflectionProbes];
};

[[vk::binding(CLUSTERED_SHADING_REFLECTIONS_BINDING + 1u, CLUSTERED_SHADING_SET)]] TextureCubeArray<RVec4>
	u_reflectionsTex;
#endif

//
// Decal uniforms (3)
//
#if defined(CLUSTERED_SHADING_DECALS_BINDING)
[[vk::binding(CLUSTERED_SHADING_DECALS_BINDING, CLUSTERED_SHADING_SET)]] cbuffer b_decals
{
	Decal u_decals2[kMaxVisibleDecals];
};

[[vk::binding(CLUSTERED_SHADING_DECALS_BINDING + 1u, CLUSTERED_SHADING_SET)]] Texture2D<RVec4> u_diffuseDecalTex;
[[vk::binding(CLUSTERED_SHADING_DECALS_BINDING + 2u, CLUSTERED_SHADING_SET)]] Texture2D<RVec4>
	u_specularRoughnessDecalTex;
#endif

//
// Fog density uniforms (1)
//
#if defined(CLUSTERED_SHADING_FOG_BINDING)
[[vk::binding(CLUSTERED_SHADING_FOG_BINDING, CLUSTERED_SHADING_SET)]] cbuffer b_fogDensityVolumes
{
	FogDensityVolume u_fogDensityVolumes[kMaxVisibleFogDensityVolumes];
};
#endif

//
// GI (2)
//
#if defined(CLUSTERED_SHADING_GI_BINDING)
[[vk::binding(CLUSTERED_SHADING_GI_BINDING, CLUSTERED_SHADING_SET)]] Texture3D<RVec4>
	u_globalIlluminationTextures[kMaxVisibleGlobalIlluminationProbes];

[[vk::binding(CLUSTERED_SHADING_GI_BINDING + 1u, CLUSTERED_SHADING_SET)]] cbuffer b_giProbes
{
	GlobalIlluminationProbe u_giProbes[kMaxVisibleGlobalIlluminationProbes];
};
#endif

//
// Cluster uniforms
//
#if defined(CLUSTERED_SHADING_CLUSTERS_BINDING)
[[vk::binding(CLUSTERED_SHADING_CLUSTERS_BINDING, CLUSTERED_SHADING_SET)]] StructuredBuffer<Cluster> u_clusters;
#endif

// Debugging function
Vec3 clusterHeatmap(Cluster cluster, U32 objectTypeMask)
{
	U32 maxObjects = 0u;
	I32 count = 0;

	if((objectTypeMask & (1u << kClusterObjectTypePointLight)) != 0u)
	{
		maxObjects += kMaxVisiblePointLights;
		count += I32(countbits(cluster.m_pointLightsMask));
	}

	if((objectTypeMask & (1u << kClusterObjectTypeSpotLight)) != 0u)
	{
		maxObjects += kMaxVisibleSpotLights;
		count += I32(countbits(cluster.m_spotLightsMask));
	}

	if((objectTypeMask & (1u << kClusterObjectTypeDecal)) != 0u)
	{
		maxObjects += kMaxVisibleDecals;
		count += I32(countbits(cluster.m_decalsMask));
	}

	if((objectTypeMask & (1u << kClusterObjectTypeFogDensityVolume)) != 0u)
	{
		maxObjects += kMaxVisibleFogDensityVolumes;
		count += countbits(cluster.m_fogDensityVolumesMask);
	}

	if((objectTypeMask & (1u << kClusterObjectTypeReflectionProbe)) != 0u)
	{
		maxObjects += kMaxVisibleReflectionProbes;
		count += countbits(cluster.m_reflectionProbesMask);
	}

	if((objectTypeMask & (1u << kClusterObjectTypeGlobalIlluminationProbe)) != 0u)
	{
		maxObjects += kMaxVisibleGlobalIlluminationProbes;
		count += countbits(cluster.m_giProbesMask);
	}

	const F32 factor = min(1.0, F32(count) / F32(maxObjects));
	return heatmap(factor);
}

/// Returns the index of the zSplit or linearizeDepth(n, f, depth)*zSplitCount
/// Simplifying this equation is 1/(a+b/depth) where a=(n-f)/(n*zSplitCount) and b=f/(n*zSplitCount)
U32 computeZSplitClusterIndex(F32 depth, U32 zSplitCount, F32 a, F32 b)
{
	const F32 fSplitIdx = 1.0 / (a + b / depth);
	return min(zSplitCount - 1u, (U32)fSplitIdx);
}

/// Return the tile index.
U32 computeTileClusterIndexFragCoord(Vec2 fragCoord, U32 tileSize, U32 tileCountX)
{
	const UVec2 tileXY = UVec2(fragCoord / (F32)tileSize);
	return tileXY.y * tileCountX + tileXY.x;
}

/// Merge the tiles with z splits into a single cluster.
Cluster mergeClusters(Cluster tileCluster, Cluster zCluster)
{
//#define ANKI_OR_MASKS(x) subgroupOr(x)
#define ANKI_OR_MASKS(x) (x)

	Cluster outCluster;
	outCluster.m_pointLightsMask = ANKI_OR_MASKS(tileCluster.m_pointLightsMask & zCluster.m_pointLightsMask);
	outCluster.m_spotLightsMask = ANKI_OR_MASKS(tileCluster.m_spotLightsMask & zCluster.m_spotLightsMask);
	outCluster.m_decalsMask = ANKI_OR_MASKS(tileCluster.m_decalsMask & zCluster.m_decalsMask);
	outCluster.m_fogDensityVolumesMask =
		ANKI_OR_MASKS(tileCluster.m_fogDensityVolumesMask & zCluster.m_fogDensityVolumesMask);
	outCluster.m_reflectionProbesMask =
		ANKI_OR_MASKS(tileCluster.m_reflectionProbesMask & zCluster.m_reflectionProbesMask);
	outCluster.m_giProbesMask = ANKI_OR_MASKS(tileCluster.m_giProbesMask & zCluster.m_giProbesMask);

#undef ANKI_OR_MASKS

	return outCluster;
}

#if defined(CLUSTERED_SHADING_CLUSTERS_BINDING)
/// Get the final cluster after ORing and ANDing the masks.
Cluster getClusterFragCoord(Vec3 fragCoord, U32 tileSize, UVec2 tileCounts, U32 zSplitCount, F32 a, F32 b)
{
	const Cluster tileCluster = u_clusters[computeTileClusterIndexFragCoord(fragCoord.xy, tileSize, tileCounts.x)];
	const Cluster zCluster =
		u_clusters[computeZSplitClusterIndex(fragCoord.z, zSplitCount, a, b) + tileCounts.x * tileCounts.y];
	return mergeClusters(tileCluster, zCluster);
}

Cluster getClusterFragCoord(Vec3 fragCoord)
{
	return getClusterFragCoord(fragCoord, u_clusteredShading.m_tileSize, u_clusteredShading.m_tileCounts,
							   u_clusteredShading.m_zSplitCount, u_clusteredShading.m_zSplitMagic.x,
							   u_clusteredShading.m_zSplitMagic.y);
}
#endif

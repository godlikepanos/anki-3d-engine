// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/LightFunctions2.glsl>

//
// Common uniforms
//
#if defined(CLUSTERED_SHADING_UNIFORMS_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTERED_SHADING_UNIFORMS_BINDING, scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	ClusteredShadingUniforms u_clusteredShading;
};
#endif

//
// Light uniforms (3)
//
#if defined(CLUSTERED_SHADING_LIGHTS_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTERED_SHADING_LIGHTS_BINDING, scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	PointLight2 u_pointLights2[MAX_VISIBLE_POINT_LIGHTS];
};

layout(set = CLUSTERED_SHADING_SET, binding = CLUSTERED_SHADING_LIGHTS_BINDING + 1,
	   scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	SpotLight2 u_spotLights2[MAX_VISIBLE_SPOT_LIGHTS];
};

layout(set = CLUSTERED_SHADING_SET, binding = CLUSTERED_SHADING_LIGHTS_BINDING + 2) uniform texture2D u_shadowAtlasTex;
#endif

//
// Reflection probes (3)
//
#if defined(CLUSTERED_SHADING_REFLECTIONS_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTERED_SHADING_REFLECTIONS_BINDING,
	   scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	ReflectionProbe2 u_reflectionProbes2[MAX_VISIBLE_REFLECTION_PROBES];
};

layout(set = CLUSTERED_SHADING_SET,
	   binding = CLUSTERED_SHADING_REFLECTIONS_BINDING + 1) uniform textureCubeArray u_reflectionsTex2;
layout(set = CLUSTERED_SHADING_SET,
	   binding = CLUSTERED_SHADING_REFLECTIONS_BINDING + 2) uniform texture2D u_integrationLut2;
#endif

//
// Decal uniforms (3)
//
#if defined(CLUSTERED_SHADING_DECALS_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTERED_SHADING_DECALS_BINDING, scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	Decal2 u_decals2[MAX_VISIBLE_DECALS];
};

layout(set = CLUSTERED_SHADING_SET, binding = CLUSTERED_SHADING_DECALS_BINDING + 1) uniform texture2D u_diffuseDecalTex;
layout(set = CLUSTERED_SHADING_SET,
	   binding = CLUSTERED_SHADING_DECALS_BINDING + 2) uniform texture2D u_specularRoughnessDecalTex;
#endif

//
// Fog density uniforms (1)
//
#if defined(CLUSTERED_SHADING_FOG_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTERED_SHADING_FOG_BINDING, scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	FogDensityVolume2 u_fogDensityVolumes[MAX_VISIBLE_FOG_DENSITY_VOLUMES];
};
#endif

//
// GI (2)
//
#if defined(CLUSTERED_SHADING_GI_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTERED_SHADING_GI_BINDING) uniform texture3D
	u_globalIlluminationTextures[MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES2];

layout(set = CLUSTERED_SHADING_SET, binding = CLUSTERED_SHADING_GI_BINDING + 1, scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	GlobalIlluminationProbe2 u_giProbes[MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES2];
};
#endif

//
// Cluster uniforms
//
#if defined(CLUSTERED_SHADING_CLUSTERS_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTERED_SHADING_CLUSTERS_BINDING,
	   scalar) readonly buffer ANKI_RANDOM_BLOCK_NAME
{
	Cluster u_clusters2[];
};
#endif

// Debugging function
Vec3 clusterHeatmap(Cluster cluster, U32 objectTypeMask)
{
	U32 maxObjects = 0u;
	I32 count = 0;

	if((objectTypeMask & (1u << CLUSTER_OBJECT_TYPE_POINT_LIGHT)) != 0)
	{
		maxObjects += MAX_VISIBLE_POINT_LIGHTS;
		count += I32(bitCount(cluster.m_pointLightsMask));
	}

	if((objectTypeMask & (1u << CLUSTER_OBJECT_TYPE_SPOT_LIGHT)) != 0)
	{
		maxObjects += MAX_VISIBLE_SPOT_LIGHTS;
		count += I32(bitCount(cluster.m_spotLightsMask));
	}

	if((objectTypeMask & (1u << CLUSTER_OBJECT_TYPE_DECAL)) != 0)
	{
		maxObjects += MAX_VISIBLE_DECALS;
		count += I32(bitCount(cluster.m_decalsMask));
	}

	if((objectTypeMask & (1u << CLUSTER_OBJECT_TYPE_FOG_DENSITY_VOLUME)) != 0)
	{
		maxObjects += MAX_VISIBLE_FOG_DENSITY_VOLUMES;
		count += bitCount(cluster.m_fogDensityVolumesMask);
	}

	if((objectTypeMask & (1u << CLUSTER_OBJECT_TYPE_REFLECTION_PROBE)) != 0)
	{
		maxObjects += MAX_VISIBLE_REFLECTION_PROBES;
		count += bitCount(cluster.m_reflectionProbesMask);
	}

	if((objectTypeMask & (1u << CLUSTER_OBJECT_TYPE_GLOBAL_ILLUMINATION_PROBE)) != 0)
	{
		maxObjects += MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES2;
		count += bitCount(cluster.m_giProbesMask);
	}

	const F32 factor = min(1.0, F32(count) / F32(maxObjects));
	return heatmap(factor);
}

/// Returns the index of the zSplit or linearizeDepth(n, f, depth)*zSplitCount
/// Simplifying this equation is 1/(a+b/depth) where a=(n-f)/(n*zSplitCount) and b=f/(n*zSplitCount)
U32 computeZSplitClusterIndex(F32 depth, U32 zSplitCount, F32 a, F32 b)
{
	const F32 fSplitIdx = 1.0 / (a + b / depth);
	return min(zSplitCount - 1u, U32(fSplitIdx));
}

/// Return the tile index.
U32 computeTileClusterIndexUv(Vec2 uv, UVec2 tileCounts)
{
	const UVec2 tileXY = UVec2(uv * Vec2(tileCounts));
	return tileXY.y * tileCounts.x + tileXY.x;
}

/// Return the tile index.
U32 computeTileClusterIndexFragCoord(Vec2 fragCoord, U32 tileSize, U32 tileCountX)
{
	const UVec2 tileXY = UVec2(fragCoord / F32(tileSize));
	return tileXY.y * tileCountX + tileXY.x;
}

/// Merge the tiles with z splits into a single cluster.
Cluster mergeClusters(Cluster tileCluster, Cluster zCluster)
{
	Cluster outCluster;
	outCluster.m_pointLightsMask = subgroupOr(tileCluster.m_pointLightsMask & zCluster.m_pointLightsMask);
	outCluster.m_spotLightsMask = subgroupOr(tileCluster.m_spotLightsMask & zCluster.m_spotLightsMask);
	outCluster.m_decalsMask = subgroupOr(tileCluster.m_decalsMask & zCluster.m_decalsMask);
	outCluster.m_fogDensityVolumesMask =
		subgroupOr(tileCluster.m_fogDensityVolumesMask & zCluster.m_fogDensityVolumesMask);
	outCluster.m_reflectionProbesMask =
		subgroupOr(tileCluster.m_reflectionProbesMask & zCluster.m_reflectionProbesMask);
	outCluster.m_giProbesMask = subgroupOr(tileCluster.m_giProbesMask & zCluster.m_giProbesMask);

	return outCluster;
}

#if defined(CLUSTERED_SHADING_CLUSTERS_BINDING)
/// Get the final cluster after ORing and ANDing the masks.
Cluster getClusterFragCoord(Vec3 fragCoord, U32 tileSize, UVec2 tileCounts, U32 zSplitCount, F32 a, F32 b)
{
	const Cluster tileCluster = u_clusters2[computeTileClusterIndexFragCoord(fragCoord.xy, tileSize, tileCounts.x)];
	const Cluster zCluster =
		u_clusters2[computeZSplitClusterIndex(fragCoord.z, zSplitCount, a, b) + tileCounts.x * tileCounts.y];
	return mergeClusters(tileCluster, zCluster);
}

Cluster getClusterFragCoord(Vec3 fragCoord)
{
	return getClusterFragCoord(fragCoord, u_clusteredShading.m_tileSize, u_clusteredShading.m_tileCounts,
							   u_clusteredShading.m_zSplitCount, u_clusteredShading.m_zSplitMagic.x,
							   u_clusteredShading.m_zSplitMagic.y);
}

Cluster getClusterUv(Vec2 uv, F32 depth, UVec2 tileCounts, U32 zSplitCount, F32 a, F32 b)
{
	const Cluster tileCluster = u_clusters2[computeTileClusterIndexUv(uv, tileCounts)];
	const Cluster zCluster =
		u_clusters2[computeZSplitClusterIndex(depth, zSplitCount, a, b) + tileCounts.x * tileCounts.y];
	return mergeClusters(tileCluster, zCluster);
}
#endif

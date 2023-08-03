// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/LightFunctions.hlsl>

// Debugging function
Vec3 clusterHeatmap(Cluster cluster, U32 objectTypeMask)
{
	U32 maxObjects = 0u;
	I32 count = 0;

	if((objectTypeMask & (1u << (U32)ClusteredObjectType::kPointLight)) != 0u)
	{
		maxObjects += kMaxVisiblePointLights;
		count += I32(countbits(cluster.m_pointLightsMask));
	}

	if((objectTypeMask & (1u << (U32)ClusteredObjectType::kSpotLight)) != 0u)
	{
		maxObjects += kMaxVisibleSpotLights;
		count += I32(countbits(cluster.m_spotLightsMask));
	}

	if((objectTypeMask & (1u << (U32)ClusteredObjectType::kDecal)) != 0u)
	{
		maxObjects += kMaxVisibleDecals;
		count += I32(countbits(cluster.m_decalsMask));
	}

	if((objectTypeMask & (1u << (U32)ClusteredObjectType::kFogDensityVolume)) != 0u)
	{
		maxObjects += kMaxVisibleFogDensityVolumes;
		count += countbits(cluster.m_fogDensityVolumesMask);
	}

	if((objectTypeMask & (1u << (U32)ClusteredObjectType::kReflectionProbe)) != 0u)
	{
		maxObjects += kMaxVisibleReflectionProbes;
		count += countbits(cluster.m_reflectionProbesMask);
	}

	if((objectTypeMask & (1u << (U32)ClusteredObjectType::kGlobalIlluminationProbe)) != 0u)
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
	const UVec2 tileXY = UVec2(fragCoord / F32(tileSize));
	return tileXY.y * tileCountX + tileXY.x;
}

/// Merge the tiles with z splits into a single cluster.
Cluster mergeClusters(Cluster tileCluster, Cluster zCluster)
{
// #define ANKI_OR_MASKS(x) subgroupOr(x)
#define ANKI_OR_MASKS(x) (x)

	Cluster outCluster;
	outCluster.m_pointLightsMask = ANKI_OR_MASKS(tileCluster.m_pointLightsMask & zCluster.m_pointLightsMask);
	outCluster.m_spotLightsMask = ANKI_OR_MASKS(tileCluster.m_spotLightsMask & zCluster.m_spotLightsMask);
	outCluster.m_decalsMask = ANKI_OR_MASKS(tileCluster.m_decalsMask & zCluster.m_decalsMask);
	outCluster.m_fogDensityVolumesMask = ANKI_OR_MASKS(tileCluster.m_fogDensityVolumesMask & zCluster.m_fogDensityVolumesMask);
	outCluster.m_reflectionProbesMask = ANKI_OR_MASKS(tileCluster.m_reflectionProbesMask & zCluster.m_reflectionProbesMask);
	outCluster.m_giProbesMask = ANKI_OR_MASKS(tileCluster.m_giProbesMask & zCluster.m_giProbesMask);

#undef ANKI_OR_MASKS

	return outCluster;
}

/// Get the final cluster after ORing and ANDing the masks.
Cluster getClusterFragCoord(StructuredBuffer<Cluster> clusters, Vec3 fragCoord, U32 tileSize, UVec2 tileCounts, U32 zSplitCount, F32 a, F32 b)
{
	const Cluster tileCluster = clusters[computeTileClusterIndexFragCoord(fragCoord.xy, tileSize, tileCounts.x)];
	const Cluster zCluster = clusters[computeZSplitClusterIndex(fragCoord.z, zSplitCount, a, b) + tileCounts.x * tileCounts.y];
	return mergeClusters(tileCluster, zCluster);
}

Cluster getClusterFragCoord(StructuredBuffer<Cluster> clusters, ClusteredShadingUniforms unis, Vec3 fragCoord)
{
	return getClusterFragCoord(clusters, fragCoord, unis.m_tileSize, unis.m_tileCounts, unis.m_zSplitCount, unis.m_zSplitMagic.x,
							   unis.m_zSplitMagic.y);
}

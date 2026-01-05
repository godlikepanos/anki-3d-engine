// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/LightFunctions.hlsl>

// Debugging function
Vec3 clusterHeatmap(Cluster cluster, U32 objectTypeMask, U32 maxObjectOverride = 0)
{
	U32 maxObjects = 0u;
	I32 count = 0;

	if((objectTypeMask & (1u << (U32)GpuSceneNonRenderableObjectType::kLight)) != 0u)
	{
		maxObjects += kMaxVisibleLights;
		for(U32 i = 0; i < kMaxVisibleLights / 32; ++i)
		{
			count += I32(countbits(cluster.m_pointLightsMask[i] | cluster.m_spotLightsMask[i]));
		}
	}

	if((objectTypeMask & (1u << (U32)GpuSceneNonRenderableObjectType::kDecal)) != 0u)
	{
		maxObjects += kMaxVisibleDecals;
		for(U32 i = 0; i < kMaxVisibleDecals / 32; ++i)
		{
			count += I32(countbits(cluster.m_decalsMask[i]));
		}
	}

	if((objectTypeMask & (1u << (U32)GpuSceneNonRenderableObjectType::kFogDensityVolume)) != 0u)
	{
		maxObjects += kMaxVisibleFogDensityVolumes;
		count += countbits(cluster.m_fogDensityVolumesMask);
	}

	if((objectTypeMask & (1u << (U32)GpuSceneNonRenderableObjectType::kReflectionProbe)) != 0u)
	{
		maxObjects += kMaxVisibleReflectionProbes;
		count += countbits(cluster.m_reflectionProbesMask);
	}

	if((objectTypeMask & (1u << (U32)GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe)) != 0u)
	{
		maxObjects += kMaxVisibleGlobalIlluminationProbes;
		count += countbits(cluster.m_giProbesMask);
	}

	const F32 factor = min(1.0, F32(count) / F32((maxObjectOverride > 0) ? maxObjectOverride : maxObjects));
	return heatmap(factor);
}

// Returns the index of the zSplit. Calculated as linearizeDepth(depth, n, f)*(f-n)/(clustererFar-n)*zSplitCount
// Simplifying this equation is 1/(a+b/depth) where a=(clustererFar-n)/(-n*zSplitCount) and b=f*(clustererFar-n)/(n*(f-n)*zSplitCount)
// If the depth is outside the clusterer's range then the return value will be creater or equal to zSplitCount
U32 computeZSplitClusterIndex(F32 depth, F32 a, F32 b)
{
	const F32 splitIdxf = 1.0 / (a + b / depth); // It's fine if depth is zero. The splitIdxf will become 0.0
	return (U32)splitIdxf;
}

// It's similar to computeZSplitClusterIndex but instead of an index it returns a tex coordinate for the w coord of a 3D texture that covers the
// clusterer. Calculated as linearizeDepth(depth, n, f)*(f-n)/(clustererFar-n).
// Simplifying this equation is 1/(a+b/depth) where a=(clustererFar-n)/(-n) and b=f*(clustererFar-n)/(n*(f-n))
// If the depth is outside the clusterer's range then the return value will be creater or equal than 1.0
F32 computeVolumeWTexCoord(F32 depth, F32 a, F32 b)
{
	return 1.0 / (a + b / depth); // It's fine if depth is zero. The expression will become 0.0
}

// Return the tile index.
U32 computeTileClusterIndexFragCoord(Vec2 fragCoord, U32 tileCountX)
{
	const UVec2 tileXY = UVec2(fragCoord / F32(kClusteredShadingTileSize));
	return tileXY.y * tileCountX + tileXY.x;
}

// Merge the tiles with z splits into a single cluster.
template<Bool kDynamicallyUniform = false>
Cluster mergeClusters(Cluster tileCluster, Cluster zCluster)
{
	Cluster outCluster;

	if(kDynamicallyUniform)
	{
		[unroll] for(U32 i = 0; i < kMaxVisibleLights / 32; ++i)
		{
			outCluster.m_pointLightsMask[i] = WaveActiveBitAnd(tileCluster.m_pointLightsMask[i] & zCluster.m_pointLightsMask[i]);
			outCluster.m_spotLightsMask[i] = WaveActiveBitAnd(tileCluster.m_spotLightsMask[i] & zCluster.m_spotLightsMask[i]);
		}

		[unroll] for(U32 i = 0; i < kMaxVisibleDecals / 32; ++i)
		{
			outCluster.m_decalsMask[i] = WaveActiveBitAnd(tileCluster.m_decalsMask[i] & zCluster.m_decalsMask[i]);
		}

		outCluster.m_fogDensityVolumesMask = WaveActiveBitAnd(tileCluster.m_fogDensityVolumesMask & zCluster.m_fogDensityVolumesMask);
		outCluster.m_reflectionProbesMask = WaveActiveBitAnd(tileCluster.m_reflectionProbesMask & zCluster.m_reflectionProbesMask);
		outCluster.m_giProbesMask = WaveActiveBitAnd(tileCluster.m_giProbesMask & zCluster.m_giProbesMask);
	}
	else
	{
		[unroll] for(U32 i = 0; i < kMaxVisibleLights / 32; ++i)
		{
			outCluster.m_pointLightsMask[i] = (tileCluster.m_pointLightsMask[i] & zCluster.m_pointLightsMask[i]);
			outCluster.m_spotLightsMask[i] = (tileCluster.m_spotLightsMask[i] & zCluster.m_spotLightsMask[i]);
		}

		[unroll] for(U32 i = 0; i < kMaxVisibleDecals / 32; ++i)
		{
			outCluster.m_decalsMask[i] = (tileCluster.m_decalsMask[i] & zCluster.m_decalsMask[i]);
		}

		outCluster.m_fogDensityVolumesMask = (tileCluster.m_fogDensityVolumesMask & zCluster.m_fogDensityVolumesMask);
		outCluster.m_reflectionProbesMask = (tileCluster.m_reflectionProbesMask & zCluster.m_reflectionProbesMask);
		outCluster.m_giProbesMask = (tileCluster.m_giProbesMask & zCluster.m_giProbesMask);
	}

	return outCluster;
}

// Get the final cluster after ORing and ANDing the masks.
template<Bool kDynamicallyUniform = false>
Cluster getClusterFragCoord(StructuredBuffer<Cluster> clusters, ClustererConstants consts, Vec3 fragCoord)
{
	U32 idx = computeTileClusterIndexFragCoord(fragCoord.xy, consts.m_tileCounts.x);
	const Cluster tileCluster = SBUFF(clusters, idx);

	idx = computeZSplitClusterIndex(fragCoord.z, consts.m_zSplitMagic.x, consts.m_zSplitMagic.y);
	idx += consts.m_tileCounts.x * consts.m_tileCounts.y;
	idx = min(idx, consts.m_clusterCount); // The "consts.m_clusterCount" is intentional. There is a hiden cluster at the end that is all zeroes
	const Cluster zCluster = SBUFF(clusters, idx);

	return mergeClusters<kDynamicallyUniform>(tileCluster, zCluster);
}

U32 iteratePointLights(inout Cluster cluster)
{
	for(U32 block = 0; block < kMaxVisibleLights / 32; ++block)
	{
		if(cluster.m_pointLightsMask[block] != 0)
		{
			const U32 idx = (U32)firstbitlow2(cluster.m_pointLightsMask[block]);
			cluster.m_pointLightsMask[block] ^= 1u << idx;
			return idx + block * 32;
		}
	}

	return kMaxU32;
}

U32 iterateSpotLights(inout Cluster cluster)
{
	for(U32 block = 0; block < kMaxVisibleLights / 32; ++block)
	{
		if(cluster.m_spotLightsMask[block] != 0)
		{
			const U32 idx = (U32)firstbitlow2(cluster.m_spotLightsMask[block]);
			cluster.m_spotLightsMask[block] ^= 1u << idx;
			return idx + block * 32;
		}
	}

	return kMaxU32;
}

U32 iterateDecals(inout Cluster cluster)
{
	for(U32 block = 0; block < kMaxVisibleDecals / 32; ++block)
	{
		if(cluster.m_decalsMask[block] != 0)
		{
			const U32 idx = (U32)firstbitlow2(cluster.m_decalsMask[block]);
			cluster.m_decalsMask[block] ^= 1u << idx;
			return idx + block * 32;
		}
	}

	return kMaxU32;
}

template<typename T>
vector<T, 3> sampleReflectionProbes(Cluster cluster, StructuredBuffer<GpuSceneReflectionProbe> probes, Vec3 reflDir, Vec3 worldPos, T reflTexLod,
									SamplerState trilinearClampSampler)
{
	const U32 probeCount = countbits(cluster.m_reflectionProbesMask);
	vector<T, 3> probeColor;

	if(probeCount == 0)
	{
		probeColor = -1.0;
	}
	else if(WaveActiveAllTrue(probeCount == 1))
	{
		// Only one probe, do a fast path without blending probes

		const GpuSceneReflectionProbe probe = probes[firstbitlow2(cluster.m_reflectionProbesMask)];

		// Sample
		Vec3 cubeUv = intersectProbe(worldPos, reflDir, probe.m_aabbMin, probe.m_aabbMax, probe.m_position);
		cubeUv.z = -cubeUv.z;
		probeColor = getBindlessTextureCubeVec4(probe.m_cubeTexture).SampleLevel(trilinearClampSampler, cubeUv, reflTexLod).rgb;
	}
	else
	{
		// More than one probes, do a slow path that blends them together

		probeColor = 0.0;
		T totalBlendWeight = 0.001;

		// Loop probes
		[loop] while(cluster.m_reflectionProbesMask != 0u)
		{
			const U32 idx = U32(firstbitlow2(cluster.m_reflectionProbesMask));
			cluster.m_reflectionProbesMask &= ~(1u << idx);
			const GpuSceneReflectionProbe probe = probes[idx];

			// Compute blend weight
			const T blendWeight = computeProbeBlendWeight(worldPos, probe.m_aabbMin, probe.m_aabbMax, 0.2);
			totalBlendWeight += blendWeight;

			// Sample reflections
			Vec3 cubeUv = intersectProbe(worldPos, reflDir, probe.m_aabbMin, probe.m_aabbMax, probe.m_position);
			cubeUv.z = -cubeUv.z;
			const vector<T, 3> c =
				getBindlessTextureNonUniformIndexCubeVec4(probe.m_cubeTexture).SampleLevel(trilinearClampSampler, cubeUv, reflTexLod).rgb;
			probeColor += c * blendWeight;
		}

		// Normalize the colors
		probeColor /= totalBlendWeight;
	}

	return probeColor;
}

template<typename T>
vector<T, 3> sampleGiProbes(Cluster cluster, StructuredBuffer<GpuSceneGlobalIlluminationProbe> probes, Vec3 normal, Vec3 worldPos,
							SamplerState trilinearClampSampler)
{
	vector<T, 3> probeColor;

	const U32 probeCount = countbits(cluster.m_giProbesMask);

	if(probeCount == 0)
	{
		probeColor = 0.0;
	}
	else if(WaveActiveAllTrue(probeCount == 1))
	{
		// All subgroups point to the same probe and there is only one probe, do a fast path without blend weight

		const GpuSceneGlobalIlluminationProbe probe = probes[firstbitlow2(cluster.m_giProbesMask)];

		// Sample
		probeColor = sampleGlobalIllumination<T>(worldPos, normal, probe, getBindlessTexture3DVec4(probe.m_volumeTexture), trilinearClampSampler);
	}
	else
	{
		// More than one probes, do a slow path that blends them together

		probeColor = 0.0;
		T totalBlendWeight = 0.001;

		// Loop probes
		[loop] while(cluster.m_giProbesMask != 0u)
		{
			const U32 idx = U32(firstbitlow2(cluster.m_giProbesMask));
			cluster.m_giProbesMask &= ~(1u << idx);
			const GpuSceneGlobalIlluminationProbe probe = probes[idx];

			// Compute blend weight
			const F32 blendWeight = computeProbeBlendWeight(worldPos, probe.m_aabbMin, probe.m_aabbMax, probe.m_fadeDistance);
			totalBlendWeight += blendWeight;

			// Sample
			const vector<T, 3> c = sampleGlobalIllumination<T>(worldPos, normal, probe,
															   getBindlessTextureNonUniformIndex3DVec4(probe.m_volumeTexture), trilinearClampSampler);
			probeColor += c * blendWeight;
		}

		// Normalize
		probeColor /= totalBlendWeight;
	}

	return probeColor;
}

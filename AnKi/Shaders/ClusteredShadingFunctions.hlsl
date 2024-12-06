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

/// Returns the index of the zSplit or linearizeDepth(n, f, depth)*zSplitCount
/// Simplifying this equation is 1/(a+b/depth) where a=(n-f)/(n*zSplitCount) and b=f/(n*zSplitCount)
U32 computeZSplitClusterIndex(F32 depth, U32 zSplitCount, F32 a, F32 b)
{
	const F32 fSplitIdx = 1.0 / (a + b / depth);
	return min(zSplitCount - 1u, (U32)fSplitIdx);
}

/// Return the tile index.
U32 computeTileClusterIndexFragCoord(Vec2 fragCoord, U32 tileCountX)
{
	const UVec2 tileXY = UVec2(fragCoord / F32(kClusteredShadingTileSize));
	return tileXY.y * tileCountX + tileXY.x;
}

/// Merge the tiles with z splits into a single cluster.
Cluster mergeClusters(Cluster tileCluster, Cluster zCluster)
{
//#define ANKI_OR_MASKS(x) WaveActiveBitOr(x)
#define ANKI_OR_MASKS(x) (x)

	Cluster outCluster;

	[unroll] for(U32 i = 0; i < kMaxVisibleLights / 32; ++i)
	{
		outCluster.m_pointLightsMask[i] = ANKI_OR_MASKS(tileCluster.m_pointLightsMask[i] & zCluster.m_pointLightsMask[i]);
		outCluster.m_spotLightsMask[i] = ANKI_OR_MASKS(tileCluster.m_spotLightsMask[i] & zCluster.m_spotLightsMask[i]);
	}

	[unroll] for(U32 i = 0; i < kMaxVisibleDecals / 32; ++i)
	{
		outCluster.m_decalsMask[i] = ANKI_OR_MASKS(tileCluster.m_decalsMask[i] & zCluster.m_decalsMask[i]);
	}

	outCluster.m_fogDensityVolumesMask = ANKI_OR_MASKS(tileCluster.m_fogDensityVolumesMask & zCluster.m_fogDensityVolumesMask);
	outCluster.m_reflectionProbesMask = ANKI_OR_MASKS(tileCluster.m_reflectionProbesMask & zCluster.m_reflectionProbesMask);
	outCluster.m_giProbesMask = ANKI_OR_MASKS(tileCluster.m_giProbesMask & zCluster.m_giProbesMask);

#undef ANKI_OR_MASKS

	return outCluster;
}

/// Get the final cluster after ORing and ANDing the masks.
Cluster getClusterFragCoord(StructuredBuffer<Cluster> clusters, GlobalRendererConstants consts, Vec3 fragCoord)
{
	const Cluster tileCluster = clusters[computeTileClusterIndexFragCoord(fragCoord.xy, consts.m_tileCounts.x)];
	const Cluster zCluster = clusters[computeZSplitClusterIndex(fragCoord.z, consts.m_zSplitCount, consts.m_zSplitMagic.x, consts.m_zSplitMagic.y)
									  + consts.m_tileCounts.x * consts.m_tileCounts.y];
	return mergeClusters(tileCluster, zCluster);
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
vector<T, 3> sampleReflectionProbes(Cluster cluster, StructuredBuffer<ReflectionProbe> probes, Vec3 reflDir, Vec3 worldPos, T reflTexLod,
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

		const ReflectionProbe probe = probes[firstbitlow2(cluster.m_reflectionProbesMask)];

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
			const ReflectionProbe probe = probes[idx];

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
vector<T, 3> sampleGiProbes(Cluster cluster, StructuredBuffer<GlobalIlluminationProbe> probes, Vec3 normal, Vec3 worldPos,
							SamplerState trilinearClampSampler)
{
	vector<T, 3> probeColor;

	const U32 probeCount = countbits(cluster.m_giProbesMask);

	if(probeCount == 0)
	{
		probeColor = -1.0;
	}
	else if(WaveActiveAllTrue(probeCount == 1))
	{
		// All subgroups point to the same probe and there is only one probe, do a fast path without blend weight

		const GlobalIlluminationProbe probe = probes[firstbitlow2(cluster.m_giProbesMask)];

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
			const GlobalIlluminationProbe probe = probes[idx];

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

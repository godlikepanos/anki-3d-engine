// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.hlsl>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>

/// Flags to fine tune the probe selection in sampleClipmapCommon
enum SampleClipmapFlag : U32
{
	kSampleClipmapFlagAccurateClipmapSelection = 1 << 0,
	kSampleClipmapFlagBiasSamplePointTowardsCamera = 1 << 1,
	kSampleClipmapFlagBiasSamplePointSurfaceNormal = 1 << 2,
	kSampleClipmapFlagInvalidProbeRejection = 1 << 3,
	kSampleClipmapFlagChebyshevOcclusion = 1 << 4,
	kSampleClipmapFlagBackfacingProbeRejection = 1 << 5,
	kSampleClipmapFlagUsePreviousFrame = 1 << 6,

	kSampleClipmapFlagFullQuality = (1 << 5) - 1,
	kSampleClipmapFlagNone = 0
};

F32 computeClipmapFade(IndirectDiffuseClipmapConstants consts, U32 clipmapIdx, Vec3 cameraPos, Vec3 worldPos, SampleClipmapFlag flags)
{
	Vec3 aabbMin;
	Vec3 aabbMax;
	if(flags & kSampleClipmapFlagUsePreviousFrame)
	{
		aabbMin = consts.m_previousFrameAabbMins[clipmapIdx].xyz;
		aabbMax = consts.m_previousFrameAabbMins[clipmapIdx].xyz + consts.m_sizes[clipmapIdx].xyz;
	}
	else
	{
		aabbMin = consts.m_aabbMins[clipmapIdx].xyz;
		aabbMax = consts.m_aabbMins[clipmapIdx].xyz + consts.m_sizes[clipmapIdx].xyz;
	}

	const Vec3 distances = select(worldPos > cameraPos, aabbMax - cameraPos, cameraPos - aabbMin);

	Vec3 a = abs(worldPos - cameraPos) / distances;
	a = min(1.0, a);
	a = pow(a, 32.0);
	a = 1.0 - a;

	F32 fade = a.x * a.y * a.z;

	return fade;
}

Bool insideClipmap(IndirectDiffuseClipmapConstants consts, U32 clipmapIdx, Vec3 worldPos, SampleClipmapFlag flags)
{
	Vec3 aabbMin;
	Vec3 aabbMax;
	if(flags & kSampleClipmapFlagUsePreviousFrame)
	{
		aabbMin = consts.m_previousFrameAabbMins[clipmapIdx].xyz;
		aabbMax = consts.m_previousFrameAabbMins[clipmapIdx].xyz + consts.m_sizes[clipmapIdx].xyz;
	}
	else
	{
		aabbMin = consts.m_aabbMins[clipmapIdx].xyz;
		aabbMax = consts.m_aabbMins[clipmapIdx].xyz + consts.m_sizes[clipmapIdx].xyz;
	}

	return (all(worldPos < aabbMax) && all(worldPos > aabbMin));
}

U16 findClipmapOnPosition(IndirectDiffuseClipmapConstants consts, Vec3 cameraPos, Vec3 worldPos, F32 randFactor, SampleClipmapFlag flags)
{
	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		const F32 fade = computeClipmapFade(consts, i, cameraPos, worldPos, flags);
		if(fade > randFactor)
		{
			return i;
		}
	}

	return kIndirectDiffuseClipmapCount;
}

U16 findClipmapOnPositionCheap(IndirectDiffuseClipmapConstants consts, Vec3 worldPos, SampleClipmapFlag flags)
{
	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		if(insideClipmap(consts, i, worldPos, flags))
		{
			return i;
		}
	}

	return kIndirectDiffuseClipmapCount;
}

struct SampleClipmapsArgs
{
	SampleClipmapFlag m_flags;
	U32 m_primaryVolume; // 0: Irradiance, 1: radiance, 2: avgIrradiance

	F32 m_clipmapSelectionRandFactor;
	Vec3 m_samplePoint;
	Vec3 m_normal;
	Vec3 m_cameraPos;
};

Vec3 computeBiasedSamplePoint(Vec3 samplePoint, Vec3 normal, Vec3 cameraPos, SampleClipmapFlag flags, F32 bias)
{
	constexpr SampleClipmapFlag bothBiases = kSampleClipmapFlagBiasSamplePointTowardsCamera | kSampleClipmapFlagBiasSamplePointSurfaceNormal;

	if(flags & bothBiases)
	{
		Vec3 biasDir = 0.0;
		if(flags & kSampleClipmapFlagBiasSamplePointTowardsCamera)
		{
			biasDir = normalize(cameraPos - samplePoint);
		}

		if(flags & kSampleClipmapFlagBiasSamplePointSurfaceNormal)
		{
			biasDir += normal;
		}

		if((flags & bothBiases) == bothBiases)
		{
			// Normalize if both biases have been applied
			biasDir = normalize(biasDir);
		}

		return samplePoint + biasDir * bias;
	}
	else
	{
		return samplePoint;
	}
}

/// Find out the clipmap for a given sample point and sample the probes in the volumes.
Vec3 sampleClipmapCommon(SampleClipmapsArgs args, SamplerState linearAnyRepeatSampler, IndirectDiffuseClipmapConstants consts)
{
	const SampleClipmapFlag flags = args.m_flags;

#if 1
	const U16 clipmapIdx = (flags & kSampleClipmapFlagAccurateClipmapSelection)
							   ? findClipmapOnPosition(consts, args.m_cameraPos, args.m_samplePoint, args.m_clipmapSelectionRandFactor, flags)
							   : findClipmapOnPositionCheap(consts, args.m_samplePoint, flags);
#else
	U16 clipmapIdx = 0;
	if(!insideClipmap(consts, clipmapIdx, args.m_samplePoint))
	{
		clipmapIdx = 10;
	}
#endif

#if 0
	if(clipmapIdx == 0)
	{
		return Vec3(1, 0, 0);
	}
	else if(clipmapIdx == 1)
	{
		return Vec3(0, 1, 0);
	}
	else if(clipmapIdx == 2)
	{
		return Vec3(0, 0, 1);
	}
	else
	{
		return Vec3(1, 0, 1);
	}
#endif

	if(clipmapIdx >= kIndirectDiffuseClipmapCount)
	{
		return 0.0;
	}

	const Vec3 clipmapSize = consts.m_sizes[clipmapIdx].xyz;
	const Vec3 probeCountsf = consts.m_probeCounts;
	const IndirectDiffuseClipmapTextures textures = consts.m_textures[clipmapIdx];

	const Vec3 probeSize = clipmapSize / probeCountsf;
	const Vec3 fakeVolumeSize = probeCountsf; // Volume size without the octahedron

	const Vec3 biasedWorldPos = computeBiasedSamplePoint(args.m_samplePoint, args.m_normal, args.m_cameraPos, flags, min3(probeSize) * 0.2);

	const Bool primaryVolumesHaveOctMap = args.m_primaryVolume != 2;
	const F32 octahedronSize =
		(primaryVolumesHaveOctMap) ? ((args.m_primaryVolume == 0) ? textures.m_irradianceOctMapSize : textures.m_radianceOctMapSize) : 1.0;
	const Vec3 realVolumeSize = probeCountsf.xzy * Vec3(octahedronSize + 2.0, octahedronSize + 2.0, 1.0);

	const F32 distMomentsOctSize = textures.m_distanceMomentsOctMapSize;
	const Vec3 distMomentsRealVolumeSize = probeCountsf.xzy * Vec3(distMomentsOctSize + 2.0, distMomentsOctSize + 2.0, 1.0);

	const Vec3 samplePointUvw = frac(biasedWorldPos / clipmapSize);
	const Vec3 icoord = floor(samplePointUvw * fakeVolumeSize - 0.5);
	const Vec3 fcoord = frac(samplePointUvw * fakeVolumeSize - 0.5);

	const Vec3 firstProbePosition = floor((biasedWorldPos - probeSize / 2.0) / probeSize) * probeSize + probeSize / 2.0;

	F32 weightSum = 0.0;
	Vec3 value = 0.0;
	for(U32 i = 0u; i < 8u; ++i)
	{
		const Vec3 xyz = Vec3(UVec3(i, i >> 1u, i >> 2u) & 1u);
		Vec3 coords = icoord + xyz;

		// Repeat
		coords = select(coords >= 0.0, coords, fakeVolumeSize + coords);
		coords = select(coords < fakeVolumeSize, coords, coords - fakeVolumeSize);

		if(flags & kSampleClipmapFlagInvalidProbeRejection)
		{
			const Bool valid = TEX(getBindlessTextureNonUniformIndex3DVec4(textures.m_probeValidityTexture), coords.xzy).x > 0.8;
			if(!valid)
			{
				continue;
			}
		}

		// Reject probes outside the current clipmap
		const Vec3 probePosition = firstProbePosition + xyz * probeSize;
		if(!insideClipmap(consts, clipmapIdx, probePosition, flags))
		{
			continue;
		}

		// Filtering weight
		const Vec3 w3 = select(xyz == 0.0, 1.0 - fcoord, fcoord);
		F32 w = w3.x * w3.y * w3.z;

		// Normal weight
		if(flags & kSampleClipmapFlagBackfacingProbeRejection)
		{
			const Vec3 dir = normalize(probePosition - args.m_samplePoint);
			const F32 wNormal = (dot(dir, args.m_normal) + 1.0) * 0.5;
			w *= (wNormal * wNormal) + 0.2;
		}

		// Chebyshev occlusion
		if(flags & kSampleClipmapFlagChebyshevOcclusion)
		{
			Vec3 uvw = coords.xzy;
			uvw.xy *= distMomentsOctSize + 2.0;
			uvw.xy += 1.0;
			uvw.xy += octahedronEncode(normalize(biasedWorldPos - probePosition)) * distMomentsOctSize;
			uvw.z += 0.5;
			uvw /= distMomentsRealVolumeSize;
			const Vec2 distMoments =
				getBindlessTextureNonUniformIndex3DVec4(textures.m_distanceMomentsTexture).SampleLevel(linearAnyRepeatSampler, uvw, 0.0);

			const F32 variance = abs(distMoments.x * distMoments.x - distMoments.y);

			const F32 posToProbeDist = length(biasedWorldPos - probePosition);
			F32 chebyshevWeight = 1.0;
			if(posToProbeDist > distMoments.x) // occluded
			{
				// v must be greater than 0, which is guaranteed by the if condition above.
				const F32 v = posToProbeDist - distMoments.x;
				chebyshevWeight = variance / (variance + (v * v));

				// Increase the contrast in the weight
				chebyshevWeight = chebyshevWeight * chebyshevWeight * chebyshevWeight;
				chebyshevWeight = max(chebyshevWeight, 0.05);
			}

			w *= chebyshevWeight;
		}

		// Compute the actual coords and sample
		U32 bindlessIndex;
		switch(args.m_primaryVolume)
		{
		case 0:
			bindlessIndex = textures.m_irradianceTexture;
			break;
		case 1:
			bindlessIndex = textures.m_radianceTexture;
			break;
		default:
			bindlessIndex = textures.m_averageIrradianceTexture;
		}

		Vec3 v;
		if(primaryVolumesHaveOctMap)
		{
			Vec3 uvw = coords.xzy;
			uvw.xy *= octahedronSize + 2.0;
			uvw.xy += 1.0;
			uvw.xy += octahedronEncode(args.m_normal) * octahedronSize;
			uvw.z += 0.5;
			uvw /= realVolumeSize;

			v = getBindlessTextureNonUniformIndex3DVec4(bindlessIndex).SampleLevel(linearAnyRepeatSampler, uvw, 0.0);
		}
		else
		{
			v = TEX(getBindlessTextureNonUniformIndex3DVec4(bindlessIndex), coords.xzy);
		}

		value += v * w;
		weightSum += w;
	}

	if(weightSum > kEpsilonF32)
	{
		value /= weightSum;
	}
	else
	{
		value = 0.0;
	}

	return value;
}

Vec3 sampleClipmapIrradiance(Vec3 samplePoint, Vec3 normal, Vec3 cameraPos, IndirectDiffuseClipmapConstants consts,
							 SamplerState linearAnyRepeatSampler, SampleClipmapFlag flags = kSampleClipmapFlagFullQuality, F32 randFactor = 0.5)
{
	SampleClipmapsArgs args;
	args.m_primaryVolume = 0;
	args.m_flags = flags;
	args.m_cameraPos = cameraPos;
	args.m_clipmapSelectionRandFactor = randFactor;
	args.m_normal = normal;
	args.m_samplePoint = samplePoint;

	return sampleClipmapCommon(args, linearAnyRepeatSampler, consts);
}

Vec3 sampleClipmapAvgIrradianceCheap(Vec3 samplePoint, Vec3 cameraPos, IndirectDiffuseClipmapConstants consts, F32 randFactor,
									 SamplerState linearAnyRepeatSampler, SampleClipmapFlag flags)
{
	const U16 clipmapIdx = (flags & kSampleClipmapFlagAccurateClipmapSelection)
							   ? findClipmapOnPosition(consts, cameraPos, samplePoint, randFactor, flags)
							   : findClipmapOnPositionCheap(consts, samplePoint, flags);

	if(clipmapIdx >= kIndirectDiffuseClipmapCount)
	{
		return 0.0;
	}

	const Vec3 clipmapSize = consts.m_sizes[clipmapIdx].xyz;
	const Vec3 probeCountsf = consts.m_probeCounts;

	const Vec3 probeSize = clipmapSize / probeCountsf;
	const Vec3 biasedWorldPos = computeBiasedSamplePoint(samplePoint, 0.0, cameraPos, flags, min3(probeSize) * 0.2);

	const Vec3 samplePointUvw = (biasedWorldPos / clipmapSize).xzy;

	return getBindlessTextureNonUniformIndex3DVec4(consts.m_textures[clipmapIdx].m_averageIrradianceTexture)
		.SampleLevel(linearAnyRepeatSampler, samplePointUvw, 0.0)
		.xyz;
}

Vec3 sampleClipmapAvgIrradiance(Vec3 samplePoint, Vec3 normal, Vec3 cameraPos, IndirectDiffuseClipmapConstants consts,
								SamplerState linearAnyRepeatSampler, SampleClipmapFlag flags = kSampleClipmapFlagFullQuality, F32 randFactor = 0.5)
{
	const SampleClipmapFlag requireManualTrilinearFiltering =
		kSampleClipmapFlagInvalidProbeRejection | kSampleClipmapFlagBackfacingProbeRejection | kSampleClipmapFlagChebyshevOcclusion;
	if(flags & requireManualTrilinearFiltering)
	{
		SampleClipmapsArgs args;
		args.m_primaryVolume = 2;
		args.m_flags = flags;
		args.m_cameraPos = cameraPos;
		args.m_clipmapSelectionRandFactor = randFactor;
		args.m_normal = normal;
		args.m_samplePoint = samplePoint;

		return sampleClipmapCommon(args, linearAnyRepeatSampler, consts);
	}
	else
	{
		return sampleClipmapAvgIrradianceCheap(samplePoint, cameraPos, consts, randFactor, linearAnyRepeatSampler, flags);
	}
}

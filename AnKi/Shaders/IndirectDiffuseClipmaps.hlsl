// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.hlsl>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>

F32 computeClipmapFade(Clipmap clipmap, Vec3 cameraPos, Vec3 lookDir, Vec3 worldPos)
{
	const Vec3 offset = normalize(lookDir) * kIndirectDiffuseClipmapForwardBias * (clipmap.m_index + 1);
	const Vec3 biasedCameraPos = cameraPos + offset;

	const Vec3 probeSize = clipmap.m_size / clipmap.m_probeCounts;
	const Vec3 halfSize = clipmap.m_size * 0.5;
	const Vec3 aabbMin = biasedCameraPos - halfSize + probeSize;
	const Vec3 aabbMax = biasedCameraPos + halfSize - probeSize;

	const Vec3 distances = select(worldPos > cameraPos, aabbMax - cameraPos, cameraPos - aabbMin);

	Vec3 a = abs(worldPos - cameraPos) / distances;
	a = min(1.0, a);
	a = pow(a, 32.0);
	a = 1.0 - a;

	F32 fade = a.x * a.y * a.z;

	return fade;
}

Bool insideClipmap(Clipmap clipmap, Vec3 worldPos)
{
	return (all(worldPos < clipmap.m_aabbMin + clipmap.m_size) && all(worldPos > clipmap.m_aabbMin));
}

U16 findClipmapOnPosition(Clipmap clipmaps[kIndirectDiffuseClipmapCount], Vec3 cameraPos, Vec3 lookDir, Vec3 worldPos, F32 randFactor)
{
	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		const F32 fade = computeClipmapFade(clipmaps[i], cameraPos, lookDir, worldPos);
		if(fade > randFactor)
		{
			return i;
		}
	}

	return kIndirectDiffuseClipmapCount;
}

U16 findClipmapOnPositionCheap(Clipmap clipmaps[kIndirectDiffuseClipmapCount], Vec3 worldPos)
{
	for(U32 i = 0; i < kIndirectDiffuseClipmapCount; ++i)
	{
		if(insideClipmap(clipmaps[i], worldPos))
		{
			return i;
		}
	}

	return kIndirectDiffuseClipmapCount;
}

struct SampleClipmapsConfig
{
	Bool m_fastClipmapSelection;
	Bool m_biasSamplePoint;
	Bool m_doInvalidProbeRejection;
	Bool m_doChebyshevOcclusion;
	Bool m_primaryVolumesHaveOctMap;
	Bool m_normalRejection;
};

struct SampleClipmapsArgs
{
	F32 m_clipmapSelectionRandFactor;
	Vec3 m_samplePoint;
	Vec3 m_normal;
	Vec3 m_cameraPos;
	Vec3 m_lookDir; // Only used when SampleClipmapsConfig::m_fastClipmapSelection is false
};

/// Find out the clipmap for a given sample point and sample the probes in the volumes.
Vec3 sampleClipmapIrradianceCommon(SampleClipmapsConfig cfg, SampleClipmapsArgs args, Texture3D<Vec4> primaryVolumes[kIndirectDiffuseClipmapCount],
								   Texture3D<Vec4> distanceMomentsVolumes[kIndirectDiffuseClipmapCount],
								   Texture3D<Vec4> probeValidityVolumes[kIndirectDiffuseClipmapCount], SamplerState linearAnyRepeatSampler,
								   Clipmap clipmaps[kIndirectDiffuseClipmapCount])
{
#if 1
	const U16 clipmapIdx = (cfg.m_fastClipmapSelection) ? findClipmapOnPositionCheap(clipmaps, args.m_samplePoint)
														: findClipmapOnPosition(clipmaps, args.m_cameraPos, args.m_lookDir, args.m_samplePoint,
																				args.m_clipmapSelectionRandFactor);
#else
	U16 clipmapIdx = 0;
	if(!insideClipmap(clipmaps[clipmapIdx], args.m_samplePoint))
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

	const Clipmap clipmap = clipmaps[clipmapIdx];
	const Vec3 probeSize = clipmap.m_size / clipmap.m_probeCounts;
	const Vec3 fakeVolumeSize = clipmap.m_probeCounts; // Volume size without the octahedron

	const Vec3 biasDir = normalize(args.m_cameraPos - args.m_samplePoint);
	const Vec3 biasedWorldPos = (cfg.m_biasSamplePoint) ? args.m_samplePoint + biasDir * probeSize * 0.2 : args.m_samplePoint;

	F32 octahedronSize = 0.0;
	Vec3 realVolumeSize = 0.0;
	{
		primaryVolumes[0].GetDimensions(realVolumeSize.x, realVolumeSize.y, realVolumeSize.z);

		if(cfg.m_primaryVolumesHaveOctMap)
		{
			octahedronSize = realVolumeSize.x / clipmap.m_probeCounts.x;
			octahedronSize -= 2.0; // The border
		}
	};

	F32 distMomentsOctSize = 0.0;
	Vec3 distMomentsRealVolumeSize = 0.0;
	if(cfg.m_doChebyshevOcclusion)
	{
		distanceMomentsVolumes[0].GetDimensions(distMomentsRealVolumeSize.x, distMomentsRealVolumeSize.y, distMomentsRealVolumeSize.z);

		distMomentsOctSize = distMomentsRealVolumeSize.x / clipmap.m_probeCounts.x;
		distMomentsOctSize -= 2.0; // The border
	}

	const Vec3 samplePointUvw = frac(biasedWorldPos / clipmap.m_size);
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

		if(cfg.m_doInvalidProbeRejection)
		{
			const Bool valid = probeValidityVolumes[NonUniformResourceIndex(clipmapIdx)][coords.xzy].x > 0.8;
			if(!valid)
			{
				continue;
			}
		}

		// Reject probes outside the current clipmap. The accurate version doesn't need that because it fades between clipmaps.
		const Vec3 probePosition = firstProbePosition + xyz * probeSize;
		if(cfg.m_fastClipmapSelection)
		{
			if(!insideClipmap(clipmap, probePosition))
			{
				continue;
			}
		}

		// Filtering weight
		const Vec3 w3 = select(xyz == 0.0, 1.0 - fcoord, fcoord);
		F32 w = w3.x * w3.y * w3.z;

		// Normal weight
		if(cfg.m_normalRejection)
		{
			const Vec3 dir = normalize(probePosition - args.m_samplePoint);
			const F32 wNormal = (dot(dir, args.m_normal) + 1.0) * 0.5;
			w *= (wNormal * wNormal) + 0.2;
		}

		// Chebyshev occlusion
		if(cfg.m_doChebyshevOcclusion)
		{
			Vec3 uvw = coords.xzy;
			uvw.xy *= distMomentsOctSize + 2.0;
			uvw.xy += 1.0;
			uvw.xy += octahedronEncode(normalize(biasedWorldPos - probePosition)) * distMomentsOctSize;
			uvw.z += 0.5;
			uvw /= distMomentsRealVolumeSize;
			const Vec2 distMoments = distanceMomentsVolumes[NonUniformResourceIndex(clipmapIdx)].SampleLevel(linearAnyRepeatSampler, uvw, 0.0);

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

		// Compute the actual coords
		Vec3 v;
		if(cfg.m_primaryVolumesHaveOctMap)
		{
			Vec3 uvw = coords.xzy;
			uvw.xy *= octahedronSize + 2.0;
			uvw.xy += 1.0;
			uvw.xy += octahedronEncode(args.m_normal) * octahedronSize;
			uvw.z += 0.5;
			uvw /= realVolumeSize;

			v = primaryVolumes[NonUniformResourceIndex(clipmapIdx)].SampleLevel(linearAnyRepeatSampler, uvw, 0.0);
		}
		else
		{
			v = primaryVolumes[NonUniformResourceIndex(clipmapIdx)][coords.xzy];
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

Vec3 sampleClipmapIrradianceAccurate(Vec3 samplePoint, Vec3 normal, Vec3 cameraPos, Vec3 lookDir, Clipmap clipmaps[kIndirectDiffuseClipmapCount],
									 Texture3D<Vec4> primaryVolumes[kIndirectDiffuseClipmapCount],
									 Texture3D<Vec4> distanceMomentsVolumes[kIndirectDiffuseClipmapCount],
									 Texture3D<Vec4> probeValidityVolumes[kIndirectDiffuseClipmapCount], SamplerState linearAnyRepeatSampler,
									 F32 randFactor, Bool biasSamplePoint = true)
{
	SampleClipmapsConfig cfg;
	cfg.m_biasSamplePoint = biasSamplePoint;
	cfg.m_doChebyshevOcclusion = true;
	cfg.m_doInvalidProbeRejection = true;
	cfg.m_fastClipmapSelection = false;
	cfg.m_primaryVolumesHaveOctMap = true;
	cfg.m_normalRejection = true;

	SampleClipmapsArgs args;
	args.m_cameraPos = cameraPos;
	args.m_clipmapSelectionRandFactor = randFactor;
	args.m_lookDir = lookDir;
	args.m_normal = normal;
	args.m_samplePoint = samplePoint;

	return sampleClipmapIrradianceCommon(cfg, args, primaryVolumes, distanceMomentsVolumes, probeValidityVolumes, linearAnyRepeatSampler, clipmaps);
}

Vec3 sampleClipmapIrradianceCheap(Vec3 samplePoint, Vec3 normal, Clipmap clipmaps[kIndirectDiffuseClipmapCount],
								  Texture3D<Vec4> primaryVolumes[kIndirectDiffuseClipmapCount], SamplerState linearAnyRepeatSampler,
								  Bool biasSamplePoint = true)
{
	SampleClipmapsConfig cfg;
	cfg.m_biasSamplePoint = biasSamplePoint;
	cfg.m_doChebyshevOcclusion = false;
	cfg.m_doInvalidProbeRejection = false;
	cfg.m_fastClipmapSelection = false;
	cfg.m_primaryVolumesHaveOctMap = true;
	cfg.m_normalRejection = false;

	SampleClipmapsArgs args;
	args.m_cameraPos = 0.0;
	args.m_clipmapSelectionRandFactor = 0.0;
	args.m_lookDir = 0.0;
	args.m_normal = normal;
	args.m_samplePoint = samplePoint;

	return sampleClipmapIrradianceCommon(cfg, args, primaryVolumes, primaryVolumes, primaryVolumes, linearAnyRepeatSampler, clipmaps);
}

Vec3 sampleClipmapAvgIrradianceAccurate(Vec3 samplePoint, Vec3 normal, Vec3 cameraPos, Vec3 lookDir, Clipmap clipmaps[kIndirectDiffuseClipmapCount],
										Texture3D<Vec4> primaryVolumes[kIndirectDiffuseClipmapCount],
										Texture3D<Vec4> distanceMomentsVolumes[kIndirectDiffuseClipmapCount],
										Texture3D<Vec4> probeValidityVolumes[kIndirectDiffuseClipmapCount], SamplerState linearAnyRepeatSampler,
										F32 randFactor, Bool biasSamplePoint = true, Bool normalRejection = true)
{
	SampleClipmapsConfig cfg;
	cfg.m_biasSamplePoint = biasSamplePoint;
	cfg.m_doChebyshevOcclusion = true;
	cfg.m_doInvalidProbeRejection = true;
	cfg.m_fastClipmapSelection = false;
	cfg.m_primaryVolumesHaveOctMap = false;
	cfg.m_normalRejection = normalRejection;

	SampleClipmapsArgs args;
	args.m_cameraPos = cameraPos;
	args.m_clipmapSelectionRandFactor = randFactor;
	args.m_lookDir = lookDir;
	args.m_normal = normal;
	args.m_samplePoint = samplePoint;

	return sampleClipmapIrradianceCommon(cfg, args, primaryVolumes, distanceMomentsVolumes, probeValidityVolumes, linearAnyRepeatSampler, clipmaps);
}

Vec3 sampleClipmapAvgIrradianceCheap(Vec3 samplePoint, Vec3 cameraPos, Vec3 lookDir, Clipmap clipmaps[kIndirectDiffuseClipmapCount],
									 Texture3D<Vec4> primaryVolumes[kIndirectDiffuseClipmapCount], F32 randFactor,
									 SamplerState linearAnyRepeatSampler, Bool biasSamplePoint = true)
{
	const U16 clipmapIdx = findClipmapOnPosition(clipmaps, cameraPos, lookDir, samplePoint, randFactor);

	if(clipmapIdx >= kIndirectDiffuseClipmapCount)
	{
		return 0.0;
	}

	const Clipmap clipmap = clipmaps[clipmapIdx];
	const Vec3 probeSize = clipmap.m_size / clipmap.m_probeCounts;

	const Vec3 biasDir = normalize(cameraPos - samplePoint);
	const Vec3 biasedWorldPos = (biasSamplePoint) ? samplePoint + biasDir * probeSize * 0.2 : samplePoint;

	const Vec3 samplePointUvw = (biasedWorldPos / clipmap.m_size).xzy;

	return primaryVolumes[NonUniformResourceIndex(clipmapIdx)].SampleLevel(linearAnyRepeatSampler, samplePointUvw, 0.0).xyz;
}

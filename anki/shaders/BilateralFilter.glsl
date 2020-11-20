// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shaders/Common.glsl>

struct BilateralSample
{
	F32 m_depth;
	Vec3 m_position;
	Vec3 m_normal;
	F32 m_roughness;
};

struct BilateralConfig
{
	F32 m_depthWeight;
	F32 m_normalWeight;
	F32 m_planeWeight;
	F32 m_roughnessWeight;
};

// https://cs.dartmouth.edu/~wjarosz/publications/mara17towards.html
F32 calculateBilateralWeight(BilateralSample center, BilateralSample tap, BilateralConfig config)
{
	F32 depthWeight = 1.0;
	F32 normalWeight = 1.0;
	F32 planeWeight = 1.0;
	F32 glossyWeight = 1.0;

	if(config.m_depthWeight > 0.0)
	{
#if 0
		depthWeight = max(0.0, 1.0 - abs(tap.m_depth - center.m_depth) * config.m_depthWeight);
#else
		const F32 diff = abs(tap.m_depth - center.m_depth);
		depthWeight = sqrt(1.0 / (EPSILON + diff)) * config.m_depthWeight;
#endif
	}

	if(config.m_normalWeight > 0.0)
	{
		F32 normalCloseness = dot(tap.m_normal, center.m_normal);
		normalCloseness = normalCloseness * normalCloseness;
		normalCloseness = normalCloseness * normalCloseness;

		const F32 normalError = (1.0 - normalCloseness);
		normalWeight = max((1.0 - normalError * config.m_normalWeight), 0.0);
	}

	if(config.m_planeWeight > 0.0)
	{
		const F32 lowDistanceThreshold2 = 0.001;

		// Change in position in camera space
		Vec3 dq = center.m_position - tap.m_position;

		// How far away is this point from the original sample in camera space? (Max value is unbounded)
		const F32 distance2 = dot(dq, dq);

		// How far off the expected plane (on the perpendicular) is this point? Max value is unbounded.
		const F32 planeError = max(abs(dot(dq, tap.m_normal)), abs(dot(dq, center.m_normal)));

		planeWeight = (distance2 < lowDistanceThreshold2)
						  ? 1.0
						  : pow(max(0.0, 1.0 - 2.0 * config.m_planeWeight * planeError / sqrt(distance2)), 2.0);
	}

	if(config.m_roughnessWeight > 0.0)
	{
		const F32 gDiff = abs(tap.m_roughness - center.m_roughness) * 10.0;
		glossyWeight = max(0.0, 1.0 - (gDiff * config.m_roughnessWeight));
	}

	return depthWeight * normalWeight * planeWeight * glossyWeight;
}

struct SpatialBilateralContext
{
	U32 sampleCount;
	UVec2 referencePointUnormalizedTextureUv;
	U32 maxRadiusPixels;
	U32 spiralTurnCount;
	F32 randomRotationAngle;
};

// Initialize the spatial bilateral context. Its purpose it to create samples that form a spiral around the reference
// point. To experiment and play with the values use this: https://www.shadertoy.com/view/3s3BRs
SpatialBilateralContext spatialBilateralInit(U32 sampleCount, UVec2 referencePointUnormalizedTextureUv,
											 U32 maxRadiusPixels, U32 spiralTurnCount, F32 time)
{
	SpatialBilateralContext ctx;

	ctx.sampleCount = sampleCount;
	ctx.referencePointUnormalizedTextureUv = referencePointUnormalizedTextureUv;
	ctx.maxRadiusPixels = maxRadiusPixels;
	ctx.spiralTurnCount = spiralTurnCount;

	const UVec2 ref = referencePointUnormalizedTextureUv;
	ctx.randomRotationAngle = F32((3u * ref.x) ^ (ref.y + ref.x * ref.y)) + time;

	return ctx;
}

// Iterate to get a new sample. See spatialBilateralInit()
UVec2 spatialBilateralIterate(SpatialBilateralContext ctx, U32 iteration)
{
	const F32 alpha = F32(F32(iteration) + 0.5) * (1.0 / F32(ctx.sampleCount));
	const F32 angle = alpha * F32(ctx.spiralTurnCount) * 6.28 + ctx.randomRotationAngle;

	const Vec2 unitOffset = Vec2(cos(angle), sin(angle));

	const IVec2 tapOffset = IVec2(alpha * F32(ctx.maxRadiusPixels) * unitOffset);
	return UVec2(IVec2(ctx.referencePointUnormalizedTextureUv) + tapOffset);
}

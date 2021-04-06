// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains a bounch of edge stopping functions plus some other things

#pragma once

#include <AnKi/Shaders/Common.glsl>

// https://cs.dartmouth.edu/~wjarosz/publications/mara17towards.html
F32 calculateBilateralWeightDepth(F32 depthCenter, F32 depthTap, F32 phi)
{
	const F32 diff = abs(depthTap - depthCenter);
#if 0
	return max(0.0, 1.0 - diff * phi);
#else
	return sqrt(1.0 / (EPSILON + diff)) * phi;
#endif
}

// From the SVGF sample code. Depth is linear
F32 calculateBilateralWeightDepth2(F32 depthCenter, F32 depthTap, F32 phi)
{
	return (phi == 0.0) ? 0.0 : abs(depthCenter - depthTap) / phi;
}

// https://cs.dartmouth.edu/~wjarosz/publications/mara17towards.html
F32 calculateBilateralWeightNormal(Vec3 center, Vec3 tap, F32 phi)
{
	F32 normalCloseness = dot(tap, center);
	normalCloseness = normalCloseness * normalCloseness;
	normalCloseness = normalCloseness * normalCloseness;

	const F32 normalError = (1.0 - normalCloseness);
	return max((1.0 - normalError * phi), 0.0);
}

F32 calculateBilateralWeightNormalCos(Vec3 ref, Vec3 tap, F32 phi)
{
	return pow(saturate(dot(ref, tap)), phi);
}

// https://cs.dartmouth.edu/~wjarosz/publications/mara17towards.html
F32 calculateBilateralWeightPlane(Vec3 positionCenter, Vec3 centerNormal, Vec3 positionTap, Vec3 normalTap, F32 phi)
{
	const F32 lowDistanceThreshold2 = 0.001;

	// Change in position in camera space
	const Vec3 dq = positionCenter - positionTap;

	// How far away is this point from the original sample in camera space? (Max value is unbounded)
	const F32 distance2 = dot(dq, dq);

	// How far off the expected plane (on the perpendicular) is this point? Max value is unbounded.
	const F32 planeError = max(abs(dot(dq, normalTap)), abs(dot(dq, centerNormal)));

	return (distance2 < lowDistanceThreshold2) ? 1.0
											   : pow(max(0.0, 1.0 - 2.0 * phi * planeError / sqrt(distance2)), 2.0);
}

// https://cs.dartmouth.edu/~wjarosz/publications/mara17towards.html
F32 calculateBilateralWeightRoughness(F32 roughnessCenter, F32 roughnessTap, F32 phi)
{
	const F32 gDiff = abs(roughnessCenter - roughnessTap) * 10.0;
	return max(0.0, 1.0 - (gDiff * phi));
}

// From SVGF sample code.
F32 calculateBilateralWeightLinearDepthAndLuminance(F32 depthCenter, F32 luminanceCenter, F32 depthTap,
													F32 luminanceTap, F32 phiDepth, F32 phiLuminance)
{
	const F32 wZ = calculateBilateralWeightDepth2(depthCenter, depthTap, phiDepth);
	const F32 wL = abs(luminanceCenter - luminanceTap) / phiLuminance;
	return exp(0.0 - max(wL, 0.0) - max(wZ, 0.0));
}

// Compute a weight given 2 viewspace positions.
F32 calculateBilateralWeightViewspacePosition(Vec3 posCenter, Vec3 posTap, F32 sigma)
{
	const Vec3 t = posCenter - posTap;
	const F32 dist2 = dot(t, t) + t.z * t.z;
	return min(exp(-(dist2) / sigma), 1.0);
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

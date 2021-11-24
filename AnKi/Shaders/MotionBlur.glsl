// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.glsl>

// Perform motion blur.
Vec3 motionBlur(texture2D motionVectorsRt, sampler motionVectorsRtSampler, ANKI_RP texture2D toBlurRt,
				Vec2 toBlurRtSize, sampler toBlurRtSampler, Vec2 uv, U32 maxSamples)
{
	// Compute velocity. Get the max velocity around the curent sample to avoid outlines. TAA's result and the motion
	// vectors RT do not quite overlap
	Vec2 velocityMin = textureLod(motionVectorsRt, motionVectorsRtSampler, uv, 0.0).rg;
	Vec2 velocityMax = velocityMin;

	Vec2 v = textureLodOffset(sampler2D(motionVectorsRt, motionVectorsRtSampler), uv, 0.0, ivec2(-2, -2)).rg;
	velocityMin = min(velocityMin, v);
	velocityMax = max(velocityMax, v);

	v = textureLodOffset(sampler2D(motionVectorsRt, motionVectorsRtSampler), uv, 0.0, ivec2(2, 2)).rg;
	velocityMin = min(velocityMin, v);
	velocityMax = max(velocityMax, v);

	v = textureLodOffset(sampler2D(motionVectorsRt, motionVectorsRtSampler), uv, 0.0, ivec2(-2, 2)).rg;
	velocityMin = min(velocityMin, v);
	velocityMax = max(velocityMax, v);

	v = textureLodOffset(sampler2D(motionVectorsRt, motionVectorsRtSampler), uv, 0.0, ivec2(2, -2)).rg;
	velocityMin = min(velocityMin, v);
	velocityMax = max(velocityMax, v);

	const F32 mag0 = length(velocityMin);
	const F32 mag1 = length(velocityMax);
	const Vec2 velocity = (mag0 > mag1) ? velocityMin : velocityMax;

	// Compute the sample count
	const Vec2 slopes = abs(velocity);
	const Vec2 sampleCount2D = slopes * toBlurRtSize;
	F32 sampleCountf = max(sampleCount2D.x, sampleCount2D.y);
	sampleCountf = clamp(sampleCountf, 1.0, F32(maxSamples));
	sampleCountf = round(sampleCountf);

	// Sample
	const ANKI_RP F32 weight = 1.0 / sampleCountf;
	ANKI_RP Vec3 outColor = textureLod(toBlurRt, toBlurRtSampler, uv, 0.0).rgb * weight;
	ANKI_LOOP for(F32 s = 1.0; s < sampleCountf; s += 1.0)
	{
		const F32 f = s / sampleCountf;
		const Vec2 sampleUv = uv + velocity * f;

		outColor += textureLod(toBlurRt, toBlurRtSampler, sampleUv, 0.0).rgb * weight;
	}

	return outColor;
}

// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.hlsl>

// Perform motion blur.
RVec3 motionBlur(Texture2D motionVectorsRt, SamplerState motionVectorsRtSampler, Texture2D<RVec4> toBlurRt,
				 Vec2 toBlurRtSize, SamplerState toBlurRtSampler, Vec2 uv, U32 maxSamples)
{
	// Compute velocity. Get the max velocity around the curent sample to avoid outlines. TAA's result and the motion
	// vectors RT do not quite overlap
	Vec2 velocityMin = motionVectorsRt.SampleLevel(motionVectorsRtSampler, uv, 0.0).rg;
	Vec2 velocityMax = velocityMin;

	Vec2 v = motionVectorsRt.SampleLevel(motionVectorsRtSampler, uv, 0.0, IVec2(-2, -2)).rg;
	velocityMin = min(velocityMin, v);
	velocityMax = max(velocityMax, v);

	v = motionVectorsRt.SampleLevel(motionVectorsRtSampler, uv, 0.0, IVec2(2, 2)).rg;
	velocityMin = min(velocityMin, v);
	velocityMax = max(velocityMax, v);

	v = motionVectorsRt.SampleLevel(motionVectorsRtSampler, uv, 0.0, IVec2(-2, 2)).rg;
	velocityMin = min(velocityMin, v);
	velocityMax = max(velocityMax, v);

	v = motionVectorsRt.SampleLevel(motionVectorsRtSampler, uv, 0.0, IVec2(2, -2)).rg;
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
	const RF32 weight = 1.0 / sampleCountf;
	RVec3 outColor = toBlurRt.SampleLevel(toBlurRtSampler, uv, 0.0).rgb * weight;
	[loop] for(F32 s = 1.0; s < sampleCountf; s += 1.0)
	{
		const F32 f = s / sampleCountf;
		const Vec2 sampleUv = uv + velocity * f;

		outColor += toBlurRt.SampleLevel(toBlurRtSampler, sampleUv, 0.0).rgb * weight;
	}

	return outColor;
}

// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <shaders/Common.glsl>

// Perform motion blur.
Vec3 motionBlur(sampler2D velocityTex,
	sampler2D toBlurTex,
	sampler2D depthTex,
	Vec2 nowUv,
	Mat4 prevViewProjMatMulInvViewProjMat,
	U32 maxSamples)
{
	// Compute previous UV
	Vec2 pastUv = textureLod(velocityTex, nowUv, 0.0).rg;

	ANKI_BRANCH if(pastUv.x < 0.0)
	{
		F32 depth = textureLod(depthTex, nowUv, 0.0).r;

		Vec4 v4 = prevViewProjMatMulInvViewProjMat * Vec4(UV_TO_NDC(nowUv), depth, 1.0);
		pastUv = NDC_TO_UV(v4.xy / v4.w);
	}

	// March direction
	Vec2 dir = pastUv - nowUv;

	Vec2 slopes = abs(dir);

	// Compute the sample count
	Vec2 sampleCount2D = slopes * Vec2(FB_SIZE);
	F32 sampleCountf = max(sampleCount2D.x, sampleCount2D.y);
	sampleCountf = clamp(sampleCountf, 1.0, F32(maxSamples));
	sampleCountf = round(sampleCountf);

	// Loop
	Vec3 outColor = Vec3(0.0);
	ANKI_LOOP for(F32 s = 0.0; s < sampleCountf; s += 1.0)
	{
		F32 f = s / sampleCountf;
		Vec2 sampleUv = nowUv + dir * f;

		outColor += textureLod(toBlurTex, sampleUv, 0.0).rgb;
	}

	outColor /= sampleCountf;

	return outColor;
}
// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shaders/Common.glsl>

// A tick to compute log of base 10
F32 log10(F32 x)
{
	return log(x) / log(10.0);
}

F32 computeLuminance(Vec3 color)
{
	return max(dot(Vec3(0.30, 0.59, 0.11), color), EPSILON);
}

F32 computeExposure(F32 avgLum, F32 threshold)
{
	const F32 keyValue = 1.03 - (2.0 / (2.0 + log10(avgLum + 1.0)));
	const F32 linearExposure = (keyValue / avgLum);
	F32 exposure = log2(linearExposure);

	exposure -= threshold;
	return exp2(exposure);
}

Vec3 computeExposedColor(Vec3 color, F32 avgLum, F32 threshold)
{
	return computeExposure(avgLum, threshold) * color;
}

// Reinhard operator
Vec3 tonemapReinhard(Vec3 color, F32 saturation)
{
	const F32 lum = computeLuminance(color);
	const F32 toneMappedLuminance = lum / (lum + 1.0);
	return toneMappedLuminance * pow(color / lum, Vec3(saturation));
}

// Uncharted 2 operator
Vec3 tonemapUncharted2(Vec3 color)
{
	const F32 A = 0.15;
	const F32 B = 0.50;
	const F32 C = 0.10;
	const F32 D = 0.20;
	const F32 E = 0.02;
	const F32 F = 0.30;

	return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
}

Vec3 tonemapACESFilm(Vec3 x)
{
	const F32 a = 2.51;
	const F32 b = 0.03;
	const F32 c = 2.43;
	const F32 d = 0.59;
	const F32 e = 0.14;

	return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

Vec3 tonemap(Vec3 color, F32 exposure)
{
	color *= exposure;
#if 0
	const F32 saturation = 1.0;
	return tonemapReinhard(color, saturation);
#else
	return tonemapACESFilm(color);
#endif
}

Vec3 tonemap(Vec3 color, F32 avgLum, F32 threshold)
{
	const F32 exposure = computeExposure(avgLum, threshold);
	return tonemap(color, exposure);
}

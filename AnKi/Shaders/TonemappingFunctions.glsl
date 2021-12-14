// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.glsl>

// A tick to compute log of base 10
ANKI_RP F32 log10(ANKI_RP F32 x)
{
	return log(x) / log(10.0);
}

ANKI_RP F32 computeLuminance(ANKI_RP Vec3 color)
{
	return max(dot(Vec3(0.30, 0.59, 0.11), color), EPSILON_RP);
}

ANKI_RP F32 computeExposure(ANKI_RP F32 avgLum, ANKI_RP F32 threshold)
{
	const ANKI_RP F32 keyValue = 1.03 - (2.0 / (2.0 + log10(avgLum + 1.0)));
	const ANKI_RP F32 linearExposure = (keyValue / avgLum);
	ANKI_RP F32 exposure = log2(linearExposure);

	exposure -= threshold;
	return exp2(exposure);
}

ANKI_RP Vec3 computeExposedColor(ANKI_RP Vec3 color, ANKI_RP F32 avgLum, ANKI_RP F32 threshold)
{
	return computeExposure(avgLum, threshold) * color;
}

// Reinhard operator
ANKI_RP Vec3 tonemapReinhard(ANKI_RP Vec3 color, ANKI_RP F32 saturation)
{
	const ANKI_RP F32 lum = computeLuminance(color);
	const ANKI_RP F32 toneMappedLuminance = lum / (lum + 1.0);
	return toneMappedLuminance * pow(color / lum, Vec3(saturation));
}

// Uncharted 2 operator
ANKI_RP Vec3 tonemapUncharted2(ANKI_RP Vec3 color)
{
	const F32 A = 0.15;
	const F32 B = 0.50;
	const F32 C = 0.10;
	const F32 D = 0.20;
	const F32 E = 0.02;
	const F32 F = 0.30;

	return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
}

ANKI_RP Vec3 tonemapACESFilm(ANKI_RP Vec3 x)
{
	const ANKI_RP F32 a = 2.51;
	const ANKI_RP F32 b = 0.03;
	const ANKI_RP F32 c = 2.43;
	const ANKI_RP F32 d = 0.59;
	const ANKI_RP F32 e = 0.14;

	return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

ANKI_RP Vec3 tonemap(ANKI_RP Vec3 color, ANKI_RP F32 exposure)
{
	color *= exposure;
#if 0
	const ANKI_RP F32 saturation = 1.0;
	return tonemapReinhard(color, saturation);
#else
	return tonemapACESFilm(color);
#endif
}

ANKI_RP Vec3 tonemap(ANKI_RP Vec3 color, ANKI_RP F32 avgLum, ANKI_RP F32 threshold)
{
	const ANKI_RP F32 exposure = computeExposure(avgLum, threshold);
	return tonemap(color, exposure);
}

// https://graphicrants.blogspot.com/2013/12/tone-mapping.html
ANKI_RP Vec3 invertibleTonemap(ANKI_RP Vec3 colour)
{
	// 1 / (1 + max(rgb))
	return colour / (1.0 + max(max(colour.r, colour.g), colour.b));
}

ANKI_RP Vec3 invertInvertibleTonemap(ANKI_RP Vec3 colour)
{
	// 1 / (1 - max(rgb))
	return colour / (1.0 - max(max(colour.r, colour.g), colour.b));
}

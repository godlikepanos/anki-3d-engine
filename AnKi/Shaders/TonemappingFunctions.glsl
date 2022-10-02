// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
	return max(dot(Vec3(0.30, 0.59, 0.11), color), kEpsilonRp);
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

const ANKI_RP F32 kACESA = 2.51;
const ANKI_RP F32 kACESB = 0.03;
const ANKI_RP F32 kACESC = 2.43;
const ANKI_RP F32 kACESD = 0.59;
const ANKI_RP F32 kACESE = 0.14;

// See ACES in action and its inverse at https://www.desmos.com/calculator/n1lkpc6hwq
ANKI_RP Vec3 tonemapACESFilm(ANKI_RP Vec3 x)
{
	return saturate((x * (kACESA * x + kACESB)) / (x * (kACESC * x + kACESD) + kACESE));
}

// https://www.desmos.com/calculator/n1lkpc6hwq
ANKI_RP Vec3 invertTonemapACESFilm(ANKI_RP Vec3 x)
{
	ANKI_RP Vec3 res = kACESD * x - kACESB;
	res += sqrt(x * x * (kACESD * kACESD - 4.0 * kACESE * kACESC) + x * (4.0 * kACESE * kACESA - 2.0 * kACESB * kACESD)
				+ kACESB * kACESB);
	res /= 2.0 * kACESA - 2.0 * kACESC * x;

	return res;
}

ANKI_RP Vec3 tonemap(ANKI_RP Vec3 color, ANKI_RP F32 exposure)
{
	color *= exposure;
	return tonemapACESFilm(color);
}

ANKI_RP Vec3 invertTonemap(ANKI_RP Vec3 color, ANKI_RP F32 exposure)
{
	color = invertTonemapACESFilm(color);
	color /= max(kEpsilonf, exposure);
	return color;
}

ANKI_RP Vec3 tonemap(ANKI_RP Vec3 color, ANKI_RP F32 avgLum, ANKI_RP F32 threshold)
{
	const ANKI_RP F32 exposure = computeExposure(avgLum, threshold);
	return tonemap(color, exposure);
}

// https://graphicrants.blogspot.com/2013/12/tone-mapping.html
Vec3 reinhardTonemap(Vec3 colour)
{
	// rgb / (1 + max(rgb))
	return colour / (1.0 + max(max(colour.r, colour.g), colour.b));
}

F32 reinhardTonemap(F32 value)
{
	return value / (1.0 + value);
}

F16 reinhardTonemap(F16 value)
{
	return value / (1.0hf + value);
}

Vec3 invertReinhardTonemap(Vec3 colour)
{
	// rgb / (1 - max(rgb))
	return colour / max(1.0 / 32768.0, 1.0 - max(max(colour.r, colour.g), colour.b));
}

HVec3 invertReinhardTonemap(HVec3 colour)
{
	// rgb / (1 - max(rgb))
	return colour / max(F16(1.0 / 32768.0), 1.0hf - max(max(colour.r, colour.g), colour.b));
}

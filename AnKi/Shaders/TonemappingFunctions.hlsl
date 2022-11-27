// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.hlsl>

// A tick to compute log of base 10
template<typename T>
T log10(T x)
{
	return log(x) / log((T)10.0);
}

RVec3 computeLuminance(RVec3 color)
{
	return max(dot(RVec3(0.30, 0.59, 0.11), color), kEpsilonRf);
}

RF32 computeExposure(RF32 avgLum, RF32 threshold)
{
	const RF32 keyValue = 1.03 - (2.0 / (2.0 + log10(avgLum + 1.0)));
	const RF32 linearExposure = (keyValue / avgLum);
	RF32 exposure = log2(linearExposure);

	exposure -= threshold;
	return exp2(exposure);
}

RVec3 computeExposedColor(RVec3 color, RF32 avgLum, RF32 threshold)
{
	return computeExposure(avgLum, threshold) * color;
}

// Uncharted 2 operator
RF32 tonemapUncharted2(RF32 color)
{
	const RF32 A = 0.15;
	const RF32 B = 0.50;
	const RF32 C = 0.10;
	const RF32 D = 0.20;
	const RF32 E = 0.02;
	const RF32 F = 0.30;

	return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
}

constexpr RF32 kAcesA = 2.51;
constexpr RF32 kAcesB = 0.03;
constexpr RF32 kAcesC = 2.43;
constexpr RF32 kAcesD = 0.59;
constexpr RF32 kAcesE = 0.14;

// See ACES in action and its inverse at https://www.desmos.com/calculator/n1lkpc6hwq
RVec3 tonemapACESFilm(RVec3 x)
{
	return saturate((x * (kAcesA * x + kAcesB)) / (x * (kAcesC * x + kAcesD) + kAcesE));
}

// https://www.desmos.com/calculator/n1lkpc6hwq
RVec3 invertTonemapACESFilm(RVec3 x)
{
	RVec3 res = kAcesD * x - kAcesB;
	res += sqrt(x * x * (kAcesD * kAcesD - 4.0 * kAcesE * kAcesC) + x * (4.0 * kAcesE * kAcesA - 2.0 * kAcesB * kAcesD)
				+ kAcesB * kAcesB);
	res /= 2.0 * kAcesA - 2.0 * kAcesC * x;

	return res;
}

RVec3 tonemap(RVec3 color, RF32 exposure)
{
	color *= exposure;
	return tonemapACESFilm(color);
}

RVec3 invertTonemap(RVec3 color, RF32 exposure)
{
	color = invertTonemapACESFilm(color);
	color /= max(kEpsilonRf, exposure);
	return color;
}

RVec3 tonemap(RVec3 color, RF32 avgLum, RF32 threshold)
{
	const RF32 exposure = computeExposure(avgLum, threshold);
	return tonemap(color, exposure);
}

// https://graphicrants.blogspot.com/2013/12/tone-mapping.html
RVec3 reinhardTonemap(RVec3 colour)
{
	// rgb / (1 + max(rgb))
	return colour / (1.0 + max(max(colour.r, colour.g), colour.b));
}

RVec3 invertReinhardTonemap(RVec3 colour)
{
	// rgb / (1 - max(rgb))
	return colour / max(1.0 / 32768.0, 1.0 - max(max(colour.r, colour.g), colour.b));
}

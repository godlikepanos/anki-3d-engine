// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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

template<typename T>
vector<T, 3> computeLuminance(vector<T, 3> color)
{
	return max(dot(vector<T, 3>(0.30, 0.59, 0.11), color), T(kEpsilonRF32));
}

template<typename T>
T computeExposure(T avgLum, T threshold)
{
	const T keyValue = T(1.03) - (T(2.0) / (T(2.0) + log10(avgLum + T(1.0))));
	const T linearExposure = (keyValue / avgLum);
	T exposure = log2(linearExposure);

	exposure -= threshold;
	return exp2(exposure);
}

template<typename T>
vector<T, 3> computeExposedColor(vector<T, 3> color, vector<T, 3> avgLum, vector<T, 3> threshold)
{
	return computeExposure(avgLum, threshold) * color;
}

// Uncharted 2 operator
template<typename T>
vector<T, 3> tonemapUncharted2(vector<T, 3> color)
{
	const T A = 0.15;
	const T B = 0.50;
	const T C = 0.10;
	const T D = 0.20;
	const T E = 0.02;
	const T F = 0.30;

	return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
}

// See ACES in action and its inverse at https://www.desmos.com/calculator/n1lkpc6hwq
template<typename T>
vector<T, 3> tonemapACESFilm(vector<T, 3> x)
{
	constexpr T kAcesA = 2.51;
	constexpr T kAcesB = 0.03;
	constexpr T kAcesC = 2.43;
	constexpr T kAcesD = 0.59;
	constexpr T kAcesE = 0.14;

	return saturate((x * (kAcesA * x + kAcesB)) / (x * (kAcesC * x + kAcesD) + kAcesE));
}

// https://www.desmos.com/calculator/n1lkpc6hwq
template<typename T>
vector<T, 3> invertTonemapACESFilm(vector<T, 3> x)
{
	constexpr T kAcesA = 2.51;
	constexpr T kAcesB = 0.03;
	constexpr T kAcesC = 2.43;
	constexpr T kAcesD = 0.59;
	constexpr T kAcesE = 0.14;

	vector<T, 3> res = kAcesD * x - kAcesB;
	res += sqrt(x * x * (kAcesD * kAcesD - T(4.0) * kAcesE * kAcesC) + x * (T(4.0) * kAcesE * kAcesA - T(2.0) * kAcesB * kAcesD) + kAcesB * kAcesB);
	res /= T(2.0) * kAcesA - T(2.0) * kAcesC * x;

	return res;
}

template<typename T>
vector<T, 3> tonemap(vector<T, 3> color, vector<T, 3> exposure)
{
	color *= exposure;
	return tonemapACESFilm(color);
}

template<typename T>
vector<T, 3> invertTonemap(vector<T, 3> color, T exposure)
{
	color = invertTonemapACESFilm(color);
	color /= max(T(kEpsilonRF32), exposure);
	return color;
}

template<typename T>
vector<T, 3> tonemap(vector<T, 3> color, T avgLum, T threshold)
{
	const T exposure = computeExposure(avgLum, threshold);
	return tonemap<T>(color, exposure);
}

// https://graphicrants.blogspot.com/2013/12/tone-mapping.html
template<typename T>
vector<T, 3> reinhardTonemap(vector<T, 3> colour)
{
	// rgb / (1 + max(rgb))
	return colour / (T(1.0) + max(max(colour.r, colour.g), colour.b));
}

template<typename T>
vector<T, 3> invertReinhardTonemap(vector<T, 3> colour)
{
	// rgb / (1 - max(rgb))
	return colour / max(T(1.0 / 32768.0), T(1.0) - max(max(colour.r, colour.g), colour.b));
}

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
	return log(x) / log(T(10));
}

template<typename T>
T computeExposure(T avgLum, T threshold)
{
	const T keyValue = T(1.03) - (T(2) / (T(2) + log10(avgLum + T(1))));
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
	constexpr T A = 0.15;
	constexpr T B = 0.50;
	constexpr T C = 0.10;
	constexpr T D = 0.20;
	constexpr T E = 0.02;
	constexpr T F = 0.30;

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
	res += sqrt(x * x * (kAcesD * kAcesD - T(4) * kAcesE * kAcesC) + x * (T(4) * kAcesE * kAcesA - T(2) * kAcesB * kAcesD) + kAcesB * kAcesB);
	res /= T(2) * kAcesA - T(2) * kAcesC * x;

	return res;
}

// https://github.com/KhronosGroup/ToneMapping
template<typename T>
vector<T, 3> tonemapNatural(vector<T, 3> color)
{
	const T startCompression = T(0.8 - 0.04);
	const T desaturation = T(0.15);

	const T x = min3(color);
	const T offset = x < T(0.08) ? x - T(6.25) * x * x : T(0.04);
	color -= offset;

	const T peak = max3(color);
	if(peak < startCompression)
	{
		return color;
	}

	const T d = T(1) - startCompression;
	const T newPeak = T(1) - d * d / (peak + d - startCompression);
	color *= newPeak / peak;

	const T g = T(1) / (desaturation * (peak - newPeak) + T(1));
	return lerp(newPeak, color, g);
}

template<typename T>
vector<T, 3> invertTonemapNeutral(vector<T, 3> color)
{
	const T startCompression = T(0.8 - 0.04);
	const T desaturation = T(0.15);

	const T peak = max3(color);
	if(peak > startCompression)
	{
		const T d = T(1) - startCompression;
		const T oldPeak = (d * d) / (T(1) - peak) - d + startCompression;
		const T fInv = desaturation * (oldPeak - peak) + T(1);
		const T f = T(1) / fInv;

		color = (color + (f - T(1)) * peak) * fInv;

		const T scale = oldPeak / peak;
		color *= scale;
	}

	const T y = min3(color);
	T offset = T(0.04);
	if(y < T(0.04))
	{
		const T x = sqrt(y / T(6.25));
		offset = x - T(6.25) * x * x;
	}

	color += offset;
	return color;
}

template<typename T>
vector<T, 3> tonemap(vector<T, 3> color, vector<T, 3> exposure)
{
	color *= exposure;
	return tonemapNatural(color);
}

template<typename T>
vector<T, 3> invertTonemap(vector<T, 3> color, T exposure)
{
	color = invertTonemapNeutral(color);
	color /= max(getEpsilon<T>(), exposure);
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
	return colour / (T(1) + max(max(colour.r, colour.g), colour.b));
}

template<typename T>
vector<T, 3> invertReinhardTonemap(vector<T, 3> colour)
{
	// rgb / (1 - max(rgb))
	return colour / max(T(1.0 / 32768.0), T(1) - max(max(colour.r, colour.g), colour.b));
}

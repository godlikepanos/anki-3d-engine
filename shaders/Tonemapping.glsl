// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_TONEMAP_GLSL
#define ANKI_SHADERS_TONEMAP_GLSL

#include "shaders/Common.glsl"

// A tick to compute log of base 10
float log10(in float x)
{
	return log(x) / log(10.0);
}

float computeLuminance(in vec3 color)
{
	return max(dot(vec3(0.30, 0.59, 0.11), color), EPSILON);
}

vec3 computeExposedColor(in vec3 color, in float avgLum, in float threshold)
{
	float keyValue = 1.03 - (2.0 / (2.0 + log10(avgLum + 1.0)));
	float linearExposure = (keyValue / avgLum);
	float exposure = log2(linearExposure);

	exposure -= threshold;
	return exp2(exposure) * color;
}

// Reinhard operator
vec3 tonemapReinhard(in vec3 color, in float saturation)
{
	float lum = computeLuminance(color);
	float toneMappedLuminance = lum / (lum + 1.0);
	return toneMappedLuminance * pow(color / lum, vec3(saturation));
}

// Uncharted 2 operator
vec3 tonemapUncharted2(in vec3 color)
{
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;

	return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
}

vec3 tonemap(in vec3 color, in float avgLum, in float threshold)
{
	vec3 c = computeExposedColor(color, avgLum, threshold);
	// float saturation = clamp(avgLum, 0.0, 1.0);
	float saturation = 1.0;
	return tonemapReinhard(c, saturation);
	// return tonemapUncharted2(c);
}

#endif

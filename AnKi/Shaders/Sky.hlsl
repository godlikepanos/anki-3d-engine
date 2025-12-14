// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.hlsl>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>

// These are per megameter
constexpr F32 kGroundRadiusMM = 6.360f;
constexpr F32 kAtmosphereRadiusMM = 6.460f;

constexpr Vec3 kViewPos = Vec3(0.0f, kGroundRadiusMM + 0.0002f, 0.0f); // 200M above the ground.

F32 safeacos(F32 x)
{
	return acos(clamp(x, -1.0f, 1.0f));
}

Vec3 getValFromSkyLut(Texture2D<Vec4> skyLut, SamplerState linearAnyClampSampler, Vec3 rayDir, Vec3 dirToSun)
{
	const F32 height = length(kViewPos);
	const Vec3 up = kViewPos / height;

	const F32 horizonAngle = safeacos(sqrt(height * height - kGroundRadiusMM * kGroundRadiusMM) / height);
	const F32 altitudeAngle = horizonAngle - acos(dot(rayDir, up)); // Between -PI/2 and PI/2
	F32 azimuthAngle; // Between 0 and 2*PI
	if(abs(altitudeAngle) > (0.5f * kPi - 0.0001f))
	{
		// Looking nearly straight up or down.
		azimuthAngle = 0.0f;
	}
	else
	{
		const Vec3 right = cross(dirToSun, up);
		const Vec3 forward = cross(up, right);

		const Vec3 projectedDir = normalize(rayDir - up * (dot(rayDir, up)));
		const F32 sinTheta = dot(projectedDir, right);
		const F32 cosTheta = dot(projectedDir, forward);
		azimuthAngle = atan2(sinTheta, cosTheta) + kPi;
	}

	// Non-linear mapping of altitude angle. See Section 5.3 of the paper.
	const F32 v = 0.5 + 0.5 * sign(altitudeAngle) * sqrt(abs(altitudeAngle) * 2.0 / kPi);
	const Vec2 uv = Vec2(azimuthAngle / (2.0f * kPi), 1.0 - v);

	return skyLut.SampleLevel(linearAnyClampSampler, uv, 0.0f).xyz;
}

Vec3 sunWithBloom(Vec3 rayDir, Vec3 dirToSun)
{
	const F32 sunSolidAngle = 0.53f * kPi / 180.0f;
	const F32 minSunCosTheta = cos(sunSolidAngle);

	const F32 cosTheta = dot(rayDir, dirToSun);
	if(cosTheta >= minSunCosTheta)
	{
		return 1.0f;
	}

	const F32 offset = minSunCosTheta - cosTheta;
	const F32 gaussianBloom = exp(-offset * 50000.0f) * 0.5f;
	const F32 invBloom = 1.0f / (0.02f + offset * 300.0f) * 0.01f;
	return gaussianBloom + invBloom;
}

/// Compute the sky color.
/// @param rayDir It's the vector: normalize(frag-cameraOrigin)
/// @param dirToSun It's opposite direction the light travels.
Vec3 computeSkyColor(Texture2D<Vec4> skyLut, SamplerState linearAnyClampSampler, Vec3 rayDir, Vec3 dirToSun, F32 sunPower, Bool drawSun)
{
	Vec3 col = getValFromSkyLut(skyLut, linearAnyClampSampler, rayDir, dirToSun);

	if(drawSun)
	{
		col += sunWithBloom(rayDir, dirToSun);
	}

	col *= sunPower;

	return col;
}

template<typename T, typename Y>
vector<T, 3> sampleSkyCheap(Sky sky, vector<Y, 3> direction, SamplerState trilinearClampSampler)
{
	vector<T, 3> color;
	if(sky.m_type == (U32)SkyType::kSolidColor)
	{
		color = sky.m_solidColor;
	}
	else
	{
		const Vec2 uv =
			(sky.m_type == (U32)SkyType::kTextureWithEquirectangularMapping) ? equirectangularMapping(direction) : octahedronEncode(direction);
		color = getBindlessTexture2DVec4(sky.m_texture).SampleLevel(trilinearClampSampler, uv, 0.0).xyz;
	}

	return color;
}

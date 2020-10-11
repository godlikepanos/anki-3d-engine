// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// NOTE: To visualize some of these functions go to https://www.shadertoy.com/view/wsBBzV

#pragma once

#include <anki/shaders/Common.glsl>

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// Using bitfieldReverse instead of bitwise ops
F32 radicalInverseVdC(U32 bits)
{
	bits = bitfieldReverse(bits);
	return F32(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
Vec2 hammersley2d(U32 i, U32 N)
{
	return Vec2(F32(i) / F32(N), radicalInverseVdC(i));
}

// Stolen from Unreal
// Returns three elements with 16 random bits each (0-0xffff)
UVec3 rand3DPCG16(UVec3 v)
{
	v = v * 1664525u + 1013904223u;

	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;

	return v >> 16u;
}

// Stolen from Unreal
// It will return a uniform 2D point inside [0.0, 1.0]. For random use rand3DPCG16()
Vec2 hammersleyRandom16(U32 sampleIdx, U32 sampleCount, UVec2 random)
{
	const F32 e1 = fract(F32(sampleIdx) / sampleCount + F32(random.x) * (1.0 / 65536.0));
	const F32 e2 = F32((bitfieldReverse(sampleIdx) >> 16) ^ random.y) * (1.0 / 65536.0);
	return Vec2(e1, e2);
}

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// From a uniform 2D point inside a circle get a 3D point in the surface of a hemisphere. It's oriented in the z axis
Vec3 hemisphereSampleUniform(Vec2 uv)
{
	const F32 phi = uv.y * 2.0 * PI;
	const F32 cosTheta = 1.0 - uv.x;
	const F32 sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return Vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// Same as hemisphereSampleUniform but it distributes points closer to the z axis
Vec3 hemisphereSampleCos(Vec2 uv)
{
	const F32 phi = uv.y * 2.0 * PI;
	const F32 cosTheta = sqrt(1.0 - uv.x);
	const F32 sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return Vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// NOTE: To visualize some of these functions go to https://www.shadertoy.com/view/wsBBzV

#pragma once

#include <AnKi/Shaders/FastMathFunctions.hlsl>

/// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
/// Using reversebits instead of bitwise ops
F32 radicalInverseVdC(U32 bits)
{
	bits = reversebits(bits);
	return F32(bits) * 2.3283064365386963e-10; // / 0x100000000
}

/// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
Vec2 hammersley2d(U32 i, U32 N)
{
	return Vec2(F32(i) / F32(N), radicalInverseVdC(i));
}

/// Stolen from Unreal
/// Returns three elements with 16 random bits each (0-0xffff)
///
/// Use it like that:
/// UVec3 seed = rand3DPCG16(UVec3(coord, frame % 8u));
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

/// Stolen from Unreal
/// It will return a uniform 2D point inside [0.0, 1.0]. For random use rand3DPCG16()
///
/// Use it like that:
/// Vec2 randFactors = hammersleyRandom16(sample, sampleCount, rand3DPCG16(...));
Vec2 hammersleyRandom16(U32 sampleIdx, U32 sampleCount, UVec2 random)
{
	const F32 e1 = frac(F32(sampleIdx) / F32(sampleCount) + F32(random.x) * (1.0 / 65536.0));
	const F32 e2 = F32((reversebits(sampleIdx) >> 16u) ^ random.y) * (1.0 / 65536.0);
	return Vec2(e1, e2);
}

/// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
/// From a uniform 2D point inside a circle get a 3D point in the surface of a hemisphere. It's oriented in the +Z. uv is in [0, 1]
///
/// Use it like that:
/// Vec3 dir = hemisphereSampleCos(hammersleyRandom16(...));
Vec3 hemisphereSampleUniform(Vec2 uv)
{
	const F32 phi = uv.y * 2.0 * kPi;
	const F32 cosTheta = 1.0 - uv.x;
	const F32 sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return Vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

/// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
/// Same as hemisphereSampleUniform but it distributes points closer to the z axis
Vec3 hemisphereSampleCos(Vec2 uv)
{
	const F32 phi = uv.y * 2.0 * kPi;
	const F32 cosTheta = sqrt(1.0 - uv.x);
	const F32 sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return Vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

/// PCG hash function.
U32 hashPcg(U32 u)
{
	const U32 state = u * 747796405u + 2891336453u;
	const U32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

U32 hilbertIndex(U32 posX, U32 posY)
{
	const U32 level = 3u;
	const U32 width = 1u << level;

	U32 index = 0u;
	for(U32 curLevel = width / 2u; curLevel > 0u; curLevel /= 2u)
	{
		const U32 regionX = (posX & curLevel) > 0u;
		const U32 regionY = (posY & curLevel) > 0u;
		index += curLevel * curLevel * ((3u * regionX) ^ regionY);
		if(regionY == 0U)
		{
			if(regionX == 1U)
			{
				posX = U32(width - 1u) - posX;
				posY = U32(width - 1U) - posY;
			}

			const U32 temp = posX;
			posX = posY;
			posY = temp;
		}
	}

	return index;
}

/// Taken from XeGTAO code
Vec2 spatioTemporalNoise(UVec2 fragCoord, U32 temporalIdx)
{
	U32 index = hilbertIndex(fragCoord.x, fragCoord.y);
	index += 288u * (temporalIdx % 64u);
	return Vec2(frac(0.5f + index * Vec2(0.75487766624669276005f, 0.5698402909980532659114f)));
}

/// Generates a point on the unit sphere. The frameIndex can be used as a randomizer. No need to modulate the frameIndex.
/// Usage:
/// for(U32 s = 0; s < 32; ++s) {
///		Vec3 point = generateUniformPointOnSphere(s, 32, frameIndex);
/// }
template<typename T, typename TInt>
vector<T, 3> generateUniformPointOnSphere(TInt sampleIndex, TInt sampleCount, U32 frameIndex)
{
	const T sampleIndexf = sampleIndex;
	const T sampleCountf = sampleCount;

	// Apply frame-dependent phase shift for randomness
	const T phaseShift = (frameIndex % sampleCount) * (T(k2Pi) / sampleCountf);

	// Compute spherical coordinates
	const T goldenRatio = 1.618;
	const T theta = fastAcos(T(1) - T(2) * (sampleIndexf + T(0.5)) / sampleCountf); // Evenly spaced latitudes
	const T phi = fmod((T(k2Pi * goldenRatio) * sampleIndexf + phaseShift), T(k2Pi)); // Golden ratio spiral with phase shift

	// Convert to Cartesian coordinates
	const T x = fastSin(theta) * fastCos(phi);
	const T y = fastSin(theta) * fastSin(phi);
	const T z = fastCos(theta);

	return normalize(vector<T, 3>(x, y, z));
}

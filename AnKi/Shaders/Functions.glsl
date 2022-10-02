// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.glsl>

#if defined(ANKI_FRAGMENT_SHADER)
Vec3 dither(Vec3 col, F32 C)
{
	Vec3 vDither = Vec3(dot(Vec2(171.0, 231.0), gl_FragCoord.xy));
	vDither.rgb = fract(vDither.rgb / Vec3(103.0, 71.0, 97.0));

	col = col * (255.0 / C) + vDither.rgb;
	col = floor(col) / 255.0;
	col *= C;

	return col;
}

F32 dither(F32 col, F32 C)
{
	F32 vDither = dot(Vec2(171.0, 231.0), gl_FragCoord.xy);
	vDither = fract(vDither / 103.0);

	col = col * (255.0 / C) + vDither;
	col = floor(col) / 255.0;
	col *= C;

	return col;
}
#endif

// Convert to linear depth
F32 linearizeDepth(F32 depth, F32 zNear, F32 zFar)
{
	return zNear / ((zNear - zFar) + zFar / depth);
}

// Convert to linear depth
Vec4 linearizeDepth(Vec4 depth, F32 zNear, F32 zFar)
{
	return zNear / ((zNear - zFar) + zFar / depth);
}

// This is the optimal linearizeDepth where a=(n-f)/n and b=f/n
F32 linearizeDepthOptimal(F32 depth, F32 a, F32 b)
{
	return 1.0 / (a + b / depth);
}

// This is the optimal linearizeDepth where a=(n-f)/n and b=f/n
Vec4 linearizeDepthOptimal(Vec4 depths, F32 a, F32 b)
{
	return 1.0 / (a + b / depths);
}

// Project a vector by knowing only the non zero values of a perspective matrix
Vec4 projectPerspective(Vec4 vec, F32 m00, F32 m11, F32 m22, F32 m23)
{
	Vec4 o;
	o.x = vec.x * m00;
	o.y = vec.y * m11;
	o.z = vec.z * m22 + vec.w * m23;
	o.w = -vec.z;
	return o;
}

#if defined(ANKI_FRAGMENT_SHADER)
// Stolen from shadertoy.com/view/4tyGDD
Vec4 textureCatmullRom4Samples(texture2D tex, sampler sampl, Vec2 uv, Vec2 texSize)
{
	const Vec2 halff = 2.0 * fract(0.5 * uv * texSize - 0.25) - 1.0;
	const Vec2 f = fract(halff);
	const Vec2 sum0 = (2.0 * f - 3.5) * f + 0.5;
	const Vec2 sum1 = (2.0 * f - 2.5) * f - 0.5;
	Vec4 w = Vec4(f * sum0 + 1.0, f * sum1);
	const Vec4 pos = Vec4((((-2.0 * f + 3.0) * f + 0.5) * f - 1.5) * f / (w.xy * texSize) + uv,
						  (((-2.0 * f + 5.0) * f - 2.5) * f - 0.5) / (sum1 * texSize) + uv);
	w.xz *= halff.x * halff.y > 0.0 ? 1.0 : -1.0;

	return (texture(tex, sampl, pos.xy) * w.x + texture(tex, sampl, pos.zy) * w.z) * w.y
		   + (texture(tex, sampl, pos.xw) * w.x + texture(tex, sampl, pos.zw) * w.z) * w.w;
}
#endif

// Stolen from shadertoy.com/view/4df3Dn
Vec4 textureBicubic(texture2D tex, sampler sampl, Vec2 uv, F32 lod, Vec2 texSize)
{
#define w0(a) ((1.0 / 6.0) * ((a) * ((a) * (-(a) + 3.0) - 3.0) + 1.0))
#define w1(a) ((1.0 / 6.0) * ((a) * (a) * (3.0 * (a)-6.0) + 4.0))
#define w2(a) ((1.0 / 6.0) * ((a) * ((a) * (-3.0 * (a) + 3.0) + 3.0) + 1.0))
#define w3(a) ((1.0 / 6.0) * ((a) * (a) * (a)))
#define g0(a) (w0(a) + w1(a))
#define g1(a) (w2(a) + w3(a))
#define h0(a) (-1.0 + w1(a) / (w0(a) + w1(a)))
#define h1(a) (1.0 + w3(a) / (w2(a) + w3(a)))
#define texSample(uv) textureLod(tex, sampl, uv, lod)

	uv = uv * texSize + 0.5;
	const Vec2 iuv = floor(uv);
	const Vec2 fuv = fract(uv);

	const F32 g0x = g0(fuv.x);
	const F32 g1x = g1(fuv.x);
	const F32 h0x = h0(fuv.x);
	const F32 h1x = h1(fuv.x);
	const F32 h0y = h0(fuv.y);
	const F32 h1y = h1(fuv.y);

	const Vec2 p0 = (Vec2(iuv.x + h0x, iuv.y + h0y) - 0.5) / texSize;
	const Vec2 p1 = (Vec2(iuv.x + h1x, iuv.y + h0y) - 0.5) / texSize;
	const Vec2 p2 = (Vec2(iuv.x + h0x, iuv.y + h1y) - 0.5) / texSize;
	const Vec2 p3 = (Vec2(iuv.x + h1x, iuv.y + h1y) - 0.5) / texSize;

	return g0(fuv.y) * (g0x * texSample(p0) + g1x * texSample(p1))
		   + g1(fuv.y) * (g0x * texSample(p2) + g1x * texSample(p3));

#undef w0
#undef w1
#undef w2
#undef g0
#undef g1
#undef h0
#undef h1
#undef texSample
}

F32 rand(Vec2 n)
{
	return 0.5 + 0.5 * fract(sin(dot(n, Vec2(12.9898, 78.233))) * 43758.5453);
}

Vec4 nearestDepthUpscale(Vec2 uv, texture2D depthFull, texture2D depthHalf, texture2D colorTex,
						 sampler linearAnyClampSampler, Vec2 linearDepthCf, F32 depthThreshold)
{
	F32 fullDepth = textureLod(depthFull, linearAnyClampSampler, uv, 0.0).r; // Sampler not important.
	fullDepth = linearizeDepthOptimal(fullDepth, linearDepthCf.x, linearDepthCf.y);

	Vec4 halfDepths = textureGather(sampler2D(depthHalf, linearAnyClampSampler), uv, 0); // Sampler not important.
	halfDepths = linearizeDepthOptimal(halfDepths, linearDepthCf.x, linearDepthCf.y);

	const Vec4 diffs = abs(Vec4(fullDepth) - halfDepths);
	Vec4 color;

	if(all(lessThan(diffs, Vec4(depthThreshold))))
	{
		// No major discontinuites, sample with bilinear
		color = textureLod(colorTex, linearAnyClampSampler, uv, 0.0);
	}
	else
	{
		// Some discontinuites, need to use the newUv
		const Vec4 r = textureGather(sampler2D(colorTex, linearAnyClampSampler), uv, 0);
		const Vec4 g = textureGather(sampler2D(colorTex, linearAnyClampSampler), uv, 1);
		const Vec4 b = textureGather(sampler2D(colorTex, linearAnyClampSampler), uv, 2);
		const Vec4 a = textureGather(sampler2D(colorTex, linearAnyClampSampler), uv, 3);

		F32 minDiff = diffs.x;
		U32 comp = 0u;

		if(diffs.y < minDiff)
		{
			comp = 1u;
			minDiff = diffs.y;
		}

		if(diffs.z < minDiff)
		{
			comp = 2u;
			minDiff = diffs.z;
		}

		if(diffs.w < minDiff)
		{
			comp = 3u;
		}

		color = Vec4(r[comp], g[comp], b[comp], a[comp]);
	}

	return color;
}

F32 _calcDepthWeight(texture2D depthLow, sampler nearestAnyClamp, Vec2 uv, F32 ref, Vec2 linearDepthCf)
{
	const F32 d = textureLod(depthLow, nearestAnyClamp, uv, 0.0).r;
	const F32 linearD = linearizeDepthOptimal(d, linearDepthCf.x, linearDepthCf.y);
	return 1.0 / (kEpsilonf + abs(ref - linearD));
}

Vec4 _sampleAndWeight(texture2D depthLow, texture2D colorLow, sampler linearAnyClamp, sampler nearestAnyClamp,
					  const Vec2 lowInvSize, Vec2 uv, const Vec2 offset, const F32 ref, const F32 weight,
					  const Vec2 linearDepthCf, inout F32 normalize)
{
	uv += offset * lowInvSize;
	const F32 dw = _calcDepthWeight(depthLow, nearestAnyClamp, uv, ref, linearDepthCf);
	const Vec4 v = textureLod(colorLow, linearAnyClamp, uv, 0.0);
	normalize += weight * dw;
	return v * dw * weight;
}

Vec4 bilateralUpsample(texture2D depthHigh, texture2D depthLow, texture2D colorLow, sampler linearAnyClamp,
					   sampler nearestAnyClamp, const Vec2 lowInvSize, const Vec2 uv, const Vec2 linearDepthCf)
{
	const Vec3 kWeights = Vec3(0.25, 0.125, 0.0625);
	const F32 depthRef =
		linearizeDepthOptimal(textureLod(depthHigh, nearestAnyClamp, uv, 0.0).r, linearDepthCf.x, linearDepthCf.y);
	F32 normalize = 0.0;

	Vec4 sum = _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(0.0, 0.0),
								depthRef, kWeights.x, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(-1.0, 0.0),
							depthRef, kWeights.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(0.0, -1.0),
							depthRef, kWeights.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(1.0, 0.0),
							depthRef, kWeights.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(0.0, 1.0),
							depthRef, kWeights.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(1.0, 1.0),
							depthRef, kWeights.z, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(1.0, -1.0),
							depthRef, kWeights.z, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(-1.0, 1.0),
							depthRef, kWeights.z, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(-1.0, -1.0),
							depthRef, kWeights.z, linearDepthCf, normalize);

	return sum / normalize;
}

Vec3 getCubemapDirection(const Vec2 norm, const U32 faceIdx)
{
	Vec3 zDir = Vec3((faceIdx <= 1u) ? 1 : 0, (faceIdx & 2u) >> 1u, (faceIdx & 4u) >> 2u);
	zDir *= (((faceIdx & 1u) == 1u) ? -1.0 : 1.0);
	const Vec3 yDir = (faceIdx == 2u)   ? Vec3(0.0, 0.0, 1.0)
					  : (faceIdx == 3u) ? Vec3(0.0, 0.0, -1.0)
										: Vec3(0.0, -1.0, 0.0);
	const Vec3 xDir = cross(zDir, yDir);
	return normalize(norm.x * xDir + norm.y * yDir + zDir);
}

// Convert 3D cubemap coordinates to 2D plus face index. v doesn't need to be normalized.
Vec2 convertCubeUvs(const Vec3 v, out F32 faceIndex)
{
	const Vec3 absV = abs(v);
	F32 mag;
	Vec2 uv;

	if(all(greaterThanEqual(absV.zz, absV.xy)))
	{
		faceIndex = (v.z < 0.0) ? 5.0 : 4.0;
		uv = Vec2((v.z < 0.0) ? -v.x : v.x, -v.y);
		mag = absV.z;
	}
	else if(absV.y >= absV.x)
	{
		faceIndex = (v.y < 0.0) ? 3.0 : 2.0;
		uv = Vec2(v.x, (v.y < 0.0) ? -v.z : v.z);
		mag = absV.y;
	}
	else
	{
		faceIndex = (v.x < 0.0) ? 1.0 : 0.0;
		uv = Vec2((v.x < 0.0) ? v.z : -v.z, -v.y);
		mag = absV.x;
	}

	return 0.5 / mag * uv + 0.5;
}

// Same as convertCubeUvs but it returns the faceIndex as unsigned I32.
Vec2 convertCubeUvsu(const Vec3 v, out U32 faceIndex)
{
	const Vec3 absV = abs(v);
	F32 mag;
	Vec2 uv;

	if(all(greaterThanEqual(absV.zz, absV.xy)))
	{
		faceIndex = (v.z < 0.0) ? 5u : 4u;
		uv = Vec2((v.z < 0.0) ? -v.x : v.x, -v.y);
		mag = absV.z;
	}
	else if(absV.y >= absV.x)
	{
		faceIndex = (v.y < 0.0) ? 3u : 2u;
		uv = Vec2(v.x, (v.y < 0.0) ? -v.z : v.z);
		mag = absV.y;
	}
	else
	{
		faceIndex = (v.x < 0.0) ? 1u : 0u;
		uv = Vec2((v.x < 0.0) ? v.z : -v.z, -v.y);
		mag = absV.x;
	}

	return 0.5 / mag * uv + 0.5;
}

Vec3 grayScale(const Vec3 col)
{
	const F32 grey = (col.r + col.g + col.b) * (1.0 / 3.0);
	return Vec3(grey);
}

Vec3 saturateColor(const Vec3 col, const F32 factor)
{
	const Vec3 lumCoeff = Vec3(0.2125, 0.7154, 0.0721);
	const Vec3 intensity = Vec3(dot(col, lumCoeff));
	return mix(intensity, col, factor);
}

Vec3 gammaCorrection(Vec3 gamma, Vec3 col)
{
	return pow(col, 1.0 / gamma);
}

// Can use 0.15 for sharpenFactor
Vec3 readSharpen(texture2D tex, sampler sampl, Vec2 uv, F32 sharpenFactor, Bool detailed)
{
	Vec3 col = textureLod(tex, sampl, uv, 0.0).rgb;

	Vec3 col2 = textureLodOffset(sampler2D(tex, sampl), uv, 0.0, IVec2(1, 1)).rgb;
	col2 += textureLodOffset(sampler2D(tex, sampl), uv, 0.0, IVec2(-1, -1)).rgb;
	col2 += textureLodOffset(sampler2D(tex, sampl), uv, 0.0, IVec2(1, -1)).rgb;
	col2 += textureLodOffset(sampler2D(tex, sampl), uv, 0.0, IVec2(-1, 1)).rgb;

	F32 f = 4.0;
	if(detailed)
	{
		col2 += textureLodOffset(sampler2D(tex, sampl), uv, 0.0, IVec2(0, 1)).rgb;
		col2 += textureLodOffset(sampler2D(tex, sampl), uv, 0.0, IVec2(1, 0)).rgb;
		col2 += textureLodOffset(sampler2D(tex, sampl), uv, 0.0, IVec2(-1, 0)).rgb;
		col2 += textureLodOffset(sampler2D(tex, sampl), uv, 0.0, IVec2(0, -1)).rgb;

		f = 8.0;
	}

	col = col * (f * sharpenFactor + 1.0) - sharpenFactor * col2;
	return max(Vec3(0.0), col);
}

Vec3 readErosion(texture2D tex, sampler sampl, const Vec2 uv)
{
	Vec3 minValue = textureLod(tex, sampl, uv, 0.0).rgb;

#define ANKI_EROSION(x, y) \
	col2 = textureLodOffset(sampler2D(tex, sampl), uv, 0.0, IVec2(x, y)).rgb; \
	minValue = min(col2, minValue);

	Vec3 col2;
	ANKI_EROSION(1, 1);
	ANKI_EROSION(-1, -1);
	ANKI_EROSION(1, -1);
	ANKI_EROSION(-1, 1);
	ANKI_EROSION(0, 1);
	ANKI_EROSION(1, 0);
	ANKI_EROSION(-1, 0);
	ANKI_EROSION(0, -1);

#undef ANKI_EROSION

	return minValue;
}

// 5 color heatmap from a factor.
Vec3 heatmap(const F32 factor)
{
	F32 intPart;
	const F32 fractional = modf(factor * 4.0, intPart);

	if(intPart < 1.0)
	{
		return mix(Vec3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0), fractional);
	}
	else if(intPart < 2.0)
	{
		return mix(Vec3(0.0, 0.0, 1.0), Vec3(0.0, 1.0, 0.0), fractional);
	}
	else if(intPart < 3.0)
	{
		return mix(Vec3(0.0, 1.0, 0.0), Vec3(1.0, 1.0, 0.0), fractional);
	}
	else
	{
		return mix(Vec3(1.0, 1.0, 0.0), Vec3(1.0, 0.0, 0.0), fractional);
	}
}

// Return a color per cubemap face. The +X is red, -X dark red, +Y green, -Y dark green, +Z blue, -Z dark blue
Vec3 colorPerCubeFace(const U32 dir)
{
	Vec3 color;
	switch(dir)
	{
	case 0:
		color = Vec3(1.0, 0.0, 0.0);
		break;
	case 1:
		color = Vec3(0.25, 0.0, 0.0);
		break;
	case 2:
		color = Vec3(0.0, 1.0, 0.0);
		break;
	case 3:
		color = Vec3(0.0, 0.25, 0.0);
		break;
	case 4:
		color = Vec3(0.0, 0.0, 1.0);
		break;
	default:
		color = Vec3(0.0, 0.0, 0.25);
	}
	return color;
}

Bool incorrectColor(const Vec3 c)
{
	return isnan(c.x) || isnan(c.y) || isnan(c.z) || isinf(c.x) || isinf(c.y) || isinf(c.z);
}

F32 areaElement(const F32 x, const F32 y)
{
	return atan(x * y, sqrt(x * x + y * y + 1.0));
}

// Compute the solid angle of a cube. Solid angle is the area of a sphere when projected into a cubemap. It's also the
// delta omega (dω) in the irradiance integral and other integrals that operate in a sphere.
// http://www.rorydriscoll.com/2012/01/15/cubemap-texel-solid-angle/
F32 cubeCoordSolidAngle(Vec2 norm, F32 cubeFaceSize)
{
	const Vec2 invSize = Vec2(1.0 / cubeFaceSize);
	const Vec2 v0 = norm - invSize;
	const Vec2 v1 = norm + invSize;
	return areaElement(v0.x, v0.y) - areaElement(v0.x, v1.y) - areaElement(v1.x, v0.y) + areaElement(v1.x, v1.y);
}

// A convenience function to skip out of bounds invocations on post-process compute shaders. Both the arguments should
// be constexpr.
#if defined(ANKI_COMPUTE_SHADER)
Bool skipOutOfBoundsInvocations(UVec2 workgroupSize, UVec2 globalInvocationCount)
{
	if((globalInvocationCount.x % workgroupSize.x) != 0u || (globalInvocationCount.y % workgroupSize.y) != 0u)
	{
		if(gl_GlobalInvocationID.x >= globalInvocationCount.x || gl_GlobalInvocationID.y >= globalInvocationCount.y)
		{
			return true;
		}
	}

	return false;
}
#endif

// Create a matrix from some direction.
Mat3 rotationFromDirection(Vec3 zAxis)
{
#if 0
	const Vec3 z = zAxis;
	const Bool alignsWithXBasis = abs(z.x - 1.0) <= kEpsilonf; // aka z == Vec3(1.0, 0.0, 0.0)
	Vec3 x = (alignsWithXBasis) ? Vec3(0.0, 0.0, 1.0) : Vec3(1.0, 0.0, 0.0);
	const Vec3 y = normalize(cross(x, z));
	x = normalize(cross(z, y));
	return Mat3(x, y, z);
#else
	// http://jcgt.org/published/0006/01/01/
	const Vec3 z = zAxis;
	const F32 sign = (z.z >= 0.0) ? 1.0 : -1.0;
	const F32 a = -1.0 / (sign + z.z);
	const F32 b = z.x * z.y * a;

	const Vec3 x = Vec3(1.0 + sign * a * pow(z.x, 2.0), sign * b, -sign * z.x);
	const Vec3 y = Vec3(b, sign + a * pow(z.y, 2.0), -z.y);

	return Mat3(x, y, z);
#endif
}

#if defined(ANKI_COMPUTE_SHADER)
// See getOptimalGlobalInvocationId8x8Amd
U32 _ABfiM(U32 src, U32 ins, U32 bits)
{
	const U32 mask = (1u << bits) - 1u;
	return (ins & mask) | (src & (~mask));
}

// See getOptimalGlobalInvocationId8x8Amd
U32 _ABfe(U32 src, U32 off, U32 bits)
{
	const U32 mask = (1u << bits) - 1u;
	return (src >> off) & mask;
}

// See getOptimalGlobalInvocationId8x8Amd
UVec2 _ARmpRed8x8(U32 a)
{
	return UVec2(_ABfiM(_ABfe(a, 2u, 3u), a, 1u), _ABfiM(_ABfe(a, 3u, 3u), _ABfe(a, 1u, 2u), 2u));
}

// https://github.com/GPUOpen-Effects/FidelityFX-CAS/blob/master/ffx-cas/ffx_a.h
UVec2 getOptimalGlobalInvocationId8x8Amd()
{
	const UVec2 localInvocationId = _ARmpRed8x8(gl_LocalInvocationIndex);
	return gl_WorkGroupID.xy * UVec2(8u) + localInvocationId;
}

// https://github.com/LouisBavoil/ThreadGroupIDSwizzling/blob/master/ThreadGroupTilingX.hlsl
UVec2 getOptimalGlobalInvocationId8x8Nvidia()
{
	const U32 maxTileWidth = 8u;
	const UVec2 workgroupSize = UVec2(8u);

	const U32 workgroupsInAPerfectTile = maxTileWidth * gl_NumWorkGroups.y;

	const U32 perfectTileCount = gl_NumWorkGroups.x / maxTileWidth;

	const U32 totalWorkgroupsInAllPerfectTiles = perfectTileCount * maxTileWidth * gl_NumWorkGroups.y;
	const U32 vThreadGroupIDFlattened = gl_NumWorkGroups.x * gl_WorkGroupID.y + gl_WorkGroupID.x;

	const U32 tileIdOfCurrentWorkgroup = vThreadGroupIDFlattened / workgroupsInAPerfectTile;
	const U32 localWorkgroupIdWithinCurrentTile = vThreadGroupIDFlattened % workgroupsInAPerfectTile;
	U32 localWorkgroupIdYWithinCurrentTile;
	U32 localWorgroupIdXWithinCurrentTile;

	if(totalWorkgroupsInAllPerfectTiles <= vThreadGroupIDFlattened)
	{
		U32 xDimensionOfLastTile = gl_NumWorkGroups.x % maxTileWidth;
		localWorkgroupIdYWithinCurrentTile = localWorkgroupIdWithinCurrentTile / xDimensionOfLastTile;
		localWorgroupIdXWithinCurrentTile = localWorkgroupIdWithinCurrentTile % xDimensionOfLastTile;
	}
	else
	{
		localWorkgroupIdYWithinCurrentTile = localWorkgroupIdWithinCurrentTile / maxTileWidth;
		localWorgroupIdXWithinCurrentTile = localWorkgroupIdWithinCurrentTile % maxTileWidth;
	}

	const U32 swizzledvThreadGroupIdFlattened = tileIdOfCurrentWorkgroup * maxTileWidth
												+ localWorkgroupIdYWithinCurrentTile * gl_NumWorkGroups.x
												+ localWorgroupIdXWithinCurrentTile;

	UVec2 swizzledvThreadGroupId;
	swizzledvThreadGroupId.y = swizzledvThreadGroupIdFlattened / gl_NumWorkGroups.x;
	swizzledvThreadGroupId.x = swizzledvThreadGroupIdFlattened % gl_NumWorkGroups.x;

	UVec2 swizzledGlobalId;
	swizzledGlobalId.x = workgroupSize.x * swizzledvThreadGroupId.x + gl_LocalInvocationID.x;
	swizzledGlobalId.y = workgroupSize.y * swizzledvThreadGroupId.y + gl_LocalInvocationID.y;

	return swizzledGlobalId.xy;
}
#endif

// Gaussian distrubution function
F32 gaussianWeight(F32 s, F32 x)
{
	F32 p = 1.0 / (s * sqrt(2.0 * kPi));
	p *= exp((x * x) / (-2.0 * s * s));
	return p;
}

Vec4 bilinearFiltering(texture2D tex, sampler nearestSampler, Vec2 uv, F32 lod, Vec2 textureSize)
{
	const Vec2 texelSize = 1.0 / textureSize;
	const Vec2 unnormTexCoord = (uv * textureSize) - 0.5;
	const Vec2 f = fract(unnormTexCoord);
	const Vec2 snapTexCoord = (floor(unnormTexCoord) + 0.5) / textureSize;
	const Vec4 s1 = textureLod(tex, nearestSampler, uv, lod);
	const Vec4 s2 = textureLod(tex, nearestSampler, uv + Vec2(texelSize.x, 0.0), lod);
	const Vec4 s3 = textureLod(tex, nearestSampler, uv + Vec2(0.0, texelSize.y), lod);
	const Vec4 s4 = textureLod(tex, nearestSampler, uv + texelSize, lod);
	return mix(mix(s1, s2, f.x), mix(s3, s4, f.x), f.y);
}

// https://www.shadertoy.com/view/WsfBDf
Vec3 animateBlueNoise(Vec3 inputBlueNoise, U32 frameIdx)
{
	const F32 goldenRatioConjugate = 0.61803398875;
	return fract(inputBlueNoise + F32(frameIdx % 64u) * goldenRatioConjugate);
}

#if defined(ANKI_FRAGMENT_SHADER)
/// https://bgolus.medium.com/distinctive-derivative-differences-cce38d36797b
/// normalizedUvs is uv*textureResolution
F32 computeMipLevel(Vec2 normalizedUvs)
{
	const Vec2 dx = dFdxCoarse(normalizedUvs);
	const Vec2 dy = dFdyCoarse(normalizedUvs);
	const F32 deltaMax2 = max(dot(dx, dx), dot(dy, dy));
	return max(0.0, 0.5 * log2(deltaMax2));
}
#endif

#if defined(U64)
/// The regular findLSB in glslang has some issues since it invokes a builtin that is only supposed to be used with
/// 32bit input. This is an alternative implementation but it expects that the input is not zero.
I32 findLSB2(U64 v)
{
	const I32 lsb1 = findLSB(U32(v));
	const I32 lsb2 = findLSB(U32(v >> 32ul));
	return (lsb1 >= 0) ? lsb1 : lsb2 + 32;
}
#endif

/// Define an alternative findLSB to go in pair with the 64bit version.
I32 findLSB2(U32 v)
{
	return findLSB(v);
}

/// Encode the shading rate to be stored in an SRI. The rates should be power of two, can't be zero and can't exceed 4.
/// So the possible values are 1,2,4
U32 encodeVrsRate(UVec2 rateXY)
{
	return (rateXY.y >> 1u) | ((rateXY.x << 1u) & 12u);
}

Vec3 visualizeVrsRate(UVec2 rate)
{
	if(rate == UVec2(1u))
	{
		return Vec3(1.0, 0.0, 0.0);
	}
	else if(rate == UVec2(2u, 1u) || rate == UVec2(1u, 2u))
	{
		return Vec3(1.0, 0.5, 0.0);
	}
	else if(rate == UVec2(2u) || rate == UVec2(4u, 1u) || rate == UVec2(1u, 4u))
	{
		return Vec3(1.0, 1.0, 0.0);
	}
	else if(rate == UVec2(4u, 2u) || rate == UVec2(2u, 4u))
	{
		return Vec3(0.65, 1.0, 0.0);
	}
	else if(rate == UVec2(4u))
	{
		return Vec3(0.0, 1.0, 0.0);
	}
	else
	{
		return Vec3(0.0, 0.0, 0.0);
	}
}

/// Decodes a number produced by encodeVrsRate(). Returns the shading rates.
UVec2 decodeVrsRate(U32 texel)
{
	UVec2 rateXY;
	rateXY.x = 1u << ((texel >> 2u) & 3u);
	rateXY.y = 1u << (texel & 3u);
	return rateXY;
}

/// 3D coordinates to equirectangular 2D coordinates.
Vec2 equirectangularMapping(Vec3 v)
{
	Vec2 uv = Vec2(atan(v.z, v.x), asin(v.y));
	uv *= vec2(0.1591, 0.3183);
	uv += 0.5;
	return uv;
}

Vec3 linearToSRgb(Vec3 linearRgb)
{
	linearRgb = max(Vec3(6.10352e-5), linearRgb);
	return min(linearRgb * 12.92, pow(max(linearRgb, 0.00313067), Vec3(1.0 / 2.4)) * 1.055 - 0.055);
}

Vec3 sRgbToLinear(Vec3 sRgb)
{
	const bvec3 cutoff = lessThan(sRgb, Vec3(0.04045));
	const Vec3 higher = pow((sRgb + 0.055) / 1.055, Vec3(2.4));
	const Vec3 lower = sRgb / 12.92;
	return mix(higher, lower, cutoff);
}

ANKI_RP Vec3 filmGrain(ANKI_RP Vec3 color, Vec2 uv, ANKI_RP F32 strength, ANKI_RP F32 time)
{
	const F32 x = (uv.x + 4.0) * (uv.y + 4.0) * time;
	const F32 grain = 1.0 - (mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01) - 0.005) * strength;
	return color * grain;
}

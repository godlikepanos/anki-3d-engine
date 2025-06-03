// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.hlsl>

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

/// Project a vector by knowing only the non zero values of a perspective matrix. Doesn't take into account jitter.
Vec4 cheapPerspectiveProjection(F32 m00, F32 m11, F32 m22, F32 m23, Vec4 vec)
{
	Vec4 o;
	o.x = vec.x * m00;
	o.y = vec.y * m11;
	o.z = vec.z * m22 + vec.w * m23;
	o.w = -vec.z;
	return o;
}

/// Project a vector by knowing only the non zero values of a perspective matrix. Doesn't take into account jitter.
Vec4 cheapPerspectiveProjection(Vec4 projMat_00_11_22_23, Vec4 vec)
{
	return cheapPerspectiveProjection(projMat_00_11_22_23.x, projMat_00_11_22_23.y, projMat_00_11_22_23.z, projMat_00_11_22_23.w, vec);
}

/// To unproject to view space. Jitter not considered. See Mat4::extractPerspectiveUnprojectionParams in C++.
Vec3 cheapPerspectiveUnprojection(Vec4 unprojParams, Vec2 ndc, F32 depth)
{
	const F32 z = unprojParams.z / (unprojParams.w + depth);
	const Vec2 xy = ndc * unprojParams.xy * z;
	return Vec3(xy, z);
}

#if ANKI_PIXEL_SHADER
// Stolen from shadertoy.com/view/4tyGDD
Vec4 textureCatmullRom4Samples(Texture2D tex, SamplerState sampl, Vec2 uv, Vec2 texSize)
{
	const Vec2 halff = 2.0 * frac(0.5 * uv * texSize - 0.25) - 1.0;
	const Vec2 f = frac(halff);
	const Vec2 sum0 = (2.0 * f - 3.5) * f + 0.5;
	const Vec2 sum1 = (2.0 * f - 2.5) * f - 0.5;
	Vec4 w = Vec4(f * sum0 + 1.0, f * sum1);
	const Vec4 pos =
		Vec4((((-2.0 * f + 3.0) * f + 0.5) * f - 1.5) * f / (w.xy * texSize) + uv, (((-2.0 * f + 5.0) * f - 2.5) * f - 0.5) / (sum1 * texSize) + uv);
	w.xz *= halff.x * halff.y > 0.0 ? 1.0 : -1.0;

	return (tex.Sample(sampl, pos.xy) * w.x + tex.Sample(sampl, pos.zy) * w.z) * w.y
		   + (tex.Sample(sampl, pos.xw) * w.x + tex.Sample(sampl, pos.zw) * w.z) * w.w;
}
#endif

// Stolen from shadertoy.com/view/4df3Dn
template<typename TVec>
TVec textureBicubic(Texture2D<TVec> tex, SamplerState sampl, Vec2 uv, F32 lod)
{
#define w0(a) ((1.0 / 6.0) * ((a) * ((a) * (-(a) + 3.0) - 3.0) + 1.0))
#define w1(a) ((1.0 / 6.0) * ((a) * (a) * (3.0 * (a)-6.0) + 4.0))
#define w2(a) ((1.0 / 6.0) * ((a) * ((a) * (-3.0 * (a) + 3.0) + 3.0) + 1.0))
#define w3(a) ((1.0 / 6.0) * ((a) * (a) * (a)))
#define g0(a) (w0(a) + w1(a))
#define g1(a) (w2(a) + w3(a))
#define h0(a) (-1.0 + w1(a) / (w0(a) + w1(a)))
#define h1(a) (1.0 + w3(a) / (w2(a) + w3(a)))
#define texSample(uv) tex.SampleLevel(sampl, uv, lod)

	UVec2 texSize;
	U32 mipCount;
	tex.GetDimensions(0, texSize.x, texSize.y, mipCount);
	const U32 lodi = min(U32(lod), mipCount - 1u);
	texSize = texSize >> lodi;

	uv = uv * texSize + 0.5;
	const Vec2 iuv = floor(uv);
	const Vec2 fuv = frac(uv);

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

	return g0(fuv.y) * (g0x * texSample(p0) + g1x * texSample(p1)) + g1(fuv.y) * (g0x * texSample(p2) + g1x * texSample(p3));

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
	return 0.5 + 0.5 * frac(sin(dot(n, Vec2(12.9898, 78.233))) * 43758.5453);
}

Vec4 nearestDepthUpscale(Vec2 uv, Texture2D<Vec4> depthFull, Texture2D<Vec4> depthHalf, Texture2D<Vec4> colorTex, SamplerState linearAnyClampSampler,
						 Vec2 linearDepthCf, F32 depthThreshold)
{
	F32 fullDepth = depthFull.SampleLevel(linearAnyClampSampler, uv, 0.0).r; // Sampler not important.
	fullDepth = linearizeDepthOptimal(fullDepth, linearDepthCf.x, linearDepthCf.y);

	Vec4 halfDepths = depthHalf.GatherRed(linearAnyClampSampler, uv); // Sampler not important.
	halfDepths = linearizeDepthOptimal(halfDepths, linearDepthCf.x, linearDepthCf.y);

	const Vec4 diffs = abs(Vec4(fullDepth, fullDepth, fullDepth, fullDepth) - halfDepths);
	Vec4 color;

	if(all(diffs < Vec4(depthThreshold, depthThreshold, depthThreshold, depthThreshold)))
	{
		// No major discontinuites, sample with bilinear
		color = colorTex.SampleLevel(linearAnyClampSampler, uv, 0.0);
	}
	else
	{
		// Some discontinuites, need to use the newUv
		const Vec4 r = colorTex.GatherRed(linearAnyClampSampler, uv);
		const Vec4 g = colorTex.GatherGreen(linearAnyClampSampler, uv);
		const Vec4 b = colorTex.GatherBlue(linearAnyClampSampler, uv);
		const Vec4 a = colorTex.GatherAlpha(linearAnyClampSampler, uv);

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

F32 _calcDepthWeight(Texture2D depthLow, SamplerState nearestAnyClamp, Vec2 uv, F32 ref, Vec2 linearDepthCf)
{
	const F32 d = depthLow.SampleLevel(nearestAnyClamp, uv, 0.0).r;
	const F32 linearD = linearizeDepthOptimal(d, linearDepthCf.x, linearDepthCf.y);
	return 1.0 / (kEpsilonF32 + abs(ref - linearD));
}

Vec4 _sampleAndWeight(Texture2D depthLow, Texture2D colorLow, SamplerState linearAnyClamp, SamplerState nearestAnyClamp, const Vec2 lowInvSize,
					  Vec2 uv, const Vec2 offset, const F32 ref, const F32 weight, const Vec2 linearDepthCf, inout F32 normalize)
{
	uv += offset * lowInvSize;
	const F32 dw = _calcDepthWeight(depthLow, nearestAnyClamp, uv, ref, linearDepthCf);
	const Vec4 v = colorLow.SampleLevel(linearAnyClamp, uv, 0.0);
	normalize += weight * dw;
	return v * dw * weight;
}

Vec4 bilateralUpsample(Texture2D depthHigh, Texture2D depthLow, Texture2D colorLow, SamplerState linearAnyClamp, SamplerState nearestAnyClamp,
					   const Vec2 lowInvSize, const Vec2 uv, const Vec2 linearDepthCf)
{
	const Vec3 kWeights = Vec3(0.25, 0.125, 0.0625);
	F32 depthRef = depthHigh.SampleLevel(nearestAnyClamp, uv, 0.0).r;
	depthRef = linearizeDepthOptimal(depthRef, linearDepthCf.x, linearDepthCf.y);
	F32 normalize = 0.0;

	Vec4 sum = _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(0.0, 0.0), depthRef, kWeights.x,
								linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(-1.0, 0.0), depthRef, kWeights.y, linearDepthCf,
							normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(0.0, -1.0), depthRef, kWeights.y, linearDepthCf,
							normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(1.0, 0.0), depthRef, kWeights.y, linearDepthCf,
							normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(0.0, 1.0), depthRef, kWeights.y, linearDepthCf,
							normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(1.0, 1.0), depthRef, kWeights.z, linearDepthCf,
							normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(1.0, -1.0), depthRef, kWeights.z, linearDepthCf,
							normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(-1.0, 1.0), depthRef, kWeights.z, linearDepthCf,
							normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(-1.0, -1.0), depthRef, kWeights.z,
							linearDepthCf, normalize);

	return sum / normalize;
}

/// Compute the UV that can be passed to a cube texture.
/// (0.5, 0) returns {1, 0, 0}
/// (0.5, 1) returns {-1, 0, 0}
/// (0.5, 2) returns {0, 1, 0}
/// (0.5, 3) returns {0, -1, 0}
/// (0.5, 4) returns {0, 0, 1}
/// (0.5, 5) returns {0, 0, -1}
Vec3 getCubemapDirection(const Vec2 uv, const U32 faceIdx)
{
	const Vec2 norm = uv * 2.0 - 1.0;
	Vec3 zDir = Vec3((faceIdx <= 1u) ? 1 : 0, (faceIdx & 2u) >> 1u, (faceIdx & 4u) >> 2u);
	zDir *= (((faceIdx & 1u) == 1u) ? -1.0 : 1.0);
	const Vec3 yDir = (faceIdx == 2u) ? Vec3(0.0, 0.0, 1.0) : (faceIdx == 3u) ? Vec3(0.0, 0.0, -1.0) : Vec3(0.0, -1.0, 0.0);
	const Vec3 xDir = cross(zDir, yDir);
	return normalize(norm.x * xDir + norm.y * yDir + zDir);
}

/// Convert 3D cubemap coordinates to 2D plus face index. vec doesn't need to be normalized. It's the opposite of getCubemapDirection.
/// This is the exact same thing AMD is doing (v_cubeid and co) with a small difference. AMD for some reason adds 1.5 to the final result instead of
/// 0.5.
template<typename T>
Vec2 convertCubeUvs(const Vec3 vec, out T faceIndex)
{
	F32 u, v;
	const F32 x = vec.x;
	const F32 y = vec.y;
	const F32 z = vec.z;
	const F32 ax = abs(vec.x);
	const F32 ay = abs(vec.y);
	const F32 az = abs(vec.z);
	F32 major;

	if(az >= ax && az >= ay)
	{
		major = az;
		u = (z < 0.0f) ? -x : x;
		v = -y;
		faceIndex = (z < 0.0f) ? (T)5 : (T)4;
	}
	else if(ay >= ax)
	{
		major = ay;
		u = x;
		v = (y < 0.0f) ? -z : z;
		faceIndex = (y < 0.0f) ? (T)3 : (T)2;
	}
	else
	{
		major = ax;
		u = (x < 0.0f) ? z : -z;
		v = -y;
		faceIndex = (x < 0.0f) ? (T)1 : (T)0;
	}

	return Vec2(u, v) / (major * 2.0f) + 0.5f;
}

template<typename T>
vector<T, 3> grayScale(const vector<T, 3> col)
{
	const T grey = (col.r + col.g + col.b) * T(1.0 / 3.0);
	return vector<T, 3>(grey, grey, grey);
}

template<typename T>
vector<T, 3> saturateColor(const vector<T, 3> col, const T factor)
{
	const vector<T, 3> lumCoeff = vector<T, 3>(0.2125, 0.7154, 0.0721);
	const T d = dot(col, lumCoeff);
	const vector<T, 3> intensity = vector<T, 3>(d, d, d);
	return lerp(intensity, col, factor);
}

template<typename T>
vector<T, 3> gammaCorrection(vector<T, 3> gamma, vector<T, 3> col)
{
	return pow(col, T(1.0) / gamma);
}

// Can use 0.15 for sharpenFactor
template<typename T>
vector<T, 3> readSharpen(Texture2D<vector<T, 4> > tex, SamplerState sampl, Vec2 uv, T sharpenFactor, Bool detailed)
{
	vector<T, 3> col = tex.SampleLevel(sampl, uv, 0.0).rgb;

	vector<T, 3> col2 = tex.SampleLevel(sampl, uv, 0.0, IVec2(1, 1)).rgb;
	col2 += tex.SampleLevel(sampl, uv, 0.0, IVec2(-1, -1)).rgb;
	col2 += tex.SampleLevel(sampl, uv, 0.0, IVec2(1, -1)).rgb;
	col2 += tex.SampleLevel(sampl, uv, 0.0, IVec2(-1, 1)).rgb;

	T f = 4.0;
	if(detailed)
	{
		col2 += tex.SampleLevel(sampl, uv, 0.0, IVec2(0, 1)).rgb;
		col2 += tex.SampleLevel(sampl, uv, 0.0, IVec2(1, 0)).rgb;
		col2 += tex.SampleLevel(sampl, uv, 0.0, IVec2(-1, 0)).rgb;
		col2 += tex.SampleLevel(sampl, uv, 0.0, IVec2(0, -1)).rgb;

		f = 8.0;
	}

	col = col * (f * sharpenFactor + T(1.0)) - sharpenFactor * col2;
	return max(vector<T, 3>(0.0, 0.0, 0.0), col);
}

template<typename T>
vector<T, 3> readErosion(Texture2D<vector<T, 4> > tex, SamplerState sampl, const Vec2 uv)
{
	vector<T, 3> minValue = tex.SampleLevel(sampl, uv, 0.0).rgb;

#define ANKI_EROSION(x, y) \
	col2 = tex.SampleLevel(sampl, uv, 0.0, IVec2(x, y)).rgb; \
	minValue = min(col2, minValue);

	vector<T, 3> col2;
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
		return lerp(Vec3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0), fractional);
	}
	else if(intPart < 2.0)
	{
		return lerp(Vec3(0.0, 0.0, 1.0), Vec3(0.0, 1.0, 0.0), fractional);
	}
	else if(intPart < 3.0)
	{
		return lerp(Vec3(0.0, 1.0, 0.0), Vec3(1.0, 1.0, 0.0), fractional);
	}
	else
	{
		return lerp(Vec3(1.0, 1.0, 0.0), Vec3(1.0, 0.0, 0.0), fractional);
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
	return atan2(x * y, sqrt(x * x + y * y + 1.0));
}

// Compute the solid angle of a cube. Solid angle is the area of a sphere when projected into a cubemap. It's also the
// delta omega (dω) in the irradiance integral and other integrals that operate in a sphere.
// http://www.rorydriscoll.com/2012/01/15/cubemap-texel-solid-angle/
F32 cubeCoordSolidAngle(Vec2 norm, F32 cubeFaceSize)
{
	const F32 s = 1.0f / cubeFaceSize;
	const Vec2 invSize = Vec2(s, s);
	const Vec2 v0 = norm - invSize;
	const Vec2 v1 = norm + invSize;
	return areaElement(v0.x, v0.y) - areaElement(v0.x, v1.y) - areaElement(v1.x, v0.y) + areaElement(v1.x, v1.y);
}

/// A convenience function to skip out of bounds invocations on post-process compute shaders.
Bool skipOutOfBoundsInvocations(UVec2 groupSize, UVec2 threadCount, UVec2 svDispatchThreadId)
{
	if((threadCount.x % groupSize.x) != 0u || (threadCount.y % groupSize.y) != 0u)
	{
		if(svDispatchThreadId.x >= threadCount.x || svDispatchThreadId.y >= threadCount.y)
		{
			return true;
		}
	}

	return false;
}

// Create a matrix from some direction.
Mat3 rotationFromDirection(Vec3 zAxis)
{
#if 0
	const Vec3 z = zAxis;
	const Bool alignsWithXBasis = abs(z.x - 1.0) <= kEpsilonF32; // aka z == Vec3(1.0, 0.0, 0.0)
	Vec3 x = (alignsWithXBasis) ? Vec3(0.0, 0.0, 1.0) : Vec3(1.0, 0.0, 0.0);
	const Vec3 y = normalize(cross(x, z));
	x = normalize(cross(z, y));
#else
	// http://jcgt.org/published/0006/01/01/
	const Vec3 z = zAxis;
	const F32 sign = (z.z >= 0.0) ? 1.0 : -1.0;
	const F32 a = -1.0 / (sign + z.z);
	const F32 b = z.x * z.y * a;

	const Vec3 x = Vec3(1.0 + sign * a * pow(z.x, 2.0), sign * b, -sign * z.x);
	const Vec3 y = Vec3(b, sign + a * pow(z.y, 2.0), -z.y);
#endif

	Mat3 o;
	o.setColumns(x, y, z);
	return o;
}

#if ANKI_COMPUTE_SHADER && ANKI_GLSL
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

	const U32 swizzledvThreadGroupIdFlattened =
		tileIdOfCurrentWorkgroup * maxTileWidth + localWorkgroupIdYWithinCurrentTile * gl_NumWorkGroups.x + localWorgroupIdXWithinCurrentTile;

	UVec2 swizzledvThreadGroupId;
	swizzledvThreadGroupId.y = swizzledvThreadGroupIdFlattened / gl_NumWorkGroups.x;
	swizzledvThreadGroupId.x = swizzledvThreadGroupIdFlattened % gl_NumWorkGroups.x;

	UVec2 swizzledGlobalId;
	swizzledGlobalId.x = workgroupSize.x * swizzledvThreadGroupId.x + gl_LocalInvocationID.x;
	swizzledGlobalId.y = workgroupSize.y * swizzledvThreadGroupId.y + gl_LocalInvocationID.y;

	return swizzledGlobalId.xy;
}
#endif

// Gaussian distrubution function. Play with the values here https://www.desmos.com/calculator/7oxmohg3ta
// s is the sigma and x is a factor where abs(x) is in [0, 1]
template<typename T>
T gaussianWeight(T s, T x)
{
	T p = T(1) / (s * sqrt(T(2) * kPi));
	p *= exp((x * x) / (T(-2) * s * s));
	return p;
}

template<typename T>
T gaussianWeight2d(T s, T x, T y)
{
	T p = T(1) / (T(2) * kPi * s * s);
	p *= exp((x * x + y * y) / (T(-2) * s * s));
	return p;
}

// https://www.shadertoy.com/view/WsfBDf
template<typename T>
vector<T, 3> animateBlueNoise(vector<T, 3> inputBlueNoise, U32 frameIdx)
{
	const T goldenRatioConjugate = 0.61803398875;
	return frac(inputBlueNoise + T(frameIdx % 64u) * goldenRatioConjugate);
}

#if ANKI_PIXEL_SHADER
/// https://bgolus.medium.com/distinctive-derivative-differences-cce38d36797b
/// normalizedUvs is uv*textureResolution
F32 computeMipLevel(Vec2 normalizedUvs)
{
	const Vec2 dx = ddx_coarse(normalizedUvs);
	const Vec2 dy = ddy_coarse(normalizedUvs);
	const F32 deltaMax2 = max(dot(dx, dx), dot(dy, dy));
	return max(0.0, 0.5 * log2(deltaMax2));
}
#endif

/// The regular firstbitlow in DXC has some issues since it invokes a builtin that is only supposed to be used with
/// 32bit input. This is an alternative implementation but it expects that the input is not zero.
I32 firstbitlow2(U64 v)
{
	const I32 lsb1 = firstbitlow((U32)v);
	const I32 lsb2 = firstbitlow((U32)(v >> 32ul));
	return (lsb1 >= 0) ? lsb1 : lsb2 + 32;
}

/// Define an alternative firstbitlow to go in pair with the 64bit version.
I32 firstbitlow2(U32 v)
{
	return firstbitlow(v);
}

/// The regular firstbitlow in DXC has some issues since it invokes a builtin that is only supposed to be used with
/// 32bit input. This is an alternative implementation but it expects that the input is not zero.
U32 countbits2(U64 v)
{
	return countbits(U32(v)) + countbits(U32(v >> 32ul));
}

/// Encode the shading rate to be stored in an SRI. The rates should be power of two, can't be zero and can't exceed 4.
/// So the possible values are 1,2,4
U32 encodeVrsRate(UVec2 rateXY)
{
	return (rateXY.y >> 1u) | ((rateXY.x << 1u) & 12u);
}

Vec3 visualizeVrsRate(UVec2 rate)
{
	if(all(rate == UVec2(1u, 1u)))
	{
		return Vec3(1.0, 0.0, 0.0);
	}
	else if(all(rate == UVec2(2u, 1u)) || all(rate == UVec2(1u, 2u)))
	{
		return Vec3(1.0, 0.5, 0.0);
	}
	else if(all(rate == UVec2(2u, 2u)) || all(rate == UVec2(4u, 1u)) || all(rate == UVec2(1u, 4u)))
	{
		return Vec3(1.0, 1.0, 0.0);
	}
	else if(all(rate == UVec2(4u, 2u)) || all(rate == UVec2(2u, 4u)))
	{
		return Vec3(0.65, 1.0, 0.0);
	}
	else if(all(rate == UVec2(4u, 4u)))
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
	Vec2 uv = Vec2(atan2(v.z, v.x), asin(v.y));
	uv *= Vec2(0.1591, 0.3183);
	uv += 0.5;
	return uv;
}

template<typename T>
vector<T, 3> linearToSRgb(vector<T, 3> linearRgb)
{
	constexpr T a = 6.10352e-5;
	constexpr T b = 1.0 / 2.4;
	linearRgb = max(vector<T, 3>(a, a, a), linearRgb);
	return min(linearRgb * T(12.92), pow(max(linearRgb, T(0.00313067)), Vec3(b, b, b)) * T(1.055) - T(0.055));
}

template<typename T>
vector<T, 3> sRgbToLinear(vector<T, 3> sRgb)
{
	const bool3 cutoff = sRgb < vector<T, 3>(0.04045, 0.04045, 0.04045);
	const vector<T, 3> higher = pow((sRgb + T(0.055)) / T(1.055), vector<T, 3>(2.4, 2.4, 2.4));
	const vector<T, 3> lower = sRgb / T(12.92);
	return lerp(higher, lower, cutoff);
}

template<typename T>
vector<T, 3> filmGrain(vector<T, 3> color, Vec2 uv, T strength, F32 time)
{
	const T x = (uv.x + 4.0) * (uv.y + 4.0) * time;
	const T grain = T(1.0) - (fmod((fmod(x, T(13.0)) + T(1.0)) * (fmod(x, T(123.0)) + T(1.0)), T(0.01)) - T(0.005)) * strength;
	return color * grain;
}

#if ANKI_COMPUTE_SHADER || ANKI_WORK_GRAPH_SHADER
/// HLSL doesn't have SubgroupID so compute it. It's a macro because we can't have functions that InterlockedAdd on local variables (the compiler
/// can't see it's groupshared).
/// @param svGroupIndex Self explanatory.
/// @param tmpGroupsharedU32Var A U32 groupshared variable that will help with the calculation.
/// @param waveIndexInsideThreadgroup The SubgroupID.
/// @param wavesPerThreadGroup Also calculate that in case some GPUs manage to mess this up.
#	define ANKI_COMPUTE_WAVE_INDEX_INSIDE_THREADGROUP(svGroupIndex, tmpGroupsharedU32Var, waveIndexInsideThreadgroup, wavesPerThreadGroup) \
		do \
		{ \
			if(svGroupIndex == 0) \
			{ \
				tmpGroupsharedU32Var = 0; \
			} \
			GroupMemoryBarrierWithGroupSync(); \
			waveIndexInsideThreadgroup = 0; \
			if(WaveIsFirstLane()) \
			{ \
				InterlockedAdd(tmpGroupsharedU32Var, 1, waveIndexInsideThreadgroup); \
			} \
			GroupMemoryBarrierWithGroupSync(); \
			wavesPerThreadGroup = tmpGroupsharedU32Var; \
			waveIndexInsideThreadgroup = WaveReadLaneFirst(waveIndexInsideThreadgroup); \
		} while(false)
#endif

/// Perturb normal, see http://www.thetenthplanet.de/archives/1180
/// Does normal mapping in the fragment shader. It assumes that green is up. viewDir and geometricNormal need to be in the same space.
/// viewDir is the -(eye - vertexPos)
Vec3 perturbNormal(Vec3 tangentNormal, Vec3 viewDir, Vec2 uv, Vec3 geometricNormal)
{
	tangentNormal.y = -tangentNormal.y; // Green is up

	// Get edge vectors of the pixel triangle
	const Vec3 dp1 = ddx(viewDir);
	const Vec3 dp2 = ddy(viewDir);
	const Vec2 duv1 = ddx(uv);
	const Vec2 duv2 = ddy(uv);

	// Solve the linear system
	const Vec3 dp2perp = cross(dp2, geometricNormal);
	const Vec3 dp1perp = cross(geometricNormal, dp1);
	const Vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	const Vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// Construct a scale-invariant frame
	const F32 invmax = rsqrt(max(dot(T, T), dot(B, B)));

	Mat3 TBN;
	TBN.setColumns(T * invmax, B * invmax, geometricNormal);
	return normalize(mul(TBN, tangentNormal));
}

/// Project a sphere into NDC. Sphere in view space. The sphere should be in front of the near plane (-sphereCenter.z > sphereRadius + znear)
/// @param P00 projection matrix's [0,0]
/// @param P11 projection matrix's [1,1]
void projectSphereView(Vec3 sphereCenter, F32 sphereRadius, F32 P00, F32 P11, out Vec2 aabbMin, out Vec2 aabbMax)
{
	sphereCenter.z = abs(sphereCenter.z);

	const Vec3 cr = sphereCenter * sphereRadius;
	const F32 czr2 = sphereCenter.z * sphereCenter.z - sphereRadius * sphereRadius;

	const F32 vx = sqrt(sphereCenter.x * sphereCenter.x + czr2);
	const F32 minx = (vx * sphereCenter.x - cr.z) / (vx * sphereCenter.z + cr.x);
	const F32 maxx = (vx * sphereCenter.x + cr.z) / (vx * sphereCenter.z - cr.x);

	const F32 vy = sqrt(sphereCenter.y * sphereCenter.y + czr2);
	const F32 miny = (vy * sphereCenter.y - cr.z) / (vy * sphereCenter.z + cr.y);
	const F32 maxy = (vy * sphereCenter.y + cr.z) / (vy * sphereCenter.z - cr.y);

	aabbMin = Vec2(minx * P00, miny * P11);
	aabbMax = Vec2(maxx * P00, maxy * P11);
}

template<typename T>
T barycentricInterpolation(T a, T b, T c, Vec3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

void unflatten3dArrayIndex(const U32 sizeA, const U32 sizeB, const U32 sizeC, const U32 flatIdx, out U32 a, out U32 b, out U32 c)
{
	ANKI_ASSERT(flatIdx < (sizeA * sizeB * sizeC));
	a = (flatIdx / (sizeB * sizeC)) % sizeA;
	b = (flatIdx / sizeC) % sizeB;
	c = flatIdx % sizeC;
}

Bool dither2x2(Vec2 svPosition, F32 factor)
{
	const U32 ditherMatrix[4] = {0, 3, 2, 1};
	const F32 axisSize = 2.0;

	const U32 x = U32(fmod(svPosition.x, axisSize));
	const U32 y = U32(fmod(svPosition.y, axisSize));
	const U32 index = x + y * U32(axisSize);
	const F32 limit = (F32(ditherMatrix[index]) + 1.0) / (1.0 + axisSize * axisSize);
	return (factor < limit) ? true : false;
}

Bool dither4x4(Vec2 svPosition, F32 factor)
{
	const U32 ditherMatrix[16] = {0, 12, 3, 15, 8, 4, 11, 7, 2, 14, 1, 13, 10, 6, 9, 5};
	const F32 axisSize = 4.0;

	const U32 x = U32(fmod(svPosition.x, axisSize));
	const U32 y = U32(fmod(svPosition.y, axisSize));
	const U32 index = x + y * U32(axisSize);
	const F32 limit = (F32(ditherMatrix[index]) + 1.0) / (1.0 + axisSize * axisSize);
	return (factor < limit) ? true : false;
}

// Encode a normal to octahedron UV coordinates
Vec2 octahedronEncode(Vec3 n)
{
	n /= (abs(n.x) + abs(n.y) + abs(n.z));
	const Vec2 octWrap = (1.0 - abs(n.yx)) * select(n.xy >= 0.0, 1.0, -1.0);
	n.xy = select(n.z >= 0.0, n.xy, octWrap);
	n.xy = n.xy * 0.5 + 0.5;
	return n.xy;
}

// The reverse of octahedronEncode
// https://twitter.com/Stubbesaurus/status/937994790553227264
Vec3 octahedronDecode(Vec2 f)
{
	f = f * 2.0 - 1.0;
	Vec3 n = Vec3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
	const F32 t = saturate(-n.z);
	n.xy += select(n.xy >= 0.0, -t, t);
	return normalize(n);
}

/// Given the size of the octahedron texture and a texel that belongs to it, return the offsets relative to this texel that belong to the border.
/// The texSize is without border and the texCoord as well.
U32 octahedronBorder(IVec2 texSize, IVec2 texCoord, out IVec2 borderTexOffsets[3])
{
	U32 borderCount = 0;
	if(all(texCoord == 0))
	{
		borderTexOffsets[borderCount++] = texSize;
	}
	else if(texCoord.x == 0 && texCoord.y == texSize.y - 1)
	{
		borderTexOffsets[borderCount++] = IVec2(texSize.x, -texSize.y);
	}
	else if(all(texCoord == texSize - 1))
	{
		borderTexOffsets[borderCount++] = -texSize;
	}
	else if(texCoord.x == texSize.x - 1 && texCoord.y == 0)
	{
		borderTexOffsets[borderCount++] = IVec2(-texSize.x, texSize.y);
	}

	if(texCoord.y == 0)
	{
		borderTexOffsets[borderCount++] = IVec2((texSize.x - 1) - 2 * texCoord.x, -1);
	}
	else if(texCoord.y == texSize.y - 1)
	{
		borderTexOffsets[borderCount++] = IVec2((texSize.x - 1) - 2 * texCoord.x, 1);
	}

	if(texCoord.x == 0)
	{
		borderTexOffsets[borderCount++] = IVec2(-1, (texSize.y - 1) - 2 * texCoord.y);
	}
	else if(texCoord.x == texSize.x - 1)
	{
		borderTexOffsets[borderCount++] = IVec2(1, (texSize.y - 1) - 2 * texCoord.y);
	}

	return borderCount;
}

/// Manual texture sampling of a 3D texture.
template<typename T, U32 kComp>
vector<T, kComp> linearTextureSampling(Texture3D<Vec4> sam, Vec3 uv)
{
	Vec3 texSize;
	sam.GetDimensions(texSize.x, texSize.y, texSize.z);

	uv = frac(uv);
	uv = uv * texSize - 0.5;
	Vec3 iuv = floor(uv);
	Vec3 fuv = frac(uv);

	vector<T, kComp> o = T(0);

	for(U32 i = 0u; i < 8u; ++i)
	{
		const Vec3 xyz = Vec3(UVec3(i, i >> 1u, i >> 2u) & 1u);
		Vec3 coords = iuv + xyz;

		// Repeat
		coords = select(coords >= 0.0, coords, texSize + coords);
		coords = select(coords < texSize, coords, coords - texSize);

		const vector<T, kComp> s = sam[coords];

		const vector<T, 3> w3 = select(xyz == 0.0, T(1) - fuv, fuv);
		const T w = w3.x * w3.y * w3.z;

		o += s * w;
	}

	return o;
}

/// Generate a 4x MSAA pattern. Returns the numbers in
/// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels
/// Divide the result by 8.0 to normalize.
IVec2 generateMsaa4x(U32 sample)
{
	sample <<= 2u;
	IVec2 pattern = IVec2(41702, 60002);
	pattern >>= sample;
	pattern &= 0xF;
	pattern -= 8;

	return pattern;
}

/// Generate a 8x MSAA pattern. Returns the numbers in
/// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels
/// Divide the result by 8.0 to normalize.
IVec2 generateMsaa8x(U32 sample)
{
	sample <<= 2u;
	IVec2 pattern = IVec2(4212350329, 528300469);
	pattern >>= sample;
	pattern &= 0xF;
	pattern -= 8;

	return pattern;
}

/// Generate a 16x MSAA pattern. Returns the numbers in
/// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels
/// Divide the result by 8.0 to normalize.
IVec2 generateMsaa16x(U32 sample)
{
	const IVec2 packed[2] = {IVec2(0xBDA3C579, 0x3BD67A59), IVec2(0x1EF02486, 0xF48C21E)};
	const U32 bit = (sample % 8u) * 4u;

	IVec2 pattern = (sample < 8) ? packed[0] : packed[1];
	pattern >>= bit;
	pattern &= 0xF;
	pattern -= 8;

	return pattern;
}

/// Given some UVs (can be 2D or 3D) and the texture size, return some improved UVs that are used for linear filtering.
/// Code taken from https://www.shadertoy.com/view/XsfGDn
template<typename TUv>
TUv improvedLinearTextureFiltering(TUv uv, TUv texSize)
{
	uv = uv * texSize + 0.5;
	const TUv iuv = floor(uv);
	const TUv fuv = frac(uv);
	uv = iuv + fuv * fuv * (3.0 - 2.0 * fuv);
	uv = (uv - 0.5) / texSize;
	return uv;
}

template<typename T>
T computeLuminance(vector<T, 3> color)
{
	return max(dot(vector<T, 3>(0.30, 0.59, 0.11), color), getEpsilon<T>());
}

template<typename T>
vector<T, 3> rgb2ycbcr(vector<T, 3> rgb)
{
	const T y = computeLuminance(rgb);
	const T cb = (rgb.b - y) * 0.565;
	const T cr = (rgb.r - y) * 0.713;

	return vector<T, 3>(y, cb, cr);
}

// YCbCr to RGB
template<typename T>
vector<T, 3> ycbcr2rgb(vector<T, 3> yuv)
{
	return vector<T, 3>(yuv.x + 1.403 * yuv.z, yuv.x - 0.344 * yuv.y - 0.714 * yuv.z, yuv.x + 1.770 * yuv.y);
}

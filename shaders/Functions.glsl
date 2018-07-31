// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <shaders/Common.glsl>

#if defined(ANKI_FRAGMENT_SHADER)
Vec3 dither(in Vec3 col, in F32 C)
{
	Vec3 vDither = Vec3(dot(Vec2(171.0, 231.0), gl_FragCoord.xy));
	vDither.rgb = fract(vDither.rgb / Vec3(103.0, 71.0, 97.0));

	col = col * (255.0 / C) + vDither.rgb;
	col = floor(col) / 255.0;
	col *= C;

	return col;
}

F32 dither(in F32 col, in F32 C)
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
F32 linearizeDepth(in F32 depth, in F32 zNear, in F32 zFar)
{
	return zNear / ((zNear - zFar) + zFar / depth);
}

// This is the optimal linearizeDepth where a=(n-f)/n and b=f/n
F32 linearizeDepthOptimal(in F32 depth, in F32 a, in F32 b)
{
	return 1.0 / (a + b / depth);
}

// This is the optimal linearizeDepth where a=(n-f)/n and b=f/n
Vec4 linearizeDepthOptimal(in Vec4 depths, in F32 a, in F32 b)
{
	return 1.0 / (a + b / depths);
}

// Project a vector by knowing only the non zero values of a perspective matrix
Vec4 projectPerspective(in Vec4 vec, in F32 m00, in F32 m11, in F32 m22, in F32 m23)
{
	Vec4 o;
	o.x = vec.x * m00;
	o.y = vec.y * m11;
	o.z = vec.z * m22 + vec.w * m23;
	o.w = -vec.z;
	return o;
}

// Stolen from shadertoy.com/view/4tyGDD
Vec4 textureCatmullRom4Samples(sampler2D tex, Vec2 uv, Vec2 texSize)
{
	Vec2 halff = 2.0 * fract(0.5 * uv * texSize - 0.25) - 1.0;
	Vec2 f = fract(halff);
	Vec2 sum0 = (2.0 * f - 3.5) * f + 0.5;
	Vec2 sum1 = (2.0 * f - 2.5) * f - 0.5;
	Vec4 w = Vec4(f * sum0 + 1.0, f * sum1);
	Vec4 pos = Vec4((((-2.0 * f + 3.0) * f + 0.5) * f - 1.5) * f / (w.xy * texSize) + uv,
		(((-2.0 * f + 5.0) * f - 2.5) * f - 0.5) / (sum1 * texSize) + uv);
	w.xz *= halff.x * halff.y > 0.0 ? 1.0 : -1.0;

	return (texture(tex, pos.xy) * w.x + texture(tex, pos.zy) * w.z) * w.y
		   + (texture(tex, pos.xw) * w.x + texture(tex, pos.zw) * w.z) * w.w;
}

F32 rand(Vec2 n)
{
	return 0.5 + 0.5 * fract(sin(dot(n, Vec2(12.9898, 78.233))) * 43758.5453);
}

Vec4 nearestDepthUpscale(
	Vec2 uv, sampler2D depthFull, sampler2D depthHalf, sampler2D colorTex, Vec2 linearDepthCf, F32 depthThreshold)
{
	F32 fullDepth = texture(depthFull, uv).r;
	fullDepth = linearizeDepthOptimal(fullDepth, linearDepthCf.x, linearDepthCf.y);

	Vec4 halfDepths = textureGather(depthHalf, uv, 0);
	halfDepths = linearizeDepthOptimal(halfDepths, linearDepthCf.x, linearDepthCf.y);

	Vec4 diffs = abs(Vec4(fullDepth) - halfDepths);
	Vec4 color;

	if(all(lessThan(diffs, Vec4(depthThreshold))))
	{
		// No major discontinuites, sample with bilinear
		color = texture(colorTex, uv);
	}
	else
	{
		// Some discontinuites, need to use the newUv
		Vec4 r = textureGather(colorTex, uv, 0);
		Vec4 g = textureGather(colorTex, uv, 1);
		Vec4 b = textureGather(colorTex, uv, 2);
		Vec4 a = textureGather(colorTex, uv, 3);

		F32 minDiff = diffs.x;
		U32 comp = 0;

		if(diffs.y < minDiff)
		{
			comp = 1;
			minDiff = diffs.y;
		}

		if(diffs.z < minDiff)
		{
			comp = 2;
			minDiff = diffs.z;
		}

		if(diffs.w < minDiff)
		{
			comp = 3;
		}

		color = Vec4(r[comp], g[comp], b[comp], a[comp]);
	}

	return color;
}

F32 _calcDepthWeight(sampler2D depthLow, Vec2 uv, F32 ref, Vec2 linearDepthCf)
{
	F32 d = texture(depthLow, uv).r;
	F32 linearD = linearizeDepthOptimal(d, linearDepthCf.x, linearDepthCf.y);
	return 1.0 / (EPSILON + abs(ref - linearD));
}

Vec4 _sampleAndWeight(sampler2D depthLow,
	sampler2D colorLow,
	Vec2 lowInvSize,
	Vec2 uv,
	Vec2 offset,
	F32 ref,
	F32 weight,
	Vec2 linearDepthCf,
	inout F32 normalize)
{
	uv += offset * lowInvSize;
	F32 dw = _calcDepthWeight(depthLow, uv, ref, linearDepthCf);
	Vec4 v = texture(colorLow, uv);
	normalize += weight * dw;
	return v * dw * weight;
}

Vec4 bilateralUpsample(
	sampler2D depthHigh, sampler2D depthLow, sampler2D colorLow, Vec2 lowInvSize, Vec2 uv, Vec2 linearDepthCf)
{
	const Vec3 WEIGHTS = Vec3(0.25, 0.125, 0.0625);
	F32 depthRef = linearizeDepthOptimal(texture(depthHigh, uv).r, linearDepthCf.x, linearDepthCf.y);
	F32 normalize = 0.0;

	Vec4 sum = _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, Vec2(0.0, 0.0), depthRef, WEIGHTS.x, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, Vec2(-1.0, 0.0), depthRef, WEIGHTS.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, Vec2(0.0, -1.0), depthRef, WEIGHTS.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, Vec2(1.0, 0.0), depthRef, WEIGHTS.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, Vec2(0.0, 1.0), depthRef, WEIGHTS.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, Vec2(1.0, 1.0), depthRef, WEIGHTS.z, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, Vec2(1.0, -1.0), depthRef, WEIGHTS.z, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, Vec2(-1.0, 1.0), depthRef, WEIGHTS.z, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, Vec2(-1.0, -1.0), depthRef, WEIGHTS.z, linearDepthCf, normalize);

	return sum / normalize;
}

Vec3 getCubemapDirection(Vec2 norm, U32 faceIdx)
{
	Vec3 zDir = Vec3((faceIdx <= 1u) ? 1 : 0, (faceIdx & 2u) >> 1u, (faceIdx & 4u) >> 2u);
	zDir *= (((faceIdx & 1u) == 1u) ? -1.0 : 1.0);
	Vec3 yDir = (faceIdx == 2u) ? Vec3(0.0, 0.0, 1.0) : (faceIdx == 3u) ? Vec3(0.0, 0.0, -1.0) : Vec3(0.0, -1.0, 0.0);
	Vec3 xDir = cross(zDir, yDir);
	return normalize(norm.x * xDir + norm.y * yDir + zDir);
}

// Convert 3D cubemap coordinates to 2D plus face index. v doesn't need to be normalized.
Vec2 convertCubeUvs(Vec3 v, out F32 faceIndex)
{
	Vec3 absV = abs(v);
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
Vec2 convertCubeUvsu(Vec3 v, out U32 faceIndex)
{
	Vec3 absV = abs(v);
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

Vec3 grayScale(Vec3 col)
{
	F32 grey = (col.r + col.g + col.b) * (1.0 / 3.0);
	return Vec3(grey);
}

Vec3 saturateColor(Vec3 col, F32 factor)
{
	const Vec3 LUM_COEFF = Vec3(0.2125, 0.7154, 0.0721);
	Vec3 intensity = Vec3(dot(col, LUM_COEFF));
	return mix(intensity, col, factor);
}

Vec3 gammaCorrection(Vec3 gamma, Vec3 col)
{
	return pow(col, 1.0 / gamma);
}

// Can use 0.15 for sharpenFactor
Vec3 readSharpen(sampler2D tex, Vec2 uv, F32 sharpenFactor, Bool detailed)
{
	Vec3 col = textureLod(tex, uv, 0.0).rgb;

	Vec3 col2 = textureLodOffset(tex, uv, 0.0, IVec2(1, 1)).rgb;
	col2 += textureLodOffset(tex, uv, 0.0, IVec2(-1, -1)).rgb;
	col2 += textureLodOffset(tex, uv, 0.0, IVec2(1, -1)).rgb;
	col2 += textureLodOffset(tex, uv, 0.0, IVec2(-1, 1)).rgb;

	F32 f = 4.0;
	if(detailed)
	{
		col2 += textureLodOffset(tex, uv, 0.0, IVec2(0, 1)).rgb;
		col2 += textureLodOffset(tex, uv, 0.0, IVec2(1, 0)).rgb;
		col2 += textureLodOffset(tex, uv, 0.0, IVec2(-1, 0)).rgb;
		col2 += textureLodOffset(tex, uv, 0.0, IVec2(0, -1)).rgb;

		f = 8.0;
	}

	col = col * (f * sharpenFactor + 1.0) - sharpenFactor * col2;
	return max(Vec3(0.0), col);
}

Vec3 readErosion(sampler2D tex, Vec2 uv)
{
	Vec3 minValue = textureLod(tex, uv, 0.0).rgb;

#define ANKI_EROSION(x, y) \
	col2 = textureLodOffset(tex, uv, 0.0, IVec2(x, y)).rgb; \
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
Vec3 heatmap(F32 factor)
{
	F32 intPart;
	F32 fractional = modf(factor * 4.0, intPart);

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

Bool incorrectColor(Vec3 c)
{
	return isnan(c.x) || isnan(c.y) || isnan(c.z) || isinf(c.x) || isinf(c.y) || isinf(c.z);
}

F32 areaElement(F32 x, F32 y)
{
	return atan(x * y, sqrt(x * x + y * y + 1.0));
}

// Compute the solid angle of a cube. Solid angle is the area of a sphere when projected into a cubemap. It's also the
// delta omega (dω) in the irradiance integral and other integrals that operate in a sphere.
// http://www.rorydriscoll.com/2012/01/15/cubemap-texel-solid-angle/
F32 cubeCoordSolidAngle(Vec2 norm, F32 cubeFaceSize)
{
	Vec2 invSize = Vec2(1.0 / cubeFaceSize);
	Vec2 v0 = norm - invSize;
	Vec2 v1 = norm + invSize;
	return areaElement(v0.x, v0.y) - areaElement(v0.x, v1.y) - areaElement(v1.x, v0.y) + areaElement(v1.x, v1.y);
}

// Intersect a ray against an AABB. The ray is inside the AABB. The function returns the distance 'a' where the
// intersection point is rayOrigin + rayDir * a
// https://community.arm.com/graphics/b/blog/posts/reflections-based-on-local-cubemaps-in-unity
F32 rayAabbIntersectionInside(Vec3 rayOrigin, Vec3 rayDir, Vec3 aabbMin, Vec3 aabbMax)
{
	Vec3 intersectMaxPointPlanes = (aabbMax - rayOrigin) / rayDir;
	Vec3 intersectMinPointPlanes = (aabbMin - rayOrigin) / rayDir;
	Vec3 largestParams = max(intersectMaxPointPlanes, intersectMinPointPlanes);
	F32 distToIntersect = min(min(largestParams.x, largestParams.y), largestParams.z);
	return distToIntersect;
}

// A convenience macro to skip out of bounds invocations on post-process compute shaders.
#define SKIP_OUT_OF_BOUNDS_INVOCATIONS() \
	if((FB_SIZE.x % WORKGROUP_SIZE.x) != 0u || (FB_SIZE.y % WORKGROUP_SIZE.y) != 0u) \
	{ \
		if(gl_GlobalInvocationID.x >= FB_SIZE.x || gl_GlobalInvocationID.y >= FB_SIZE.y) \
		{ \
			return; \
		} \
	}

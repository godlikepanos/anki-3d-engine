// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shaders/Common.glsl>

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

F32 _calcDepthWeight(texture2D depthLow, sampler nearestAnyClamp, Vec2 uv, F32 ref, Vec2 linearDepthCf)
{
	const F32 d = textureLod(depthLow, nearestAnyClamp, uv, 0.0).r;
	const F32 linearD = linearizeDepthOptimal(d, linearDepthCf.x, linearDepthCf.y);
	return 1.0 / (EPSILON + abs(ref - linearD));
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
	const Vec3 WEIGHTS = Vec3(0.25, 0.125, 0.0625);
	const F32 depthRef =
		linearizeDepthOptimal(textureLod(depthHigh, nearestAnyClamp, uv, 0.0).r, linearDepthCf.x, linearDepthCf.y);
	F32 normalize = 0.0;

	Vec4 sum = _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(0.0, 0.0),
								depthRef, WEIGHTS.x, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(-1.0, 0.0),
							depthRef, WEIGHTS.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(0.0, -1.0),
							depthRef, WEIGHTS.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(1.0, 0.0),
							depthRef, WEIGHTS.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(0.0, 1.0),
							depthRef, WEIGHTS.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(1.0, 1.0),
							depthRef, WEIGHTS.z, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(1.0, -1.0),
							depthRef, WEIGHTS.z, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(-1.0, 1.0),
							depthRef, WEIGHTS.z, linearDepthCf, normalize);
	sum += _sampleAndWeight(depthLow, colorLow, linearAnyClamp, nearestAnyClamp, lowInvSize, uv, Vec2(-1.0, -1.0),
							depthRef, WEIGHTS.z, linearDepthCf, normalize);

	return sum / normalize;
}

Vec3 getCubemapDirection(const Vec2 norm, const U32 faceIdx)
{
	Vec3 zDir = Vec3((faceIdx <= 1u) ? 1 : 0, (faceIdx & 2u) >> 1u, (faceIdx & 4u) >> 2u);
	zDir *= (((faceIdx & 1u) == 1u) ? -1.0 : 1.0);
	const Vec3 yDir =
		(faceIdx == 2u) ? Vec3(0.0, 0.0, 1.0) : (faceIdx == 3u) ? Vec3(0.0, 0.0, -1.0) : Vec3(0.0, -1.0, 0.0);
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
	const Vec3 LUM_COEFF = Vec3(0.2125, 0.7154, 0.0721);
	const Vec3 intensity = Vec3(dot(col, LUM_COEFF));
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

// Intersect a ray against an AABB. The ray is inside the AABB. The function returns the distance 'a' where the
// intersection point is rayOrigin + rayDir * a
// https://community.arm.com/graphics/b/blog/posts/reflections-based-on-local-cubemaps-in-unity
F32 rayAabbIntersectionInside(Vec3 rayOrigin, Vec3 rayDir, Vec3 aabbMin, Vec3 aabbMax)
{
	const Vec3 intersectMaxPointPlanes = (aabbMax - rayOrigin) / rayDir;
	const Vec3 intersectMinPointPlanes = (aabbMin - rayOrigin) / rayDir;
	const Vec3 largestParams = max(intersectMaxPointPlanes, intersectMinPointPlanes);
	const F32 distToIntersect = min(min(largestParams.x, largestParams.y), largestParams.z);
	return distToIntersect;
}

// Return true if to AABBs overlap
Bool aabbsOverlap(const Vec3 aMin, const Vec3 aMax, const Vec3 bMin, const Vec3 bMax)
{
	return all(lessThan(aMin, bMax)) && all(lessThan(bMin, aMax));
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

// Create a matrix from some direction.
Mat3 rotationFromDirection(Vec3 zAxis)
{
#if 0
	const Vec3 z = zAxis;
	const Bool alignsWithXBasis = abs(z.x - 1.0) <= EPSILON; // aka z == Vec3(1.0, 0.0, 0.0)
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

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
Bool rayTriangleIntersect(Vec3 orig, Vec3 dir, Vec3 v0, Vec3 v1, Vec3 v2, out F32 t, out F32 u, out F32 v)
{
	const Vec3 v0v1 = v1 - v0;
	const Vec3 v0v2 = v2 - v0;
	const Vec3 pvec = cross(dir, v0v2);
	const F32 det = dot(v0v1, pvec);

	if(det < EPSILON)
	{
		return false;
	}

	const F32 invDet = 1.0 / det;

	const Vec3 tvec = orig - v0;
	u = dot(tvec, pvec) * invDet;
	if(u < 0.0 || u > 1.0)
	{
		return false;
	}

	const Vec3 qvec = cross(tvec, v0v1);
	v = dot(dir, qvec) * invDet;
	if(v < 0.0 || u + v > 1.0)
	{
		return false;
	}

	t = dot(v0v2, qvec) * invDet;
	return true;
}

// Given some info of the current fragment unproject it to the previous frame. Will return UV coordinates
Vec2 reprojectHistoryBuffer(Vec2 cnrtPixelUv, F32 crntPixelDepth, Mat4 prevViewProjMatMulInvViewProjMat, Vec2 velocity)
{
	Vec2 oldUv;
	if(velocity.x != -1.0)
	{
		oldUv = cnrtPixelUv + velocity;
	}
	else
	{
		const Vec4 v4 = prevViewProjMatMulInvViewProjMat * Vec4(UV_TO_NDC(cnrtPixelUv), crntPixelDepth, 1.0);
		oldUv = NDC_TO_UV(v4.xy / v4.w);
	}

	return oldUv;
}
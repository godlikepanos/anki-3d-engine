// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains the complete code for a SSAO pass. It requires some variables to be defined before this file is included.

#pragma anki mutator USE_NORMAL 0 1
#pragma anki mutator SOFT_BLUR 0 1

ANKI_SPECIALIZATION_CONSTANT_U32(NOISE_MAP_SIZE, 0, 1);
ANKI_SPECIALIZATION_CONSTANT_UVEC2(FB_SIZE, 1, UVec2(1));
ANKI_SPECIALIZATION_CONSTANT_F32(RADIUS, 3, 0.0);
ANKI_SPECIALIZATION_CONSTANT_F32(BIAS, 4, 0.0);
ANKI_SPECIALIZATION_CONSTANT_F32(STRENGTH, 5, 0.0);
ANKI_SPECIALIZATION_CONSTANT_U32(SAMPLE_COUNT, 6, 1);

#pragma once

#include <anki/shaders/Common.glsl>
#include <anki/shaders/Pack.glsl>
#include <anki/shaders/Functions.glsl>
#define KERNEL_SIZE 3
#include <anki/shaders/GaussianBlurCommon.glsl>

#if defined(ANKI_FRAGMENT_SHADER)
#	define USE_COMPUTE 0
#else
#	define USE_COMPUTE 1
const UVec2 WORKGROUP_SIZE = UVec2(16, 16);
#endif

// Do a compute soft blur
#define DO_SOFT_BLUR (USE_COMPUTE && SOFT_BLUR)

#if !USE_COMPUTE
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out F32 out_color;
#else
layout(local_size_x = WORKGROUP_SIZE.x, local_size_y = WORKGROUP_SIZE.y, local_size_z = 1) in;

layout(set = 0, binding = 5) writeonly uniform image2D out_img;
#endif

layout(push_constant, std140, row_major) uniform _pc
{
	Vec4 u_unprojectionParams;
	Vec4 u_projectionMat;
	Mat3 u_viewRotMat;
};

layout(set = 0, binding = 0) uniform sampler u_linearAnyClampSampler;
layout(set = 0, binding = 1) uniform sampler u_trilinearRepeatSampler;

layout(set = 0, binding = 2) uniform texture2D u_depthRt;
layout(set = 0, binding = 3) uniform texture2DArray u_noiseMap;
#if USE_NORMAL
layout(set = 0, binding = 4) uniform texture2D u_msRt;
#endif

// To compute the normals we need some extra work on compute
#define COMPLEX_NORMALS (!USE_NORMAL && USE_COMPUTE)
#if DO_SOFT_BLUR
shared Vec3 s_scratch[WORKGROUP_SIZE.y][WORKGROUP_SIZE.x];
#endif

#if USE_NORMAL
// Get normal
Vec3 readNormal(Vec2 uv)
{
	Vec3 normal = readNormalFromGBuffer(u_msRt, u_linearAnyClampSampler, uv);
	normal = u_viewRotMat * normal;
	return normal;
}
#endif

// Read the noise tex
Vec3 readRandom(Vec2 uv, F32 layer)
{
	const Vec2 tmp = Vec2(F32(FB_SIZE.x) / F32(NOISE_MAP_SIZE), F32(FB_SIZE.y) / F32(NOISE_MAP_SIZE));
	const Vec3 r = textureLod(u_noiseMap, u_trilinearRepeatSampler, Vec3(tmp * uv, layer), 0.0).rgb;
	return r;
}

Vec4 project(Vec4 point)
{
	return projectPerspective(point, u_projectionMat.x, u_projectionMat.y, u_projectionMat.z, u_projectionMat.w);
}

Vec3 unproject(Vec2 ndc, F32 depth)
{
	const F32 z = u_unprojectionParams.z / (u_unprojectionParams.w + depth);
	const Vec2 xy = ndc * u_unprojectionParams.xy * z;
	return Vec3(xy, z);
}

F32 smallerDelta(F32 left, F32 mid, F32 right)
{
	const F32 a = mid - left;
	const F32 b = right - mid;

	return (abs(a) < abs(b)) ? a : b;
}

// Compute the normal depending on some defines
Vec3 computeNormal(Vec2 uv, Vec3 origin, F32 depth)
{
#if USE_NORMAL
	const Vec3 normal = readNormal(uv);
#elif !COMPLEX_NORMALS
	const Vec3 normal = normalize(cross(dFdx(origin), dFdy(origin)));
#else
	const F32 depthLeft = textureLodOffset(sampler2D(u_depthRt, u_linearAnyClampSampler), uv, 0.0, ivec2(-2, 0)).r;
	const F32 depthRight = textureLodOffset(sampler2D(u_depthRt, u_linearAnyClampSampler), uv, 0.0, ivec2(2, 0)).r;
	const F32 depthTop = textureLodOffset(sampler2D(u_depthRt, u_linearAnyClampSampler), uv, 0.0, ivec2(0, 2)).r;
	const F32 depthBottom = textureLodOffset(sampler2D(u_depthRt, u_linearAnyClampSampler), uv, 0.0, ivec2(0, -2)).r;

	const F32 ddx = smallerDelta(depthLeft, depth, depthRight);
	const F32 ddy = smallerDelta(depthBottom, depth, depthTop);

	const Vec2 ndc = UV_TO_NDC(uv);
	const Vec2 TEXEL_SIZE = 1.0 / Vec2(FB_SIZE);
	const Vec2 NDC_TEXEL_SIZE = 2.0 * TEXEL_SIZE;
	const Vec3 right = unproject(ndc + Vec2(NDC_TEXEL_SIZE.x, 0.0), depth + ddx);
	const Vec3 top = unproject(ndc + Vec2(0.0, NDC_TEXEL_SIZE.y), depth + ddy);

	Vec3 normal = cross(origin - top, right - origin);
	normal = normalize(normal);
#endif

	return normal;
}

void main(void)
{
#if USE_COMPUTE
	if(gl_GlobalInvocationID.x >= FB_SIZE.x || gl_GlobalInvocationID.y >= FB_SIZE.y)
	{
#	if DO_SOFT_BLUR
		// Store something anyway because alive threads might read it when SOFT_BLUR is enabled
		s_scratch[gl_LocalInvocationID.y][gl_LocalInvocationID.x] = Vec3(1.0);
#	endif

		// Skip if it's out of bounds
		return;
	}

	const Vec2 uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5) / Vec2(FB_SIZE);
#else
	const Vec2 uv = in_uv;
#endif

	const Vec2 ndc = UV_TO_NDC(uv);

	// Compute origin
	const F32 depth = textureLod(u_depthRt, u_linearAnyClampSampler, uv, 0.0).r;
	const Vec3 origin = unproject(ndc, depth);

	// Get normal
	const Vec3 normal = computeNormal(uv, origin, depth);

	// Find the projected radius
	const Vec3 sphereLimit = origin + Vec3(RADIUS, 0.0, 0.0);
	const Vec4 projSphereLimit = project(Vec4(sphereLimit, 1.0));
	const Vec2 projSphereLimit2 = projSphereLimit.xy / projSphereLimit.w;
	const F32 projRadius = length(projSphereLimit2 - ndc);

	// Loop to compute
	const F32 randFactor = readRandom(uv, 0.0).r;
	F32 ssao = 0.0;
	const F32 SAMPLE_COUNTF = F32(SAMPLE_COUNT);
	ANKI_UNROLL for(U32 i = 0; i < SAMPLE_COUNT; ++i)
	{
		// Compute disk. Basically calculate a point in a spiral. See how it looks here
		// https://shadertoy.com/view/Md3fDr
		const F32 fi = F32(i);
		const F32 TURNS = F32(SAMPLE_COUNT) / 2.0; // Calculate the number of the spiral turns
		const F32 ANG = (PI * 2.0 * TURNS) / (SAMPLE_COUNTF - 1.0); // The angle distance between samples
		F32 ang = ANG * fi;
		ang += randFactor * PI; // Turn the angle a bit

		F32 radius = (1.0 / SAMPLE_COUNTF) * (fi + 1.0);
		radius = sqrt(radius); // Move the points a bit away from the center of the spiral

		const Vec2 point = Vec2(cos(ang), sin(ang)) * radius; // In NDC

		const Vec2 finalDiskPoint = ndc + point * projRadius;

		// Compute factor
		const Vec3 s =
			unproject(finalDiskPoint, textureLod(u_depthRt, u_linearAnyClampSampler, NDC_TO_UV(finalDiskPoint), 0.0).r);
		const Vec3 u = s - origin;
		ssao += max(dot(normal, u) + BIAS, EPSILON) / max(dot(u, u), EPSILON);
	}

	ssao *= (1.0 / F32(SAMPLE_COUNT));
	ssao = 1.0 - ssao * STRENGTH;

	// Maybe do soft blur
#if DO_SOFT_BLUR
	// Everyone stores its result
	s_scratch[gl_LocalInvocationID.y][gl_LocalInvocationID.x].x = ssao;

	// Do some pre-work to find out the neighbours
	const U32 left = (gl_LocalInvocationID.x != 0u) ? (gl_LocalInvocationID.x - 1u) : 0u;
	const U32 right =
		(gl_LocalInvocationID.x != WORKGROUP_SIZE.x - 1u) ? (gl_LocalInvocationID.x + 1u) : (WORKGROUP_SIZE.x - 1u);
	const U32 bottom = (gl_LocalInvocationID.y != 0u) ? (gl_LocalInvocationID.y - 1u) : 0u;
	const U32 top =
		(gl_LocalInvocationID.y != WORKGROUP_SIZE.y - 1u) ? (gl_LocalInvocationID.y + 1u) : (WORKGROUP_SIZE.y - 1u);

	// Wait for all threads
	memoryBarrierShared();
	barrier();

	// Sample neighbours
	ssao += s_scratch[gl_LocalInvocationID.y][left].x;
	ssao += s_scratch[gl_LocalInvocationID.y][right].x;
	ssao += s_scratch[bottom][gl_LocalInvocationID.x].x;
	ssao += s_scratch[top][gl_LocalInvocationID.x].x;

	ssao += s_scratch[bottom][left].x;
	ssao += s_scratch[bottom][right].x;
	ssao += s_scratch[top][left].x;
	ssao += s_scratch[top][right].x;

	ssao *= (1.0 / 9.0);
#endif

	// Store the result
#if USE_COMPUTE
	imageStore(out_img, IVec2(gl_GlobalInvocationID.xy), Vec4(ssao));
#else
	out_color = ssao;
#endif
}

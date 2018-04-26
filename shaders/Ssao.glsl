// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains the complete code for a SSAO pass. It requires some variables to be defined before this file is included.

#ifndef ANKI_SHADERS_SSAO_GLSL
#define ANKI_SHADERS_SSAO_GLSL

#include "shaders/Common.glsl"
#include "shaders/Pack.glsl"
#include "shaders/Functions.glsl"
#define KERNEL_SIZE 3
#include "shaders/GaussianBlurCommon.glsl"

#if defined(ANKI_FRAGMENT_SHADER)
#	define USE_COMPUTE 0
#else
#	define USE_COMPUTE 1
#endif

// Do a compute soft blur
#define DO_SOFT_BLUR (USE_COMPUTE && SOFT_BLUR)

#if !USE_COMPUTE
layout(location = 0) in vec2 in_uv;
layout(location = 0) out float out_color;
#else
layout(local_size_x = WORKGROUP_SIZE.x, local_size_y = WORKGROUP_SIZE.y, local_size_z = 1) in;

layout(ANKI_IMAGE_BINDING(0, 0)) writeonly uniform image2D out_img;
#endif

layout(ANKI_UBO_BINDING(0, 0), std140, row_major) uniform _blk
{
	vec4 u_unprojectionParams;
	vec4 u_projectionMat;
	mat3 u_viewRotMat;
};

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_mMsDepthRt;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2DArray u_noiseMap;
#if USE_NORMAL
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2D u_msRt;
#endif

// To compute the normals we need some extra work on compute
#define COMPLEX_NORMALS (!USE_NORMAL && USE_COMPUTE)
#if COMPLEX_NORMALS || DO_SOFT_BLUR
shared vec3 s_scratch[WORKGROUP_SIZE.y][WORKGROUP_SIZE.x];
#endif

#if USE_NORMAL
// Get normal
vec3 readNormal(in vec2 uv)
{
	vec3 normal;
	readNormalFromGBuffer(u_msRt, uv, normal);
	normal = u_viewRotMat * normal;
	return normal;
}
#endif

// Read the noise tex
vec3 readRandom(vec2 uv, float layer)
{
	const vec2 tmp = vec2(float(FB_SIZE.x) / float(NOISE_MAP_SIZE), float(FB_SIZE.y) / float(NOISE_MAP_SIZE));
	vec3 r = texture(u_noiseMap, vec3(tmp * uv, layer)).rgb;
	return r;
}

// Returns the Z of the position in view space
float readZ(in vec2 uv)
{
	float depth = textureLod(u_mMsDepthRt, uv, 0.0).r;
	float z = u_unprojectionParams.z / (u_unprojectionParams.w + depth);
	return z;
}

// Read position in view space
vec3 readPosition(in vec2 uv)
{
	vec3 fragPosVspace;
	fragPosVspace.z = readZ(uv);
	fragPosVspace.xy = UV_TO_NDC(uv) * u_unprojectionParams.xy * fragPosVspace.z;

	return fragPosVspace;
}

vec4 project(vec4 point)
{
	return projectPerspective(point, u_projectionMat.x, u_projectionMat.y, u_projectionMat.z, u_projectionMat.w);
}

// Compute the normal depending on some defines
vec3 computeNormal(vec2 uv, vec3 origin)
{
#if USE_NORMAL
	vec3 normal = readNormal(uv);
#elif !COMPLEX_NORMALS
	vec3 normal = normalize(cross(dFdx(origin), dFdy(origin)));
#else
	// Every thread stores its position
	s_scratch[gl_LocalInvocationID.y][gl_LocalInvocationID.x] = origin;

	memoryBarrierShared();
	barrier();

	// Have one thread every quad to compute the normal
	if(((gl_LocalInvocationID.x & 1u) + (gl_LocalInvocationID.y & 1u)) == 0u)
	{
		// It's the bottom left pixel of the quad

		vec3 center, right, top;
		center = origin;
		right = s_scratch[gl_LocalInvocationID.y][gl_LocalInvocationID.x + 1u];
		top = s_scratch[gl_LocalInvocationID.y + 1u][gl_LocalInvocationID.x];

		vec3 normal = normalize(cross(right - center, top - center));

		// Broadcast the normal
		s_scratch[gl_LocalInvocationID.y + 0u][gl_LocalInvocationID.x + 0u] = normal;
		s_scratch[gl_LocalInvocationID.y + 0u][gl_LocalInvocationID.x + 1u] = normal;
		s_scratch[gl_LocalInvocationID.y + 1u][gl_LocalInvocationID.x + 0u] = normal;
		s_scratch[gl_LocalInvocationID.y + 1u][gl_LocalInvocationID.x + 1u] = normal;
	}

	memoryBarrierShared();
	barrier();

	vec3 normal = s_scratch[gl_LocalInvocationID.y][gl_LocalInvocationID.x];
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
		s_scratch[gl_LocalInvocationID.y][gl_LocalInvocationID.x] = vec3(1.0);
#	endif

		// Skip if it's out of bounds
		return;
	}

	vec2 uv = (vec2(gl_GlobalInvocationID.xy) + 0.5) / vec2(FB_SIZE);
#else
	vec2 uv = in_uv;
#endif

	vec2 ndc = UV_TO_NDC(uv);

	// Compute origin
	vec3 origin = readPosition(uv);

	// Get normal
	vec3 normal = computeNormal(uv, origin);

	// Find the projected radius
	vec3 sphereLimit = origin + vec3(RADIUS, 0.0, 0.0);
	vec4 projSphereLimit = project(vec4(sphereLimit, 1.0));
	vec2 projSphereLimit2 = projSphereLimit.xy / projSphereLimit.w;
	float projRadius = length(projSphereLimit2 - ndc);

	// Loop to compute
	float randFactor = readRandom(uv, 0.0).r;
	float ssao = 0.0;
	const float SAMPLE_COUNTF = float(SAMPLE_COUNT);
	ANKI_UNROLL for(uint i = 0; i < SAMPLE_COUNT; ++i)
	{
		// Compute disk. Basically calculate a point in a spiral. See how it looks here
		// https://shadertoy.com/view/Md3fDr
		float fi = float(i);
		const float TURNS = float(SAMPLE_COUNT) / 2.0; // Calculate the number of the spiral turns
		const float ANG = (PI * 2.0 * TURNS) / (SAMPLE_COUNTF - 1.0); // The angle distance between samples
		float ang = ANG * fi;
		ang += randFactor * PI; // Turn the angle a bit

		float radius = (1.0 / SAMPLE_COUNTF) * (fi + 1.0);
		radius = sqrt(radius); // Move the points a bit away from the center of the spiral

		vec2 point = vec2(cos(ang), sin(ang)) * radius; // In NDC

		vec2 finalDiskPoint = ndc + point * projRadius;

		// Compute factor
		vec3 s = readPosition(NDC_TO_UV(finalDiskPoint));
		vec3 u = s - origin;
		ssao += max(dot(normal, u) + BIAS, EPSILON) / max(dot(u, u), EPSILON);
	}

	ssao *= (1.0 / float(SAMPLE_COUNT));
	ssao = 1.0 - ssao * STRENGTH;

	// Maybe do soft blur
#if DO_SOFT_BLUR
	// Everyone stores its result
	s_scratch[gl_LocalInvocationID.y][gl_LocalInvocationID.x].x = ssao;

	// Do some pre-work to find out the neighbours
	uint left = (gl_LocalInvocationID.x != 0u) ? (gl_LocalInvocationID.x - 1u) : 0u;
	uint right =
		(gl_LocalInvocationID.x != WORKGROUP_SIZE.x - 1u) ? (gl_LocalInvocationID.x + 1u) : (WORKGROUP_SIZE.x - 1u);
	uint bottom = (gl_LocalInvocationID.y != 0u) ? (gl_LocalInvocationID.y - 1u) : 0u;
	uint top =
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
	imageStore(out_img, ivec2(gl_GlobalInvocationID.xy), vec4(ssao));
#else
	out_color = ssao;
#endif
}

#endif

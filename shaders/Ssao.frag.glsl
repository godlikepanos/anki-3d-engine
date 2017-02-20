// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// SSAO fragment shader
#include "shaders/Common.glsl"
#include "shaders/Pack.glsl"
#include "shaders/Functions.glsl"

const vec2 KERNEL[SAMPLES] = KERNEL_ARRAY; // This will be appended in C++
const float BIAS = 0.2;
const float STRENGTH = 1.1;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out float out_color;

layout(ANKI_UBO_BINDING(0, 0), std140, row_major) uniform _blk
{
	vec4 u_unprojectionParams;
	vec4 u_projectionMat;
};

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_mMsDepthRt;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_msRt;
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2DArray u_noiseMap;

// Get normal
vec3 readNormal(in vec2 uv)
{
	vec3 normal;
	readNormalFromGBuffer(u_msRt, uv, normal);
	return normal;
}

// Read the noise tex
float readRandom(in vec2 uv)
{
	const vec2 tmp = vec2(float(WIDTH) / float(NOISE_MAP_SIZE), float(HEIGHT) / float(NOISE_MAP_SIZE));
	return texture(u_noiseMap, vec3(tmp * uv, 0.0)).r;
}

// Returns the Z of the position in view space
float readZ(in vec2 uv)
{
	float depth = texture(u_mMsDepthRt, uv).r;
	float z = u_unprojectionParams.z / (u_unprojectionParams.w + depth);
	return z;
}

// Read position in view space
vec3 readPosition(in vec2 uv)
{
	vec3 fragPosVspace;
	fragPosVspace.z = readZ(uv);

	fragPosVspace.xy = (2.0 * uv - 1.0) * u_unprojectionParams.xy * fragPosVspace.z;

	return fragPosVspace;
}

vec4 project(vec4 point)
{
	return projectPerspective(point, u_projectionMat.x, u_projectionMat.y, u_projectionMat.z, u_projectionMat.w);
}

void main(void)
{
	vec2 ndc = in_uv * 2.0 - 1.0;
	vec3 origin = readPosition(in_uv);
	vec3 normal = readNormal(in_uv);

	float randFactor = readRandom(in_uv);
	float randAng = randFactor * PI * 2.0;
	float cosAng = cos(randAng);
	float sinAng = sin(randAng);

	// Find the projected radius
	vec3 sphereLimit = origin + vec3(RADIUS, 0.0, 0.0);
	vec4 projSphereLimit = project(vec4(sphereLimit, 1.0));
	vec2 projSphereLimit2 = projSphereLimit.xy / projSphereLimit.w;
	float projRadius = length(projSphereLimit2 - ndc);

	// Integrate
	float factor = 0.0;
	for(uint i = 0U; i < SAMPLES; ++i)
	{
		// Rotate the disk point
		vec2 diskPoint = KERNEL[i] * projRadius;
		vec2 rotDiskPoint;
		rotDiskPoint.x = diskPoint.x * cosAng - diskPoint.y * sinAng;
		rotDiskPoint.y = diskPoint.y * cosAng + diskPoint.x * sinAng;
		vec2 finalDiskPoint = ndc + rotDiskPoint;

		vec3 s = readPosition(NDC_TO_UV(finalDiskPoint));
		vec3 u = s - origin;

		float f = max(dot(normal, u) + BIAS, 0.0) / max(dot(u, u), EPSILON);

		factor += f;
	}

	out_color = 1.0 - factor / float(SAMPLES) * STRENGTH;
}

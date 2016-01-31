// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// SSAO fragment shader
#include "shaders/Common.glsl"
#include "shaders/Pack.glsl"
#include "shaders/LinearDepth.glsl"
#include "shaders/RendererCommonUniforms.glsl"

const vec3 KERNEL[KERNEL_SIZE] = KERNEL_ARRAY; // This will be appended in C++

// Radius in game units
const float RADIUS = 1.1;

// Initial is 1.0 but the bigger it is the more darker the SSAO factor gets
const float DARKNESS_MULTIPLIER = 2.5;

// The algorithm will chose the number of samples depending on the distance
const float MAX_DISTANCE = 40.0;

layout(location = 0) in vec2 in_texCoords;

layout(location = 0) out float out_color;

layout(UBO_BINDING(0, 0), std140, row_major) uniform _blk
{
	RendererCommonUniforms u_uniforms;
};

layout(TEX_BINDING(0, 0)) uniform sampler2D u_mMsDepthRt;
layout(TEX_BINDING(0, 1)) uniform sampler2D u_msRt;
layout(TEX_BINDING(0, 2)) uniform sampler2D u_noiseMap;

// Get normal
vec3 readNormal(in vec2 uv)
{
	vec3 normal;
	readNormalFromGBuffer(u_msRt, uv, normal);
	return normal;
}

// Read the noise tex
vec3 readRandom(in vec2 uv)
{
	const vec2 tmp = vec2(float(WIDTH) / float(NOISE_MAP_SIZE),
		float(HEIGHT) / float(NOISE_MAP_SIZE));

	vec3 noise = texture(u_noiseMap, tmp * uv).xyz;
	// return normalize(noise * 2.0 - 1.0);
	return noise;
}

// Returns the Z of the position in view space
float readZ(in vec2 uv)
{
	float depth = texture(u_mMsDepthRt, uv).r;
	float z =
		u_uniforms.projectionParams.z / (u_uniforms.projectionParams.w + depth);
	return z;
}

// Read position in view space
vec3 readPosition(in vec2 uv)
{
	float depth = texture(u_mMsDepthRt, uv).r;

	vec3 fragPosVspace;
	fragPosVspace.z = readZ(uv);

	fragPosVspace.xy =
		(2.0 * uv - 1.0) * u_uniforms.projectionParams.xy * fragPosVspace.z;

	return fragPosVspace;
}

void main(void)
{
	vec3 origin = readPosition(in_texCoords);

	// Chose the number of samples dynamicaly
	float sampleCountf =
		max(1.0 + origin.z / MAX_DISTANCE, 0.0) * float(KERNEL_SIZE);
	uint sampleCount = uint(sampleCountf);

	vec3 normal = readNormal(in_texCoords);
	vec3 rvec = readRandom(in_texCoords);

	vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bitangent, normal);

	// Iterate kernel
	float factor = 0.0;
	for(uint i = 0U; i < sampleCount; ++i)
	{
		// get position
		vec3 sample_ = tbn * KERNEL[i];
		sample_ = sample_ * RADIUS + origin;

		// project sample position:
		vec4 offset = vec4(sample_, 1.0);
		offset = u_uniforms.projectionMatrix * offset;
		offset.xy = offset.xy / (2.0 * offset.w) + 0.5; // persp div &
		// to NDC -> [0, 1]

		// get sample depth:
		float sampleDepth = readZ(offset.xy);

		// range check & accumulate:
		const float ADVANCE = DARKNESS_MULTIPLIER / sampleCountf;

#if 1
		float rangeCheck =
			abs(origin.z - sampleDepth) * (1.0 / (RADIUS * 10.0));
		rangeCheck = 1.0 - rangeCheck;

		factor += clamp(sampleDepth - sample_.z, 0.0, ADVANCE) * rangeCheck;
#else
		float rangeCheck = abs(origin.z - sampleDepth) < RADIUS ? 1.0 : 0.0;
		factor += (sampleDepth > sample_.z ? ADVANCE : 0.0) * rangeCheck;
#endif
	}

	out_color = 1.0 - factor;
}

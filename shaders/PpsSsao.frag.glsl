// SSAO fragment shader
#pragma anki type frag
#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/Pack.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

const vec3 KERNEL[KERNEL_SIZE] = KERNEL_ARRAY; // This will be appended in C++

layout(location = 0) in vec2 inTexCoords;

layout(location = 0) out float outColor;

layout(std140, row_major, binding = 0) readonly buffer bCommon
{
	vec4 uProjectionParams;

	/// The projection matrix
	mat4 uProjectionMatrix;
};

layout(binding = 0) uniform sampler2D uMsDepthRt;
layout(binding = 1) uniform sampler2D uMsRt;
layout(binding = 2) uniform sampler2D uNoiseMap;

#define RADIUS 0.5
#define DARKNESS_MULTIPLIER 1.0 // Initial is 1.0 but the bigger it is the more
                                // darker the SSAO factor gets

// Get normal
vec3 readNormal(in vec2 uv)
{
	vec3 normal;
	readNormalFromGBuffer(uMsRt, uv, normal);
	return normal;
}

// Read the noise tex
vec3 readRandom(in vec2 uv)
{
	const vec2 tmp = vec2(
		float(WIDTH) / float(NOISE_MAP_SIZE), 
		float(HEIGHT) / float(NOISE_MAP_SIZE));

	vec3 noise = texture(uNoiseMap, tmp * uv).xyz;
	//return normalize(noise * 2.0 - 1.0);
	return noise;
}

// Returns the Z of the position in view space
float readZ(in vec2 uv)
{
	float depth = textureRt(uMsDepthRt, uv).r;
	float z = uProjectionParams.z / (uProjectionParams.w + depth);
	return z;
}

// Read position in view space
vec3 readPosition(in vec2 uv)
{
	float depth = textureRt(uMsDepthRt, uv).r;

	vec3 fragPosVspace;
	fragPosVspace.z = readZ(uv);
	
	fragPosVspace.xy = 
		(2.0 * uv - 1.0) * uProjectionParams.xy * fragPosVspace.z;

	return fragPosVspace;
}

void main(void)
{
	vec3 origin = readPosition(inTexCoords);

	vec3 normal = readNormal(inTexCoords);
	vec3 rvec = readRandom(inTexCoords);
	
	vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bitangent, normal);

	// Iterate kernel
	float factor = 0.0;
	for(uint i = 0U; i < KERNEL_SIZE; ++i) 
	{
		// get position
		vec3 sample_ = tbn * KERNEL[i];
		sample_ = sample_ * RADIUS + origin;

		// project sample position:
		vec4 offset = vec4(sample_, 1.0);
		offset = uProjectionMatrix * offset;
		offset.xy = offset.xy / (2.0 * offset.w) + 0.5; // persp div & 
		                                                // to NDC -> [0, 1]

		// get sample depth:
		float sampleDepth = readZ(offset.xy);

		// range check & accumulate:
		const float ADVANCE = DARKNESS_MULTIPLIER / float(KERNEL_SIZE);

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

	outColor = 1.0 - factor;
}


#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader
#pragma anki include "shaders/CommonFrag.glsl"
#pragma anki include "shaders/Pack.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

in vec2 vTexCoords;

layout(location = 0) out float fColor;

layout(std140, row_major) uniform commonBlock
{
	/// Packs:
	/// - x: zNear. For the calculation of frag pos in view space
	/// - zw: Planes. For the calculation of frag pos in view space
	vec4 nearPlanes;

	/// For the calculation of frag pos in view space. The xy is the 
	/// limitsOfNearPlane and the zw is an optimization see PpsSsao.glsl and 
	/// r403 for the clean one
	vec4 limitsOfNearPlane_;

	/// The projection matrix
	mat4 projectionMatrix;
};

#define zNear nearPlanes.x
#define zFar nearPlanes.y
#define planes nearPlanes.zw
#define limitsOfNearPlane limitsOfNearPlane_.xy
#define limitsOfNearPlane2 limitsOfNearPlane_.zw

uniform sampler2D msDepthFai;
#if USE_MRT
uniform sampler2D msGFai;
#else
uniform highp usampler2D msGFai;
#endif
uniform sampler2D noiseMap; 
/// @}

#define RADIUS 0.5
#define DARKNESS_MULTIPLIER 2.0 // Initial is 1.0 but the bigger it is the more
                                // darker the SSAO factor gets

// Get normal
vec3 getNormal(in vec2 uv)
{
#if USE_MRT
	vec3 normal = unpackNormal(texture(msGFai, uv).rg);
#else
	uvec2 msAll = texture(msGFai, uv).rg;
	vec3 normal = unpackNormal(unpackHalf2x16(msAll[1]));
#endif
	return normal;
}

// Read the noise tex
vec3 getRandom(in vec2 uv)
{
	const vec2 tmp = vec2(
		float(WIDTH) / float(NOISE_MAP_SIZE), 
		float(HEIGHT) / float(NOISE_MAP_SIZE));

	vec3 noise = texture(noiseMap, tmp * uv).xyz;
	//return normalize(noise * 2.0 - 1.0);
	return noise;
}

// Get position in view space
vec3 getPosition(in vec2 uv)
{
	float depth = texture(msDepthFai, uv).r;

	vec3 fragPosVspace;
	fragPosVspace.z = -planes.y / (planes.x + depth);
	
	fragPosVspace.xy = (uv * limitsOfNearPlane2) - limitsOfNearPlane;
	
	float sc = -fragPosVspace.z;
	fragPosVspace.xy *= sc;

	return fragPosVspace;
}

float getZ(in vec2 uv)
{
	float depth = texture(msDepthFai, uv).r;
	float z = -planes.y / (planes.x + depth);
	return z;
}

void main(void)
{
	vec3 origin = getPosition(vTexCoords);

	vec3 normal = getNormal(vTexCoords);
	vec3 rvec = getRandom(vTexCoords);
	
	vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bitangent, normal);

	// Iterate kernel
	float factor = 0.0;
	for(uint i = 0; i < KERNEL_SIZE; ++i) 
	{
		// get position
		vec3 sample_ = tbn * KERNEL[i];
		sample_ = sample_ * RADIUS + origin;

		// project sample position:
		vec4 offset = vec4(sample_, 1.0);
		offset = projectionMatrix * offset;
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;

		// get sample depth:
		float sampleDepth = getZ(offset.xy);

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

	fColor = 1.0 - factor;
}


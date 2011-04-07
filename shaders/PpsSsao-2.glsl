#pragma anki vertShaderBegins

layout(location = 0) in vec2 position; ///< the vert coords are {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0}
layout(location = 1) in vec3 viewVector;

out vec2 vTexCoords;
out vec3 vPosition;


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main()
{
	vPosition = viewVector;
	vTexCoords = position;
	vec2 vertPosNdc = position * 2.0 - 1.0;
	gl_Position = vec4(vertPosNdc, 0.0, 1.0);
}


#pragma anki fragShaderBegins

#pragma anki include "shaders/Pack.glsl"

/// @name Uniforms
/// @{
uniform vec2 camerarange;  // = vec2( znear, zfar )
uniform vec2 planes; ///< for the calculation of frag pos in view space	
uniform sampler2D msDepthFai;
uniform sampler2D noiseMap;
uniform sampler2D msNormalFai;

uniform float noiseMapSize;
uniform float g_sample_rad;
uniform float g_intensity;
uniform float g_scale;
uniform float g_bias;
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
in vec3 vPosition; ///< for the calculation of frag pos in view space
/// @}

/// @name Output
/// @{
layout(location = 0) out float fColor;
/// @}


vec3 getPosition(in sampler2D depthMap, in vec2 uv)
{
	float depth = texture2D(depthMap, uv).r;

	vec3 fragPosVspace;
	vec3 vposn = normalize(vPosition);
	fragPosVspace.z = -planes.y / (planes.x + depth);
	fragPosVspace.xy = vposn.xy * (fragPosVspace.z / vposn.z);
	return fragPosVspace;
}


vec3 getNormal(in sampler2D normalMap, in vec2 uv)
{
	return unpackNormal(texture2D(normalMap, uv).rg);
}


vec2 getRandom(in sampler2D noiseMap, in vec2 uv)
{
	return normalize(texture2D(noiseMap, screenSize * uv / noiseMapSize).xy * 2.0f - 1.0f);
}


float doAmbientOcclusion(in vec2 tcoord, in vec2 uv, in vec3 p, in vec3 cnorm)
{
	vec3 diff = getPosition(tcoord + uv) - p;
	vec3 v = normalize(diff);
	float d = length(diff) * g_scale;
	return max(0.0, dot(cnorm, v) - g_bias) * (1.0 / (1.0 + d)) * g_intensity;
}


void main(void)
{
	fColor = 1.0f;
	
	const vec3 kernel[4] = vec3[](vec2(1, 0), vec2(-1, 0), vec2(0, 1), vec2(0, -1));

	vec3 p = getPosition(msDepthFai, vTexCoords);
	vec3 n = getNormal(msNormalFai, vTexCoords);
	vec2 rand = getRandom(noiseMap, vTexCoords);

	float ao = 0.0f;
	float rad = g_sample_rad / p.z;

	// SSAO Calculation
	const int iterations = 4;
	for(int j = 0; j < iterations; ++j)
	{
		vec2 coord1 = reflect(kernel[j], rand) * rad;
		vec2 coord2 = vec2(coord1.x * 0.707 - coord1.y * 0.707, coord1.x * 0.707 + coord1.y * 0.707);

		ao += doAmbientOcclusion(vTexCoords, coord1 * 0.25, p, n);
		ao += doAmbientOcclusion(vTexCoords, coord2 * 0.5, p, n);
		ao += doAmbientOcclusion(vTexCoords, coord1 * 0.75, p, n);
		ao += doAmbientOcclusion(vTexCoords, coord2, p, n);
	} 
	ao /= 4.0;

	fColor = ao;
}


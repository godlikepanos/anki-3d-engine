#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

#pragma anki include "shaders/Pack.glsl"

/// @name Uniforms
/// @{
uniform vec2 planes; ///< for the calculation of frag pos in view space
uniform vec2 limitsOfNearPlane; ///< for the calculation of frag pos in view space
uniform float zNear; ///< for the calculation of frag pos in view space
uniform sampler2D msDepthFai; ///< for the calculation of frag pos in view space

uniform sampler2D noiseMap; /// Used in getRandom
uniform float noiseMapSize = 100.0; /// Used in getRandom
uniform vec2 screenSize; /// Used in getRandom

uniform sampler2D msNormalFai; /// Used in getNormal
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
/// @}

/// @name Output
/// @{
layout(location = 0) out float fColor;
/// @}


/// @name Consts
/// @{
uniform float SAMPLE_RAD = 0.1;  /// Used in main
uniform float SCALE = 1.0; /// Used in doAmbientOcclusion
uniform float INTENSITY = 3.0; /// Used in doAmbientOcclusion
uniform float BIAS = 0.00; /// Used in doAmbientOcclusion
/// @}


/// globals: msNormalFai
vec3 getNormal(in vec2 uv)
{
	return unpackNormal(texture2D(msNormalFai, uv).rg);
}


/// globals: noiseMap, screenSize, noiseMapSize
vec2 getRandom(in vec2 uv)
{
	return normalize(texture2D(noiseMap, screenSize * uv / noiseMapSize).xy * 2.0 - 1.0);
}


/// Get frag position in view space
/// globals: msDepthFai, planes, zNear, limitsOfNearPlane
vec3 getPosition(in vec2 uv)
{
	float depth = texture2D(msDepthFai, uv).r;

	vec3 fragPosVspace;
	fragPosVspace.z = -planes.y / (planes.x + depth);
	
	fragPosVspace.xy = (((uv) * 2.0) - 1.0) * limitsOfNearPlane;
	
	float sc = -fragPosVspace.z / zNear;
	fragPosVspace.xy *= sc;

	return fragPosVspace;
}

/// Calculate the ambient occlusion factor
float doAmbientOcclusion(in vec2 tcoord, in vec2 uv, in vec3 original, in vec3 cnorm)
{
	vec3 newp = getPosition(tcoord + uv);
	vec3 diff = newp - original;
	vec3 v = normalize(diff);
	float d = length(diff) /* * SCALE*/;

	float ret = max(0.0, dot(cnorm, v) /* - BIAS*/) * (INTENSITY / (1.0 + d));
	return ret;
}


void main(void)
{
	const vec2 KERNEL[16] = vec2[](vec2(0.53812504, 0.18565957), vec2(0.13790712, 0.24864247), vec2(0.33715037, 0.56794053), vec2(-0.6999805, -0.04511441), vec2(0.06896307, -0.15983082), vec2(0.056099437, 0.006954967), vec2(-0.014653638, 0.14027752), vec2(0.010019933, -0.1924225), vec2(-0.35775623, -0.5301969), vec2(-0.3169221, 0.106360726), vec2(0.010350345, -0.58698344), vec2(-0.08972908, -0.49408212), vec2(0.7119986, -0.0154690035), vec2(-0.053382345, 0.059675813), vec2(0.035267662, -0.063188605), vec2(-0.47761092, 0.2847911));

	vec3 p = getPosition(vTexCoords);
	vec3 n = getNormal(vTexCoords);
	vec2 rand = getRandom(vTexCoords);

	fColor = 0.0;

	const int ITERATIONS = 16;
	for(int j = 0; j < ITERATIONS; ++j)
	{
		vec2 coord = reflect(KERNEL[j], rand) * SAMPLE_RAD;
		fColor += doAmbientOcclusion(vTexCoords, coord, p, n);
	}

	fColor = 1.0 - fColor / ITERATIONS;
}


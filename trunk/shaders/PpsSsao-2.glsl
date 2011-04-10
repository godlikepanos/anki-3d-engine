#pragma anki vertShaderBegins

layout(location = 0) in vec2 position; ///< the vert coords are {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0}
layout(location = 1) in vec3 viewVector;

out vec2 vTexCoords;
out vec3 vViewVector;


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main()
{
	vViewVector = viewVector;
	vTexCoords = position;
	vec2 vertPosNdc = position * 2.0 - 1.0;
	gl_Position = vec4(vertPosNdc, 0.0, 1.0);
}


#pragma anki fragShaderBegins

#pragma anki include "shaders/Pack.glsl"

/// @name Uniforms
/// @{
uniform vec2 planes; ///< for the calculation of frag pos in view space	
uniform sampler2D msDepthFai; ///< for the calculation of frag pos in view space	
uniform sampler2D noiseMap;
uniform sampler2D msNormalFai;
uniform float noiseMapSize = 100.0; /// Used in getRandom
uniform float sampleRad = 0.1;  /// Used in main
uniform float scale = 1.0; /// Used in doAmbientOcclusion
uniform float intensity = 1.0; /// Used in doAmbientOcclusion
uniform float bias = 0.001; /// Used in doAmbientOcclusion
uniform vec2 screenSize; /// Used in getRandom
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
in vec3 vViewVector; ///< for the calculation of frag pos in view space
/// @}

/// @name Output
/// @{
layout(location = 0) out float fColor;
/// @}


/// Get frag position in view space
/// globals: msDepthFai, planes, vViewVector
vec3 getPosition(in vec2 uv)
{
	float depth = texture2D(msDepthFai, uv).r;

	vec3 fragPosVspace;
	vec3 vposn = normalize(vViewVector);
	fragPosVspace.z = -planes.y / (planes.x + depth);
	fragPosVspace.xy = vposn.xy * (fragPosVspace.z / vposn.z);
	return fragPosVspace;
}


/// globals: msNormalFai
vec3 getNormal(in vec2 uv)
{
	return unpackNormal(texture2D(msNormalFai, uv).rg);
}


/// globals: noiseMap, screenSize, noiseMapSize
vec2 getRandom(in vec2 uv)
{
	return normalize(texture2D(noiseMap, screenSize * uv / noiseMapSize).xy * 2.0f - 1.0f);
}


float doAmbientOcclusion(in vec2 tcoord, in vec2 uv, in vec3 p, in vec3 cnorm)
{
	vec3 diff = getPosition(tcoord + uv) - p;
	vec3 v = normalize(diff);
	float d = length(diff) * scale;
	return max(0.0, dot(cnorm, v) - bias) * (1.0 / (1.0 + d)) * intensity;
}


void main(void)
{	
	const vec2 kernel[4] = vec2[](vec2(1, 0), vec2(-1, 0), vec2(0, 1), vec2(0, -1));

	vec3 p = getPosition(vTexCoords);
	vec3 n = getNormal(vTexCoords);
	vec2 rand = getRandom(vTexCoords);

	float ao = 0.0f;
	float rad = sampleRad ;// p.z;

	// SSAO Calculation
	const int ITERATIONS = 4;
	for(int j = 0; j < ITERATIONS; ++j)
	{
		vec2 coord1 = reflect(kernel[j], rand) * rad;
		vec2 coord2 = vec2(coord1.x * 0.707 - coord1.y * 0.707, coord1.x * 0.707 + coord1.y * 0.707);

		ao += doAmbientOcclusion(vTexCoords, coord1 * 0.25, p, n);
		ao += doAmbientOcclusion(vTexCoords, coord2 * 0.5, p, n);
		ao += doAmbientOcclusion(vTexCoords, coord1 * 0.75, p, n);
		ao += doAmbientOcclusion(vTexCoords, coord2, p, n);
	} 
	ao /= ITERATIONS * 4.0;

	fColor = 1 - ao;
	
	
	/*vec2 v = getRandom(vTexCoords);	
	fColor = (fColor - fColor) + ((v.x + v.y) / 2.0);*/
}


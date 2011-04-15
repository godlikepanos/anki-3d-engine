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
uniform sampler2D msPosFai;
uniform sampler2D noiseMap;
uniform sampler2D msNormalFai;
uniform vec2 limitsOfNearPlane;
uniform float noiseMapSize = 100.0; /// Used in getRandom
uniform float sampleRad = 0.1;  /// Used in main
uniform float scale = 1.0; /// Used in doAmbientOcclusion
uniform float intensity = 2.0; /// Used in doAmbientOcclusion
uniform float bias = 0.00; /// Used in doAmbientOcclusion
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
/// globals: msDepthFai, planes
vec3 getPosition(in vec2 uv)
{
	float depth = texture2D(msDepthFai, uv).r;

	vec3 fragPosVspace;
	fragPosVspace.z = -planes.y / (planes.x + depth);

	/*vec3 vposn = normalize(vViewVector);
	fragPosVspace.xy = vposn.xy * (fragPosVspace.z / vposn.z);*/
	
	fragPosVspace.xy = (((uv) * 2.0) - 1.0) * limitsOfNearPlane;
	
	float sc = -fragPosVspace.z / 0.5;
    fragPosVspace.xy *= sc;

	return fragPosVspace;
	//return abs(fragPosVspace - texture2D(msPosFai, uv).xyz);
	//return vec3(texture2D(msPosFai, uv).xy, fragPosVspace.z);
}


float doAmbientOcclusion(in vec2 tcoord, in vec2 uv, in vec3 original, in vec3 cnorm)
{
	vec3 newp = getPosition(tcoord + uv);
	vec3 diff = newp - original;
	vec3 v = normalize(diff);
	float d = length(diff) * scale;

	float ret = max(0.0, dot(cnorm, v) - bias) * (1.0 / (1.0 + d)) * intensity;
	return ret;
	//return newp.x / 10.0;
}


void main(void)
{
	//const vec2 kernel[4] = vec2[](vec2(1, 0), vec2(-1, 0), vec2(0, 1), vec2(0, -1));

	const vec2 KERNEL[16] = vec2[](vec2(0.53812504, 0.18565957), vec2(0.13790712, 0.24864247), vec2(0.33715037, 0.56794053), vec2(-0.6999805, -0.04511441), vec2(0.06896307, -0.15983082), vec2(0.056099437, 0.006954967), vec2(-0.014653638, 0.14027752), vec2(0.010019933, -0.1924225), vec2(-0.35775623, -0.5301969), vec2(-0.3169221, 0.106360726), vec2(0.010350345, -0.58698344), vec2(-0.08972908, -0.49408212), vec2(0.7119986, -0.0154690035), vec2(-0.053382345, 0.059675813), vec2(0.035267662, -0.063188605), vec2(-0.47761092, 0.2847911));

	vec3 original = getPosition(vTexCoords);
	vec3 n = getNormal(vTexCoords);
	vec2 rand = getRandom(vTexCoords);

	//rand = rand - rand + vec2(0.0, 0.0);

	fColor = 0.0;
	float rad = sampleRad ;// p.z;

	// SSAO Calculation
	/*const int ITERATIONS = 4;
	for(int j = 0; j < ITERATIONS; ++j)
	{
		vec2 coord1 = reflect(kernel[j], rand) * rad;
		vec2 coord2 = vec2(coord1.x * 0.707 - coord1.y * 0.707, coord1.x * 0.707 + coord1.y * 0.707);

		ao += doAmbientOcclusion(vTexCoords, coord1 * 0.25, p, n);
		ao += doAmbientOcclusion(vTexCoords, coord2 * 0.5, p, n);
		ao += doAmbientOcclusion(vTexCoords, coord1 * 0.75, p, n);
		ao += doAmbientOcclusion(vTexCoords, coord2, p, n);
	} 
	ao /= ITERATIONS * 4.0;*/

	const int ITERATIONS = 16;
	for(int j = 0; j < ITERATIONS; ++j)
	{
		vec2 coord = reflect(KERNEL[j], rand) * rad;
		fColor += doAmbientOcclusion(vTexCoords, coord, original, n);
	}

	fColor = 1.0 - fColor / ITERATIONS;

	//fColor = gl_FragCoord.y / (720.0 / 2);

	/*vec3 diff = getPosition(vTexCoords) - texture2D(msPosFai, vTexCoords).xyz;
	fColor = (diff.x + diff.y + diff.z) / 3.0;*/

	//fColor = original.x;
	
	/*vec2 v = getRandom(vTexCoords);	
	fColor = (fColor - fColor) + ((v.x + v.y) / 2.0);*/
}


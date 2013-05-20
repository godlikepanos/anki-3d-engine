#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader
#pragma anki include "shaders/CommonFrag.glsl"
#pragma anki include "shaders/Pack.glsl"

/// @name Varyings
/// @{
in vec2 vTexCoords;
/// @}

/// @name Output
/// @{
layout(location = 0) out float fColor;
/// @}


/// @name Uniforms
/// @{
layout(std140, row_major) uniform commonBlock
{
	/// Packs:
	/// - x: zNear. For the calculation of frag pos in view space
	/// - zw: Planes. For the calculation of frag pos in view space
	uniform vec4 nearPlanes;

	/// For the calculation of frag pos in view space. The xy is the 
	/// limitsOfNearPlane and the zw is an optimization see PpsSsao.glsl and 
	/// r403 for the clean one
	uniform vec4 limitsOfNearPlane_;
};

#define planes nearPlanes.zw
#define zNear nearPlanes.x
#define limitsOfNearPlane limitsOfNearPlane_.xy
#define limitsOfNearPlane2 limitsOfNearPlane_.zw

uniform sampler2D msDepthFai;
uniform highp usampler2D msGFai;
uniform sampler2D noiseMap; 
/// @}

#define SAMPLE_RAD 0.08
#define SCALE 1.0
#define INTENSITY 3.0
#define BIAS 0.1

vec3 getNormal(in vec2 uv)
{
	uvec2 msAll = texture(msGFai, uv).rg;
	vec3 normal = unpackNormal(unpackHalf2x16(msAll[1]));
	return normal;
}

vec2 getRandom(in vec2 uv)
{
	const vec2 tmp = vec2(
		float(WIDTH) / float(NOISE_MAP_SIZE), 
		float(HEIGHT) / float(NOISE_MAP_SIZE));

	vec2 noise = texture(noiseMap, tmp * uv).xy;
	//return normalize(noise * 2.0 - 1.0);r
	return noise;
}

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

float calcAmbientOcclusionFactor(in vec2 uv, in vec3 original, in vec3 cnorm)
{
	vec3 newp = getPosition(uv);
	vec3 diff = newp - original;
	vec3 v = normalize(diff);
	float d = length(diff) /* * SCALE*/;

	float ret = max(0.0, dot(cnorm, v)  - BIAS) * (INTENSITY / (1.0 + d));
	return ret;
}

#define KERNEL_SIZE 16
const vec2 KERNEL[KERNEL_SIZE] = vec2[](
	vec2(0.53812504, 0.18565957), vec2(0.13790712, 0.24864247), 
	vec2(0.33715037, 0.56794053), vec2(-0.6999805, -0.04511441),
	vec2(0.06896307, -0.15983082), vec2(0.056099437, 0.006954967),
	vec2(-0.014653638, 0.14027752), vec2(0.010019933, -0.1924225),
	vec2(-0.35775623, -0.5301969), vec2(-0.3169221, 0.106360726),
	vec2(0.010350345, -0.58698344), vec2(-0.08972908, -0.49408212),
	vec2(0.7119986, -0.0154690035), vec2(-0.053382345, 0.059675813),
	vec2(0.035267662, -0.063188605), vec2(-0.47761092, 0.2847911));

void main(void)
{
	vec3 p = getPosition(vTexCoords);
	vec3 n = getNormal(vTexCoords);
	vec2 rand = getRandom(vTexCoords);
	//rand = rand * 0.000001 + vec2(0.0, 0.0);

	fColor = 0.0;
	
	for(int j = 0; j < KERNEL_SIZE; ++j)
	{
		vec2 coord = reflect(KERNEL[j], rand) * SAMPLE_RAD;
		fColor += calcAmbientOcclusionFactor(vTexCoords + coord, p, n);
	}

	fColor = 1.0 - fColor / float(KERNEL_SIZE);

	//fColor = fColor * 0.00001 + (rand.x + rand.y) / 2.0;
}


/// @file
///
/// Illumination stage lighting pass general shader program

#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

#pragma anki include "shaders/Pack.glsl"

#define DISCARD 0
#if !defined(MAX_LIGHTS)
#	error "MAX_LIGHTS not defined"
#endif

/// @name Uniforms
/// @{

struct Light
{
	vec4 posAndRadius; ///< xyz: Light pos in eye space. w: The radius
	vec4 diffuseColor;
	vec4 specularColor;
#if SPOT_LIGHT
	mat4 texProjectionMat;
#endif
};

// Light data
layout(std140, row_major) uniform lightBlock
{
	uniform Light lights[MAX_LIGHTS];
};

// Common uniforms between lights
layout(std140) uniform generalBlock
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

uniform float lightsCount;

uniform usampler2D msFai0;
uniform sampler2D msDepthFai;
uniform sampler2D lightTex;
uniform sampler2DShadow shadowMap;
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
/// @}

/// @name Output
/// @{
out vec3 fColor;
/// @}

//==============================================================================
/// @return frag pos in view space
vec3 getFragPosVSpace()
{
	float depth = texture(msDepthFai, vTexCoords).r;

#if DISCARD
	if(depth == 1.0)
	{
		discard;
	}
#endif

	vec3 fragPosVspace;
	fragPosVspace.z = -planes.y / (planes.x + depth);

	fragPosVspace.xy = (vTexCoords * limitsOfNearPlane2) - limitsOfNearPlane;

	float sc = -fragPosVspace.z / zNear;
	fragPosVspace.xy *= sc;

	return fragPosVspace;
}

//==============================================================================
/// @return The blurred shadow
float pcfLow(in vec3 shadowUv)
{
	float shadowCol = textureOffset(shadowMap, shadowUv, ivec2(-1, -1));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(0, -1));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(1, -1));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(-1, 0));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(0, 0));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(1, 0));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(-1, 1));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(0, 1));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(1, 1));

	shadowCol *= (1.0 / 9.0);
	return shadowCol;
}

//==============================================================================
/// Performs phong lighting using the MS FAIs and a few other things
/// @param fragPosVspace The fragment position in view space
/// @return The final color
vec3 doPhong(in vec3 fragPosVspace, in vec3 normal, in vec3 diffuse, 
	in vec2 specularAll, in Light light)
{
	// get the vector from the frag to the light
	vec3 frag2LightVec = light.posAndRadius.xyz - fragPosVspace;

	// Instead of using normalize(frag2LightVec) we brake the operation 
	// because we want fragLightDist for the calc of the attenuation
	float fragLightDistSqrt = sqrt(dot(frag2LightVec, frag2LightVec));
	vec3 lightDir = frag2LightVec / fragLightDistSqrt;

	// Lambert term
	float lambertTerm = dot(normal, lightDir);

#if DISCARD
	if(lambertTerm < 0.0)
	{
		discard;
	}
#else
	lambertTerm = max(0.0, lambertTerm);
#endif

	// Attenuation
	float attenuation = 
		clamp(1.0 - fragLightDistSqrt / light.posAndRadius.w, 0.0, 1.0);

	// Diffuse
	vec3 difCol = diffuse * light.diffuseColor.rgb;

	// Specular
	vec3 specCol;
	if(fragLightDistSqrt < 0.9)
	{
		vec3 eyeVec = normalize(-fragPosVspace);
		vec3 h = normalize(lightDir + eyeVec);
		float specIntensity = pow(max(0.0, dot(normal, h)), specularAll.g);
		specCol = light.specularColor.rgb * (specIntensity * specularAll.r);
	}
	else
	{
		specCol = vec3(0.0);
	}
	
	// end
	return (difCol + specCol) * (attenuation * lambertTerm);
}

//==============================================================================
void main()
{
	// Read texture first. Optimize for future out of order HW
	uvec2 msAll = texture(msFai0, vTexCoords).rg;

	// get frag pos in view space
	vec3 fragPosVspace = getFragPosVSpace();

	// Decode MS
	vec3 normal = unpackNormal(unpackHalf2x16(msAll[1]));
	vec4 diffuseAndSpec = unpackUnorm4x8(msAll[0]);
	vec2 specularAll = unpackSpecular(diffuseAndSpec.a);

#if POINT_LIGHT
	fColor = doPhong(fragPosVspace, normal, diffuseAndSpec.rgb, 
		specularAll, lights[0]);
	for(int i = 1; i < int(lightsCount); i++)
	{
		fColor += doPhong(fragPosVspace, normal, diffuseAndSpec.rgb, 
			specularAll, lights[i]);
	}
#elif SPOT_LIGHT
	// XXX
#else
#	error "See file"
#endif
}

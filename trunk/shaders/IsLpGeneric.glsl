/// @file
///
/// Illumination stage lighting pass general shader program

#pragma anki start vertexShader

#pragma anki include "shaders/IsLpVertex.glsl"

#pragma anki start fragmentShader

#pragma anki include "shaders/Pack.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

#define DISCARD 0

#if !MAX_LIGHTS_PER_TILE || !TILES_X_COUNT || !TILES_Y_COUNT || !TILES_COUNT
#	error "See file"
#endif

/// @name Uniforms
/// @{

// Common uniforms between lights
layout(std140, row_major, binding = 0) uniform commonBlock
{
	/// Packs:
	/// - x: zNear. For the calculation of frag pos in view space
	/// - zw: Planes. For the calculation of frag pos in view space
	uniform vec4 nearPlanes;

	/// For the calculation of frag pos in view space. The xy is the 
	/// limitsOfNearPlane and the zw is an optimization see PpsSsao.glsl and 
	/// r403 for the clean one
	uniform vec4 limitsOfNearPlane_;

	uniform vec4 sceneAmbientColor;
};

#define planes nearPlanes.zw
#define zNear nearPlanes.x
#define limitsOfNearPlane limitsOfNearPlane_.xy
#define limitsOfNearPlane2 limitsOfNearPlane_.zw

struct Light
{
	vec4 posAndRadius; ///< xyz: Light pos in eye space. w: The radius
	vec4 diffuseColor;
	vec4 specularColor;
#if SPOT_LIGHT
	mat4 texProjectionMat;
#endif
};

layout(std140, row_major, binding = 1) uniform lightsBlock
{
	Light lights[MAX_LIGHTS];
};

struct Tile
{
	uint lightsCount;
	uvec4 lightIndices[MAX_LIGHTS_PER_TILE / 4];
};

layout(std140, row_major, binding = 2) uniform tilesBlock
{
	Tile tiles[TILES_X_COUNT * TILES_Y_COUNT];
};

uniform usampler2D msFai0;
uniform sampler2D msDepthFai;
uniform sampler2D lightTex;
uniform sampler2DShadow shadowMap;
uniform sampler2D minmax;
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
flat in uint vInstanceId;
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
		max(1.0 - fragLightDistSqrt / light.posAndRadius.w, 0.0);

	// Diffuse
	vec3 difCol = diffuse * light.diffuseColor.rgb;

	// Specular
	vec3 eyeVec = normalize(-fragPosVspace);
	vec3 h = normalize(lightDir + eyeVec);
	float specIntensity = pow(max(0.0, dot(normal, h)), specularAll.g);
	vec3 specCol = light.specularColor.rgb * (specIntensity * specularAll.r);
	
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

	// Lighting
	uint lightsCount = tiles[vInstanceId].lightsCount;

	fColor = diffuseAndSpec.rgb * sceneAmbientColor.rgb;
	for(uint i = 0; i < lightsCount; ++i)
	{
		uint lightId = tiles[vInstanceId].lightIndices[i / 4][i % 4];

#if 1
		fColor += doPhong(fragPosVspace, normal, diffuseAndSpec.rgb, 
			specularAll, lights[lightId]);
#else
		fColor += lights[lightId].diffuseColor.rgb;
#endif
	}

#if 0
	float depth = texture(msDepthFai, vTexCoords).r;
	vec2 mm = texture(minmax, vTexCoords).rg;

	if(depth < mm.x)
	{
		fColor *= vec3(10.0, 0.0, 0.0);
	}
	if(depth > mm.y)
	{
		fColor *= vec3(0.0, 0.0, 10.0);
	}
#endif

#if 0
	if(lightsCount > 0)
	{
		fColor += vec3(0.0, float(lightsCount) / 7.0, 0.0);
	}
#endif
}

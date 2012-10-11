/// @file
///
/// Illumination stage lighting pass general shader program

#pragma anki start vertexShader

#pragma anki include "shaders/IsLpVertex.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

#pragma anki start fragmentShader

#pragma anki include "shaders/Pack.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

#if !MAX_LIGHTS_PER_TILE || !TILES_X_COUNT || !TILES_Y_COUNT
#	error "See file"
#endif

#if !MAX_POINT_LIGHTS || !MAX_SPOT_LIGHTS
#	error "See file"
#endif

#define ATTENUATION_FINE 0

#define ATTENUATION_BOOST (0.1)

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
};

struct SpotLight
{
	Light light;
	vec4 lightDirection;
	mat4 texProjectionMat;
};

layout(std140, row_major, binding = 1) uniform pointLightsBlock
{
	Light plights[MAX_POINT_LIGHTS];
};

layout(std140, binding = 2) uniform spotLightsBlock
{
	SpotLight slights[MAX_SPOT_LIGHTS];
};

struct Tile
{
	uvec4 lightsCount;
	uvec4 lightIndices[MAX_LIGHTS_PER_TILE / 4];
};

layout(std140, row_major, binding = 3) uniform tilesBlock
{
	Tile tiles[TILES_X_COUNT * TILES_Y_COUNT];
};

uniform usampler2D msFai0;
uniform sampler2D msDepthFai;

uniform sampler2D lightTextures[MAX_SPOT_LIGHTS];
uniform sampler2DShadow shadowMaps[MAX_SPOT_LIGHTS];
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
	const float depth = texture(msDepthFai, vTexCoords).r;

	vec3 fragPosVspace;
	fragPosVspace.z = -planes.y / (planes.x + depth);

	fragPosVspace.xy = (vTexCoords * limitsOfNearPlane2) - limitsOfNearPlane;

	const float sc = -fragPosVspace.z / zNear;
	fragPosVspace.xy *= sc;

	return fragPosVspace;
}

//==============================================================================
float calcAttenuationFactor(
	in vec3 fragPosVspace, 
	in Light light,
	out vec3 rayDir)
{
	// get the vector from the frag to the light
	const vec3 frag2LightVec = light.posAndRadius.xyz - fragPosVspace;

	// Instead of using normalize(frag2LightVec) we brake the operation 
	// because we want fragLightDist for the calc of the attenuation
	const float fragLightDistSquared = dot(frag2LightVec, frag2LightVec);
	const float fragLightDist = sqrt(fragLightDistSquared);
	rayDir = frag2LightVec / fragLightDist;

	const float att = max((1.0 + ATTENUATION_BOOST) 
		- fragLightDist / light.posAndRadius.w, 0.0);

	return att;
}

//==============================================================================
float calcLambertTerm(in vec3 normal, in vec3 rayDir)
{
	return max(0.0, dot(normal, rayDir));
}

//==============================================================================
/// Performs phong lighting using the MS FAIs and a few other things
vec3 calcPhong(in vec3 fragPosVspace, in vec3 diffuse, 
	in vec2 specularAll, in vec3 normal, in Light light, in vec3 rayDir)
{
	// Diffuse
	vec3 difCol = diffuse * light.diffuseColor.rgb;

	// Specular
	vec3 eyeVec = normalize(fragPosVspace);
	vec3 h = normalize(rayDir - eyeVec);
	float specIntensity = pow(max(0.0, dot(normal, h)), specularAll.g);
	vec3 specCol = light.specularColor.rgb * (specIntensity * specularAll.r);
	
	// end
	return difCol + specCol;
}

//==============================================================================
float calcSpotFactor(in SpotLight light, in vec3 fragPosVspace)
{
	vec3 l = normalize(fragPosVspace - light.light.posAndRadius.xyz);

	float costheta = dot(l, light.lightDirection.xyz);
	float spotFactor = smoothstep(
		light.light.diffuseColor.w, 
		light.light.specularColor.w, 
		costheta);

	return spotFactor;
}

//==============================================================================
float calcShadowFactor(in SpotLight light, in vec3 fragPosVspace, 
	in sampler2DShadow shadowMap)
{
	vec4 texCoords4 = light.texProjectionMat * vec4(fragPosVspace, 1.0);
	vec3 texCoords3 = texCoords4.xyz / texCoords4.w;
	float shadowFactor = texture(shadowMap, texCoords3).r;

	return shadowFactor;
}

//==============================================================================
void main()
{
	// Read texture first. Optimize for future out of order HW
	uvec2 msAll = texture(msFai0, vTexCoords).rg;

	// get frag pos in view space
	const vec3 fragPosVspace = getFragPosVSpace();

	// Decode MS
	vec3 normal = unpackNormal(unpackHalf2x16(msAll[1]));
	vec4 diffuseAndSpec = unpackUnorm4x8(msAll[0]);
	vec2 specularAll = unpackSpecular(diffuseAndSpec.a);

	// Ambient color
	fColor = diffuseAndSpec.rgb * sceneAmbientColor.rgb;

	// Point lights
	uint pointLightsCount = tiles[vInstanceId].lightsCount[0];
	for(uint i = 0; i < pointLightsCount; ++i)
	{
		uint lightId = tiles[vInstanceId].lightIndices[i / 4][i % 4];

		vec3 ray;
		const float att = calcAttenuationFactor(fragPosVspace, 
			plights[lightId], ray);

		const float lambert = calcLambertTerm(normal, ray);

		fColor += calcPhong(fragPosVspace, diffuseAndSpec.rgb, 
			specularAll, normal, plights[lightId], ray) * (att * lambert);
	}

	// Spot lights
	uint spotLightsCount = tiles[vInstanceId].lightsCount[1];

	uint opt = pointLightsCount + spotLightsCount;
	for(uint i = pointLightsCount; i < opt; ++i)
	{
		uint lightId = tiles[vInstanceId].lightIndices[i / 4][i % 4];

		vec3 ray;
		const float att = calcAttenuationFactor(fragPosVspace, 
			slights[lightId].light, ray);

		const float lambert = calcLambertTerm(normal, ray);

		const float spot = calcSpotFactor(slights[lightId], fragPosVspace);

		const vec3 col = calcPhong(fragPosVspace, diffuseAndSpec.rgb, 
			specularAll, normal, slights[lightId].light, ray);

		fColor += col * (att * lambert * spot);
	}

	// Spot lights with shadow
	const uint spotLightsShadowCount = tiles[vInstanceId].lightsCount[2];
	opt = pointLightsCount + spotLightsCount;

	for(uint i = 0; i < spotLightsShadowCount; ++i)
	{
		uint id = i + opt;
		uint lightId = tiles[vInstanceId].lightIndices[id / 4][id % 4];

		vec3 ray;
		const float att = calcAttenuationFactor(fragPosVspace, 
			slights[lightId].light, ray);

		const float lambert = calcLambertTerm(normal, ray);

		const float spot = calcSpotFactor(slights[lightId], fragPosVspace);

		const float midFactor = att * lambert * spot;

		//if(midFactor > 0.0)
		{

			float shadow = calcShadowFactor(slights[lightId], fragPosVspace,
				shadowMaps[i]);

			const vec3 col = calcPhong(fragPosVspace, diffuseAndSpec.rgb, 
				specularAll, normal, slights[lightId].light, ray);

			fColor += col * (midFactor * shadow);
		}
	}

#if 0
	if(tiles[vInstanceId].lightsCount[2] > 0)
	{
		uint lightId = 0;

		vec3 pureColor = doPhong(fragPosVspace, normal, diffuseAndSpec.rgb, 
			specularAll, slights[lightId].light);

		float spotFactor = calcSpotFactor(slights[lightId], fragPosVspace);

		fColor += pureColor * spotFactor;
	}
#endif

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
	if(tiles[vInstanceId].lightsCount[0] > 0)
	{
		fColor += vec3(0.0, 0.1, 0.0);
	}
#endif
}

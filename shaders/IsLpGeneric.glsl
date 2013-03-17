/// @file
///
/// Illumination stage lighting pass general shader program

#pragma anki start vertexShader

#pragma anki include "shaders/IsLpVertex.glsl"

#pragma anki start fragmentShader

#pragma anki include "shaders/Pack.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

#define ATTENUATION_FINE 0

#define ATTENUATION_BOOST (0.05)

/// @name Uniforms
/// @{

// Common uniforms between lights
layout(std140, row_major, binding = 0) uniform commonBlock
{
	/// Packs:
	/// - xy: Planes. For the calculation of frag pos in view space
	uniform vec4 planes_;

	uniform vec4 sceneAmbientColor;

	uniform vec4 groundLightDir;
};

#define planes planes_.xy

struct Light
{
	vec4 posAndRadius; ///< xyz: Light pos in eye space. w: The radius
	vec4 diffuseColorShadowmapId;
	vec4 specularColor;
};

struct SpotLight
{
	Light light;
	vec4 lightDirection;
	vec4 outerCosInnerCos;
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

//uniform sampler2D lightTextures[MAX_SPOT_LIGHTS];
uniform sampler2DArrayShadow shadowMapArr;
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
flat in int vInstanceId;
in vec2 vLimitsOfNearPlaneOpt;
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
	fragPosVspace.z = planes.y / (planes.x + depth);
	fragPosVspace.xy = vLimitsOfNearPlaneOpt * fragPosVspace.z;
	fragPosVspace.z = -fragPosVspace.z;

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
	vec3 difCol = diffuse * light.diffuseColorShadowmapId.rgb;

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
		light.outerCosInnerCos.x, 
		light.outerCosInnerCos.y, 
		costheta);

	return spotFactor;
}

//==============================================================================
#if PCF == 1
float pcfLow(in sampler2DArrayShadow shadowMap, in vec3 shadowUv)
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
#endif

//==============================================================================
float calcShadowFactor(in SpotLight light, in vec3 fragPosVspace, 
	in sampler2DArrayShadow shadowMapArr, in float layer)
{
	vec4 texCoords4 = light.texProjectionMat * vec4(fragPosVspace, 1.0);
	vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

#if PCF == 1
	float shadowFactor = pcfLow(shadowMapArr, texCoords3);
#else
	float shadowFactor = texture(shadowMapArr, 
		vec4(texCoords3.x, texCoords3.y, layer, texCoords3.z));
#endif

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
	for(uint i = 0U; i < pointLightsCount; ++i)
	{
		uint lightId = tiles[vInstanceId].lightIndices[i / 4U][i % 4U];

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
		uint lightId = tiles[vInstanceId].lightIndices[i / 4U][i % 4U];

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

	for(uint i = 0U; i < spotLightsShadowCount; ++i)
	{
		uint id = i + opt;
		uint lightId = tiles[vInstanceId].lightIndices[id / 4U][id % 4U];

		vec3 ray;
		const float att = calcAttenuationFactor(fragPosVspace, 
			slights[lightId].light, ray);

		const float lambert = calcLambertTerm(normal, ray);

		const float spot = calcSpotFactor(slights[lightId], fragPosVspace);

		const float midFactor = att * lambert * spot;

		//if(midFactor > 0.0)
		{
			const float shadowmapLayerId = 
				slights[lightId].light.diffuseColorShadowmapId.w;

			const float shadow = calcShadowFactor(slights[lightId], 
				fragPosVspace, shadowMapArr, shadowmapLayerId);

			const vec3 col = calcPhong(fragPosVspace, diffuseAndSpec.rgb, 
				specularAll, normal, slights[lightId].light, ray);

			fColor += col * (midFactor * shadow);
		}
	}


#if GROUND_LIGHT
	fColor *= dot(normal, groundLightDir.xyz) + 1.0;
#endif

#if 0
	if(tiles[vInstanceId].lightsCount[3] > 0)
	{
		fColor += vec3(0.0, 0.2, 0.0);
	}

	if(tiles[vInstanceId].lightsCount[0] > 0)
	{
		fColor += vec3(0.2, 0.0, 0.0);
	}
#endif

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
	fColor += vec3(slights[0].light.diffuseColorShadowmapId.w,
		slights[1].light.diffuseColorShadowmapId.w, 0.0);
#endif

#if 0
	if(tiles[vInstanceId].lightsCount[0] > 0)
	{
		fColor += vec3(0.0, 0.1, 0.0);
	}
#endif

#if 0
	vec3 tmpc = vec3((vInstanceId % 4) / 3.0, (vInstanceId % 3) / 2.0, 
		(vInstanceId % 2));
	fColor += tmpc / 40.0;
#endif
}

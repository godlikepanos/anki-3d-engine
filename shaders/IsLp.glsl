/// @file
///
/// Illumination stage lighting pass shader program

#pragma anki start vertexShader

#if !TILES_X_COUNT || !TILES_Y_COUNT
#	error See file
#endif

layout(location = 0) in vec2 position;

uniform vec4 limitsOfNearPlane;

out vec2 vTexCoords;
flat out int vInstanceId;
out vec2 vLimitsOfNearPlaneOpt;

void main()
{
	float instIdF = float(gl_InstanceID);

	vec2 ij = vec2(
		mod(instIdF, float(TILES_X_COUNT)), 
		floor(instIdF / float(TILES_X_COUNT)));

	vInstanceId = int(gl_InstanceID);

	const vec2 SIZES = 
		vec2(1.0 / float(TILES_X_COUNT), 1.0 / float(TILES_Y_COUNT));

	vTexCoords = (position + ij) * SIZES;
	vec2 vertPosNdc = vTexCoords * 2.0 - 1.0;
	gl_Position = vec4(vertPosNdc, 0.0, 1.0);

	vLimitsOfNearPlaneOpt = 
		(vTexCoords * limitsOfNearPlane.zw) - limitsOfNearPlane.xy;
}

#pragma anki start fragmentShader
#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/Pack.glsl"
#pragma anki include "shaders/LinearDepth.glsl"
#pragma anki include "shaders/IsCommon.glsl"

#define ATTENUATION_FINE 0

#define ATTENUATION_BOOST (0.05)

/// @name Uniforms
/// @{

// Common uniforms between lights
layout(std140, row_major) uniform commonBlock
{
	/// Packs:
	/// - xy: Planes. For the calculation of frag pos in view space
	uniform vec4 planes_;

	uniform vec4 sceneAmbientColor;

	uniform vec4 groundLightDir;
};

#define planes planes_.xy

layout(std140) uniform pointLightsBlock
{
	Light pointLights[MAX_POINT_LIGHTS];
};

layout(std140) uniform spotLightsBlock
{
	SpotLight spotLights[MAX_SPOT_LIGHTS];
};

layout(std140) uniform spotTexLightsBlock
{
	SpotTexLight spotTexLights[MAX_SPOT_TEX_LIGHTS];
};

#if __VERSION__ > 430
layout(std430) 
#else
layout(std140) 
#endif
	uniform tilesBlock
{
	Tile tiles[TILES_COUNT];
};

#if USE_MRT
uniform sampler2D msFai0;
uniform sampler2D msFai1;
#else
uniform highp usampler2D msFai0;
#endif
uniform sampler2D msDepthFai;

//uniform sampler2D lightTextures[MAX_SPOT_LIGHTS];
uniform mediump sampler2DArrayShadow shadowMapArr;
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
	float depth = textureFai(msDepthFai, vTexCoords).r;

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
	out vec3 frag2LightVec)
{
	// get the vector from the frag to the light
	frag2LightVec = light.posRadius.xyz - fragPosVspace;

	float fragLightDist = length(frag2LightVec);
	frag2LightVec = normalize(frag2LightVec);

	float att = max(  
		(fragLightDist * light.posRadius.w) + (1.0 + ATTENUATION_BOOST), 
		0.0);

	return att;
}

//==============================================================================
float calcLambertTerm(in vec3 normal, in vec3 frag2LightVec)
{
	return max(0.0, dot(normal, frag2LightVec));
}

//==============================================================================
/// Performs phong lighting using the MS FAIs and a few other things
vec3 calcPhong(in vec3 fragPosVspace, in vec3 diffuse, 
	in float specColor, in float specPower, in vec3 normal, in Light light, 
	in vec3 rayDir)
{
	// Specular
	vec3 eyeVec = normalize(fragPosVspace);
	vec3 h = normalize(rayDir - eyeVec);
	float specIntensity = pow(max(0.0, dot(normal, h)), specPower);
	vec3 specCol = 
		light.specularColorTexId.rgb * (specIntensity * specColor);
	
	// Do a mad optimization
	// diffCol = diffuse * light.diffuseColorShadowmapId.rgb
	return (diffuse * light.diffuseColorShadowmapId.rgb) + specCol;
}

//==============================================================================
float calcSpotFactor(in SpotLight light, in vec3 frag2LightVec)
{
	float costheta = -dot(frag2LightVec, light.lightDir.xyz);
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
float calcShadowFactor(in SpotTexLight light, in vec3 fragPosVspace, 
	in mediump sampler2DArrayShadow shadowMapArr, in float layer)
{
	vec4 texCoords4 = light.texProjectionMat * vec4(fragPosVspace, 1.0);
	vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

#if PCF == 1
	float shadowFactor = pcfLow(shadowMapArr, texCoords3);
#else
	vec4 tcoord = vec4(texCoords3.x, texCoords3.y, layer, texCoords3.z);
	float shadowFactor = texture(shadowMapArr, tcoord);
#endif

	return shadowFactor;
}

//==============================================================================
void main()
{
	// get frag pos in view space
	vec3 fragPosVspace = getFragPosVSpace();

	// Decode GBuffer
	vec3 normal;
	vec3 diffColor;
	float specColor;
	float specPower;

	readGBuffer(
		msFai0,
#if USE_MRT
		msFai1,
#endif
		vTexCoords, diffColor, normal, specColor, specPower);

	// Ambient color
	fColor = diffColor * sceneAmbientColor.rgb;

	//Tile tile = tiles[vInstanceId];
	#define tile tiles[vInstanceId]

	// Point lights
	uint pointLightsCount = tile.lightsCount[0];
	for(uint i = 0U; i < pointLightsCount; ++i)
	{
#if __VERSION__ > 430
		uint lightId = tile.pointLightIndices[i];
#else
		uint lightId = tile.pointLightIndices[i / 4U][i % 4U];
#endif
		PointLight light = pointLights[lightId];

		vec3 ray;
		float att = calcAttenuationFactor(fragPosVspace, light, ray);

		float lambert = calcLambertTerm(normal, ray);

		fColor += calcPhong(fragPosVspace, diffColor, 
			specColor, specPower, normal, light, ray) * (att * lambert);
	}

	// Spot lights
	uint spotLightsCount = tile.lightsCount[2];

	for(uint i = 0U; i < spotLightsCount; ++i)
	{
#if __VERSION__ > 430
		uint lightId = tile.spotLightIndices[i];
#else
		uint lightId = tile.spotLightIndices[i / 4U][i % 4U];
#endif
		SpotLight light = spotLights[lightId];

		vec3 ray;
		float att = calcAttenuationFactor(fragPosVspace, 
			light.lightBase, ray);

		float lambert = calcLambertTerm(normal, ray);

		float spot = calcSpotFactor(light, ray);

		vec3 col = calcPhong(fragPosVspace, diffColor, 
			specColor, specPower, normal, light.lightBase, ray);

		fColor += col * (att * lambert * spot);
	}

	// Spot lights with shadow
	uint spotTexLightsCount = tile.lightsCount[3];

	for(uint i = 0U; i < spotTexLightsCount; ++i)
	{
#if __VERSION__ > 430
		uint lightId = tile.spotTexLightIndices[i];
#else
		uint lightId = tile.spotTexLightIndices[i / 4U][i % 4U];
#endif
		SpotTexLight light = spotTexLights[lightId];

		vec3 ray;
		float att = calcAttenuationFactor(fragPosVspace, 
			light.spotLightBase.lightBase, ray);

		float lambert = calcLambertTerm(normal, ray);

		float spot = 
			calcSpotFactor(light.spotLightBase, ray);

		float midFactor = att * lambert * spot;

		//if(midFactor > 0.0)
		{
			float shadowmapLayerId = 
				light.spotLightBase.lightBase.diffuseColorShadowmapId.w;

			float shadow = calcShadowFactor(light, 
				fragPosVspace, shadowMapArr, shadowmapLayerId);

			vec3 col = calcPhong(fragPosVspace, diffColor, 
				specColor, specPower, normal, light.spotLightBase.lightBase, 
				ray);

			fColor += col * (midFactor * shadow);
		}
	}

#if GROUND_LIGHT
	fColor += max(dot(normal, groundLightDir.xyz), 0.0) 
		* vec3(0.05, 0.05, 0.01);
#endif

#if 0
	if(vInstanceId != 99999)
	{
		fColor = vec3(diffColor);
	}
#endif

#if 0
	if(tiles[vInstanceId].lightsCount[0] > 0)
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
	fColor = fColor * 0.0001 
		+ vec3(uintBitsToFloat(tiles[vInstanceId].lightsCount[1]));
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

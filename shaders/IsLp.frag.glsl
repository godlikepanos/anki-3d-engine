// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag
#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/Pack.glsl"
#pragma anki include "shaders/LinearDepth.glsl"
#pragma anki include "shaders/IsCommon.glsl"

#define ATTENUATION_FINE 0

#define ATTENUATION_BOOST (0.05)

#define SUBSURFACE_COLOR (0.1)

layout(std140, binding = 1) readonly buffer pointLightsBlock
{
	PointLight uPointLights[MAX_POINT_LIGHTS];
};

layout(std140, binding = 2) readonly buffer spotLightsBlock
{
	SpotLight uSpotLights[MAX_SPOT_LIGHTS];
};

layout(std140, binding = 3) readonly buffer spotTexLightsBlock
{
	SpotLight uSpotTexLights[MAX_SPOT_TEX_LIGHTS];
};

layout(std430, binding = 4) readonly buffer tilesBlock
{
	Tile uTiles[TILES_COUNT];
};

layout(binding = 0) uniform sampler2D uMsRt0;
layout(binding = 1) uniform sampler2D uMsRt1;
layout(binding = 2) uniform sampler2D uMsDepthRt;

layout(binding = 3) uniform highp sampler2DArrayShadow uShadowMapArr;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) flat in int inInstanceId;
layout(location = 2) in vec2 inProjectionParams;

layout(location = 0) out vec3 outColor;

//==============================================================================
// Return frag pos in view space
vec3 getFragPosVSpace()
{
	float depth = textureRt(uMsDepthRt, inTexCoord).r;

	vec3 fragPos;
	fragPos.z = uProjectionParams.z / (uProjectionParams.w + depth);
	fragPos.xy = inProjectionParams * fragPos.z;

	return fragPos;
}

//==============================================================================
float computeAttenuationFactor(
	in float lightRadius,
	in vec3 frag2Light)
{
	float fragLightDist = length(frag2Light);

	float att = (fragLightDist * lightRadius) + (1.0 + ATTENUATION_BOOST);
	att = max(0.0, att);

	return att;
}

//==============================================================================
float computeLambertTerm(in vec3 normal, in vec3 frag2LightDir)
{
	return max(SUBSURFACE_COLOR, dot(normal, frag2LightDir));
}

//==============================================================================
// Performs phong lighting using the MS FAIs and a few other things
vec3 computeSpecularColor(
	in vec3 viewDir,
	in vec3 frag2LightDir,
	in vec3 normal,
	in float specCol, 
	in float specPower, 
	in vec3 lightSpecCol)
{
	vec3 h = normalize(frag2LightDir + viewDir);
	float specIntensity = pow(max(0.0, dot(normal, h)), specPower);
	vec3 outSpecCol = lightSpecCol * (specIntensity * specCol);
	
	return outSpecCol;
}

//==============================================================================
vec3 computeDiffuseColor(in vec3 diffCol, in vec3 lightDiffCol)
{
	return diffCol * lightDiffCol;
}

//==============================================================================
float computeSpotFactor(
	in vec3 frag2LightDir,
	in float outerCos,
	in float innerCos,
	in vec3 spotDir)
{
	float costheta = -dot(frag2LightDir, spotDir);
	float spotFactor = smoothstep(outerCos, innerCos, costheta);
	return spotFactor;
}

//==============================================================================
float computeShadowFactor(
	in mat4 lightProjectionMat, 
	in vec3 fragPos, 
	in highp sampler2DArrayShadow shadowMapArr, 
	in float layer)
{
	vec4 texCoords4 = lightProjectionMat * vec4(fragPos, 1.0);
	vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

#if POISSON == 1
	const vec2 poissonDisk[4] = vec2[](
		vec2(-0.94201624, -0.39906216),
		vec2(0.94558609, -0.76890725),
		vec2(-0.094184101, -0.92938870),
		vec2(0.34495938, 0.29387760));

	float shadowFactor = 0.0;

	vec2 cordpart0 = vec2(layer, texCoords3.z);

	for(int i = 0; i < 4; i++)
	{
		vec2 cordpart1 = texCoords3.xy + poissonDisk[i] / (300.0);
		vec4 tcoord = vec4(cordpart1, cordpart0);
		
		shadowFactor += texture(uShadowMapArr, tcoord);
	}

	return shadowFactor / 4.0;
#else
	vec4 tcoord = vec4(texCoords3.x, texCoords3.y, layer, texCoords3.z);
	float shadowFactor = texture(shadowMapArr, tcoord);

	return shadowFactor;
#endif
}

//==============================================================================
void main()
{
	// get frag pos in view space
	vec3 fragPos = getFragPosVSpace();
	vec3 viewDir = normalize(-fragPos);

	// Decode GBuffer
	vec3 normal;
	vec3 diffCol;
	float specCol;
	float specPower;

	readGBuffer(
		uMsRt0, uMsRt1, inTexCoord, diffCol, normal, specCol, specPower);

	specPower *= 128.0;

	// Ambient color
	outColor = diffCol * uSceneAmbientColor.rgb;

	//Tile tile = uTiles[inInstanceId];
	#define tile uTiles[inInstanceId]

	// Point lights
	uint pointLightsCount = tile.lightsCount[0];
	for(uint i = 0U; i < pointLightsCount; ++i)
	{
		uint lightId = tile.pointLightIndices[i];
		PointLight light = uPointLights[lightId];

		vec3 frag2Light = light.posRadius.xyz - fragPos;
		float att = computeAttenuationFactor(light.posRadius.w, frag2Light);

		vec3 frag2LightDir = normalize(frag2Light);
		float lambert = computeLambertTerm(normal, frag2LightDir);

		vec3 specC = computeSpecularColor(viewDir, frag2LightDir, normal,
			specCol, specPower, light.specularColorTexId.rgb);

		vec3 diffC = computeDiffuseColor(
			diffCol, light.diffuseColorShadowmapId.rgb);

		outColor += (specC + diffC) * (att * lambert);
	}

	// Spot lights
	uint spotLightsCount = tile.lightsCount[2];
	for(uint i = 0U; i < spotLightsCount; ++i)
	{
		uint lightId = tile.spotLightIndices[i];
		SpotLight slight = uSpotLights[lightId];
		Light light = slight.lightBase;

		vec3 frag2Light = light.posRadius.xyz - fragPos;
		float att = computeAttenuationFactor(light.posRadius.w, frag2Light);

		vec3 frag2LightDir = normalize(frag2Light);
		float lambert = computeLambertTerm(normal, frag2LightDir);

		vec3 specC = computeSpecularColor(viewDir, frag2LightDir, normal,
			specCol, specPower, light.specularColorTexId.rgb);

		vec3 diffC = computeDiffuseColor(
			diffCol, light.diffuseColorShadowmapId.rgb);

		float spot = computeSpotFactor(
			frag2LightDir, slight.outerCosInnerCos.x, 
			slight.outerCosInnerCos.y, 
			slight.lightDir.xyz);

		outColor += (diffC + specC) * (att * lambert * spot);
	}

	// Spot lights with shadow
	uint spotTexLightsCount = tile.lightsCount[3];
	for(uint i = 0U; i < spotTexLightsCount; ++i)
	{
		uint lightId = tile.spotTexLightIndices[i];
		SpotLight slight = uSpotTexLights[lightId];
		Light light = slight.lightBase;

		vec3 frag2Light = light.posRadius.xyz - fragPos;
		float att = computeAttenuationFactor(light.posRadius.w, frag2Light);

		vec3 frag2LightDir = normalize(frag2Light);
		float lambert = computeLambertTerm(normal, frag2LightDir);

		vec3 specC = computeSpecularColor(viewDir, frag2LightDir, normal,
			specCol, specPower, light.specularColorTexId.rgb);

		vec3 diffC = computeDiffuseColor(
			diffCol, light.diffuseColorShadowmapId.rgb);

		float spot = computeSpotFactor(
			frag2LightDir, slight.outerCosInnerCos.x, 
			slight.outerCosInnerCos.y, 
			slight.lightDir.xyz);

		float shadowmapLayerId = light.diffuseColorShadowmapId.w;
		float shadow = computeShadowFactor(slight.texProjectionMat, 
			fragPos, uShadowMapArr, shadowmapLayerId);

		outColor += (diffC + specC) * (att * lambert * spot * shadow);
	}

#if GROUND_LIGHT
	outColor += max(dot(normal, uGroundLightDir.xyz), 0.0) 
		* vec3(0.10, 0.01, 0.01);
#endif

#if 0
	if(inInstanceId != 99999)
	{
		outColor = vec3(specPower / 120.0);
	}
#endif
}

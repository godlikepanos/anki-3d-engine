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

#define LAMBERT_MIN (0.1)

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
	SpotTexLight uSpotTexLights[MAX_SPOT_TEX_LIGHTS];
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

	vec3 fragPosVspace;
	fragPosVspace.z = uProjectionParams.z / (uProjectionParams.w + depth);
	fragPosVspace.xy = inProjectionParams * fragPosVspace.z;

	return fragPosVspace;
}

//==============================================================================
float computeAttenuationFactor(
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
float computeLambertTerm(in vec3 normal, in vec3 frag2LightVec)
{
	return max(LAMBERT_MIN, dot(normal, frag2LightVec));
}

//==============================================================================
// Performs phong lighting using the MS FAIs and a few other things
vec3 computePhong(in vec3 fragPosVspace, in vec3 diffuse, 
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
float computeSpotFactor(in SpotLight light, in vec3 frag2LightVec)
{
	float costheta = -dot(frag2LightVec, light.lightDir.xyz);
	float spotFactor = smoothstep(
		light.outerCosInnerCos.x, 
		light.outerCosInnerCos.y, 
		costheta);

	return spotFactor;
}

//==============================================================================
float computeShadowFactor(in SpotTexLight light, in vec3 fragPosVspace, 
	in highp sampler2DArrayShadow shadowMapArr, in float layer)
{
	vec4 texCoords4 = light.texProjectionMat * vec4(fragPosVspace, 1.0);
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
	vec3 fragPosVspace = getFragPosVSpace();

	// Decode GBuffer
	vec3 normal;
	vec3 diffColor;
	float specColor;
	float specPower;

	readGBuffer(uMsRt0, uMsRt1,
		inTexCoord, diffColor, normal, specColor, specPower);

	// Ambient color
	outColor = diffColor * uSceneAmbientColor.rgb;

	//Tile tile = uTiles[inInstanceId];
	#define tile uTiles[inInstanceId]

	// Point lights
	uint pointLightsCount = tile.lightsCount[0];
	for(uint i = 0U; i < pointLightsCount; ++i)
	{
		uint lightId = tile.pointLightIndices[i];
		PointLight light = uPointLights[lightId];

		vec3 ray;
		float att = computeAttenuationFactor(fragPosVspace, light, ray);

		float lambert = computeLambertTerm(normal, ray);

		outColor += computePhong(fragPosVspace, diffColor, 
			specColor, specPower, normal, light, ray) * (att * lambert);
	}

	// Spot lights
	uint spotLightsCount = tile.lightsCount[2];

	for(uint i = 0U; i < spotLightsCount; ++i)
	{
		uint lightId = tile.spotLightIndices[i];
		SpotLight light = uSpotLights[lightId];

		vec3 ray;
		float att = computeAttenuationFactor(fragPosVspace, 
			light.lightBase, ray);

		float lambert = computeLambertTerm(normal, ray);

		float spot = computeSpotFactor(light, ray);

		vec3 col = computePhong(fragPosVspace, diffColor, 
			specColor, specPower, normal, light.lightBase, ray);

		outColor += col * (att * lambert * spot);
	}

	// Spot lights with shadow
	uint spotTexLightsCount = tile.lightsCount[3];

	for(uint i = 0U; i < spotTexLightsCount; ++i)
	{
		uint lightId = tile.spotTexLightIndices[i];
		SpotTexLight light = uSpotTexLights[lightId];

		vec3 ray;
		float att = computeAttenuationFactor(fragPosVspace, 
			light.spotLightBase.lightBase, ray);

		float lambert = computeLambertTerm(normal, ray);

		float spot = 
			computeSpotFactor(light.spotLightBase, ray);

		float midFactor = att * lambert * spot;

		//if(midFactor > 0.0)
		{
			float shadowmapLayerId = 
				light.spotLightBase.lightBase.diffuseColorShadowmapId.w;

			float shadow = computeShadowFactor(light, 
				fragPosVspace, uShadowMapArr, shadowmapLayerId);

			vec3 col = computePhong(fragPosVspace, diffColor, 
				specColor, specPower, normal, light.spotLightBase.lightBase, 
				ray);

			outColor += col * (midFactor * shadow);
		}
	}

#if GROUND_LIGHT
	outColor += max(dot(normal, uGroundLightDir.xyz), 0.0) 
		* vec3(0.05, 0.05, 0.01);
#endif

#if 0
	if(inInstanceId != 99999)
	{
		outColor = vec3(diffColor);
	}
#endif

#if 0
	if(spotTexLightsCount > 0)
	{
		outColor += vec3(0.1);
	}
#endif

#if 0
	outColor += vec3(slights[0].light.diffuseColorShadowmapId.w,
		slights[1].light.diffuseColorShadowmapId.w, 0.0);
#endif

#if 0
	outColor = outColor * 0.0001 
		+ vec3(uintBitsToFloat(tiles[inInstanceId].lightsCount[1]));
#endif

#if 0
	if(tiles[inInstanceId].lightsCount[0] > 0)
	{
		outColor += vec3(0.0, 0.1, 0.0);
	}
#endif

#if 0
	vec3 tmpc = vec3((inInstanceId % 4) / 3.0, (inInstanceId % 3) / 2.0, 
		(inInstanceId % 2));
	outColor += tmpc / 40.0;
#endif
}

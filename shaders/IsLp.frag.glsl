// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag
#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/Pack.glsl"

#define PI 3.1415926535

#define ATTENUATION_FINE 0
#define ATTENUATION_BOOST (0.05)

#ifndef BRDF
#	define BRDF 1
#endif

// Representation of a tile
struct Tile
{
	uvec4 lightsCount;
	uint pointLightIndices[MAX_POINT_LIGHTS_PER_TILE];
	uint spotLightIndices[MAX_SPOT_LIGHTS_PER_TILE];
	uint spotTexLightIndices[MAX_SPOT_TEX_LIGHTS_PER_TILE];
};

// The base of all lights
struct Light
{
	vec4 posRadius; // xyz: Light pos in eye space. w: The -1/radius
	vec4 diffuseColorShadowmapId; // xyz: diff color, w: shadowmap tex ID
	vec4 specularColorTexId; // xyz: spec color, w: diffuse tex ID
};

// Point light
#define PointLight Light

// Spot light
struct SpotLight
{
	Light lightBase;
	vec4 lightDir;
	vec4 outerCosInnerCos;
	vec4 extendPoints[4]; // The positions of the 4 camera points
	mat4 texProjectionMat;
};

// A container of many lights
struct Lights
{
	uvec4 count; // x: points, z: 
	PointLight pointLights[MAX_POINT_LIGHTS];
	SpotLight spotLights[MAX_SPOT_LIGHTS];
	SpotLight spotTexLights[MAX_SPOT_TEX_LIGHTS];
};

// Common uniforms between lights
layout(std140, row_major, binding = 0) uniform _commonBlock
{
	vec4 u_projectionParams;
	vec4 u_sceneAmbientColor;
	vec4 u_groundLightDir;
};

layout(std140, binding = 1) readonly buffer pointLightsBlock
{
	PointLight u_pointLights[MAX_POINT_LIGHTS];
};

layout(std140, binding = 2) readonly buffer spotLightsBlock
{
	SpotLight u_spotLights[MAX_SPOT_LIGHTS];
};

layout(std140, binding = 3) readonly buffer spotTexLightsBlock
{
	SpotLight u_spotTexLights[MAX_SPOT_TEX_LIGHTS];
};

layout(std430, binding = 4) readonly buffer tilesBlock
{
	Tile u_tiles[TILES_COUNT];
};

layout(binding = 0) uniform sampler2D u_msRt0;
layout(binding = 1) uniform sampler2D u_msRt1;
layout(binding = 2) uniform sampler2D u_msDepthRtg;

layout(binding = 3) uniform highp sampler2DArrayShadow u_shadowMapArr;

layout(location = 0) in vec2 in_texCoord;
layout(location = 1) flat in int in_instanceId;
layout(location = 2) in vec2 in_projectionParams;

layout(location = 0) out vec3 out_color;

//==============================================================================
// Return frag pos in view space
vec3 getFragPosVSpace()
{
	float depth = textureRt(u_msDepthRtg, in_texCoord).r;

	vec3 fragPos;
	fragPos.z = u_projectionParams.z / (u_projectionParams.w + depth);
	fragPos.xy = in_projectionParams * fragPos.z;

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
// Performs phong specular lighting
vec3 computeSpecularColor(
	in vec3 v,
	in vec3 l,
	in vec3 n,
	in float specCol, 
	in float specPower, 
	in vec3 lightSpecCol)
{
	vec3 h = normalize(l + v);
	float specIntensity = pow(max(0.0, dot(n, h)), specPower);
	vec3 outSpecCol = lightSpecCol * (specIntensity * specCol);
	
	return outSpecCol;
}

//==============================================================================
// Performs BRDF specular lighting
vec3 computeSpecularColorBrdf(
	in vec3 v,
	in vec3 l,
	in vec3 n,
	in float specCol, 
	in float specPower, 
	in vec3 lightSpecCol,
	in float a2, // rougness^2
	in float nol) // N dot L
{
	vec3 h = normalize(l + v);

	// Fresnel
	float loh = max(0.0, dot(l, h));
	float f = specCol + (1.0 - specCol) * pow((1.0 - loh), 5.0);
	//float f = specColor + (1.0 - specColor) 
	//	* pow(2.0, (-5.55473 * loh - 6.98316) * loh);

	// NDF: GGX Trowbridge-Reitz
	float noh = max(0.0, dot(n, h));
	float d = a2 / (PI * pow(noh * noh * (a2 - 1.0) + 1.0, 2.0));

	// Visibility term: Geometric shadowing devided by BRDF denominator
	float nov = max(0.0001, dot(n, v));
	float vv = nov + sqrt((nov - nov * a2) * nov + a2);
	float vl = nol + sqrt((nol - nol * a2) * nol + a2);
	float vis = 1.0 / (vv * vl);

	return (vis * d * f) * lightSpecCol;
}

//==============================================================================
vec3 computeDiffuseColor(in vec3 diffCol, in vec3 lightDiffCol)
{
	return diffCol * lightDiffCol;
}

//==============================================================================
float computeSpotFactor(
	in vec3 l,
	in float outerCos,
	in float innerCos,
	in vec3 spotDir)
{
	float costheta = -dot(l, spotDir);
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
		
		shadowFactor += texture(u_shadowMapArr, tcoord);
	}

	return shadowFactor / 4.0;
#else
	vec4 tcoord = vec4(texCoords3.x, texCoords3.y, layer, texCoords3.z);
	float shadowFactor = texture(shadowMapArr, tcoord);

	return shadowFactor;
#endif
}

//==============================================================================
// Common code for lighting

#define LIGHTING_COMMON_PHONG() \
	vec3 frag2Light = light.posRadius.xyz - fragPos; \
	vec3 l = normalize(frag2Light); \
	float nol = max(0.0, dot(normal, l)); \
	vec3 specC = computeSpecularColor( \
		viewDir, l, normal, specCol, specPower, light.specularColorTexId.rgb); \
	vec3 diffC = computeDiffuseColor( \
		diffCol, light.diffuseColorShadowmapId.rgb); \
	float att = computeAttenuationFactor(light.posRadius.w, frag2Light); \
	float lambert = nol;

#define LIGHTING_COMMON_BRDF() \
	vec3 frag2Light = light.posRadius.xyz - fragPos; \
	vec3 l = normalize(frag2Light); \
	float nol = max(0.0, dot(normal, l)); \
	vec3 specC = computeSpecularColorBrdf(viewDir, l, normal, specCol, \
		specPower, light.specularColorTexId.rgb, a2, nol); \
	vec3 diffC = computeDiffuseColor( \
		diffCol, light.diffuseColorShadowmapId.rgb); \
	float att = computeAttenuationFactor(light.posRadius.w, frag2Light); \
	float lambert = nol;

#if BRDF
#	define LIGHTING_COMMON LIGHTING_COMMON_BRDF
#else
#	define LIGHTING_COMMON LIGHTING_COMMON_PHONG
#endif

//==============================================================================
void main()
{
	// Get frag pos in view space
	vec3 fragPos = getFragPosVSpace();
	vec3 viewDir = normalize(-fragPos);

	// Decode GBuffer
	vec3 normal;
	vec3 diffCol;
	float specCol;
	float specPower;
	const float subsurface = 0.0;

	readGBuffer(
		u_msRt0, u_msRt1, in_texCoord, diffCol, normal, specCol, specPower);

#if BRDF
	float a2 = pow(max(0.0001, specPower), 4.0);
#else
	specPower = max(0.0001, specPower) * 128.0;
#endif

	// Ambient color
	out_color = diffCol * u_sceneAmbientColor.rgb;

	//Tile tile = u_tiles[in_instanceId];
	#define tile u_tiles[in_instanceId]

	// Point lights
	uint pointLightsCount = tile.lightsCount[0];
	for(uint i = 0U; i < pointLightsCount; ++i)
	{
		uint lightId = tile.pointLightIndices[i];
		PointLight light = u_pointLights[lightId];

		LIGHTING_COMMON();

		out_color += (specC + diffC) * (att * max(subsurface, lambert));
	}

	// Spot lights
	uint spotLightsCount = tile.lightsCount[2];
	for(uint i = 0U; i < spotLightsCount; ++i)
	{
		uint lightId = tile.spotLightIndices[i];
		SpotLight slight = u_spotLights[lightId];
		Light light = slight.lightBase;

		LIGHTING_COMMON();

		float spot = computeSpotFactor(
			l, slight.outerCosInnerCos.x, 
			slight.outerCosInnerCos.y, 
			slight.lightDir.xyz);

		out_color += (diffC + specC) * (att * max(subsurface, lambert) * spot);
	}

	// Spot lights with shadow
	uint spotTexLightsCount = tile.lightsCount[3];
	for(uint i = 0U; i < spotTexLightsCount; ++i)
	{
		uint lightId = tile.spotTexLightIndices[i];
		SpotLight slight = u_spotTexLights[lightId];
		Light light = slight.lightBase;

		LIGHTING_COMMON();

		float spot = computeSpotFactor(
			l, slight.outerCosInnerCos.x, 
			slight.outerCosInnerCos.y, 
			slight.lightDir.xyz);

		float shadowmapLayerId = light.diffuseColorShadowmapId.w;
		float shadow = computeShadowFactor(slight.texProjectionMat, 
			fragPos, u_shadowMapArr, shadowmapLayerId);

		out_color += (diffC + specC) 
			* (att * spot * max(subsurface, lambert * shadow));
	}

#if GROUND_LIGHT
	out_color += max(dot(normal, u_groundLightDir.xyz), 0.0) 
		* vec3(0.01, 0.001, 0.001);
#endif

#if 0
	if(pointLightsCount != 0)
	{
		out_color = vec3(0.05 * float(pointLightsCount));
	}
#endif
}

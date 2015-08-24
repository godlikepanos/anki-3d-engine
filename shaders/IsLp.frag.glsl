// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag
#pragma anki include "shaders/IsCommon.glsl"
#pragma anki include "shaders/Pack.glsl"

#define PI 3.1415926535

#define ATTENUATION_FINE 0
#define ATTENUATION_BOOST (0.1)

#ifndef BRDF
#	define BRDF 1
#endif

// Representation of a tile
struct Tile
{
	uvec4 offsetCounts; // x:offset y:points_count z:spots_count w:stex_count
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
	mat4 texProjectionMat;
};

layout(std140, binding = 0) readonly buffer _s0
{
	PointLight u_pointLights[MAX_POINT_LIGHTS];
};

layout(std140, binding = 1) readonly buffer _s1
{
	SpotLight u_spotLights[MAX_SPOT_LIGHTS];
};

layout(std140, binding = 2) readonly buffer _s2
{
	SpotLight u_spotTexLights[MAX_SPOT_TEX_LIGHTS];
};

layout(std140, binding = 3) readonly buffer _s3
{
	Tile u_tiles[TILES_COUNT];
};

layout(std430, binding = 4) readonly buffer _s5
{
	uint u_lightIndices[MAX_LIGHT_INDICES];
};

layout(binding = 0) uniform sampler2D u_msRt0;
layout(binding = 1) uniform sampler2D u_msRt1;
layout(binding = 2) uniform sampler2D u_msRt2;
layout(binding = 3) uniform sampler2D u_msDepthRt;
layout(binding = 4) uniform highp sampler2DArrayShadow u_spotMapArr;
layout(binding = 5) uniform highp samplerCubeArrayShadow u_omniMapArr;

layout(location = 0) in vec2 in_texCoord;
layout(location = 1) flat in int in_instanceId;
layout(location = 2) in vec2 in_projectionParams;

layout(location = 0) out vec3 out_color;

//==============================================================================
// Return frag pos in view space
vec3 getFragPosVSpace()
{
	float depth = textureRt(u_msDepthRt, in_texCoord).r;

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
	att *= att;

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
	in vec3 specCol,
	in float specPower,
	in vec3 lightSpecCol,
	in float a2, // rougness^2
	in float nol) // N dot L
{
	vec3 h = normalize(l + v);

	// Fresnel (Schlick)
	float loh = max(EPSILON, dot(l, h));
	vec3 f = specCol + (1.0 - specCol) * pow((1.0 + EPSILON - loh), 5.0);
	//float f = specColor + (1.0 - specColor)
	//	* pow(2.0, (-5.55473 * loh - 6.98316) * loh);

	// NDF: GGX Trowbridge-Reitz
	float noh = max(EPSILON, dot(n, h));
	float d = a2 / (PI * pow(noh * noh * (a2 - 1.0) + 1.0, 2.0));

	// Visibility term: Geometric shadowing devided by BRDF denominator
	float nov = max(EPSILON, dot(n, v));
	float vv = nov + sqrt((nov - nov * a2) * nov + a2);
	float vl = nol + sqrt((nol - nol * a2) * nol + a2);
	float vis = 1.0 / (vv * vl);

	return f * (vis * d) * lightSpecCol;
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

		shadowFactor += texture(u_spotMapArr, tcoord);
	}

	return shadowFactor / 4.0;
#else
	vec4 tcoord = vec4(texCoords3.x, texCoords3.y, layer, texCoords3.z);
	float shadowFactor = texture(shadowMapArr, tcoord);

	return shadowFactor;
#endif
}

//==============================================================================
float computeShadowFactorOmni(in vec3 frag2Light, in float layer,
	in float radius)
{
	vec3 dir = (u_viewMat * vec4(-frag2Light, 1.0)).xyz;
	vec3 dirabs = abs(dir);
	float dist = -max(dirabs.x, max(dirabs.y, dirabs.z));
	dir = normalize(dir);

	const float near = OMNI_LIGHT_FRUSTUM_NEAR_PLANE;
	const float far = radius;

	// Original code:
	// float g = near - far;
	// float z = (far + near) / g * dist + (2.0 * far * near) / g;
	// float w = -dist;
	// z /= w;
	// z = z * 0.5 + 0.5;
	// Optimized:
	float z = (far * (dist + near)) / (dist * (far - near));

	float shadowFactor = texture(u_omniMapArr, vec4(dir, layer), z).r;
	return shadowFactor;
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
void debugIncorrectColor(inout vec3 c)
{
	if(isnan(c.x) || isnan(c.y) || isnan(c.z)
		|| isinf(c.x) || isinf(c.y) || isinf(c.z))
	{
		c = vec3(1.0, 0.0, 1.0);
	}
}

//==============================================================================
void main()
{
	// Get frag pos in view space
	vec3 fragPos = getFragPosVSpace();
	vec3 viewDir = normalize(-fragPos);

	// Decode GBuffer
	vec3 normal;
	vec3 diffCol;
	vec3 specCol;
	float specPower;
	const float subsurface = 0.0;

	readGBuffer(
		u_msRt0, u_msRt1, u_msRt2,
		in_texCoord, diffCol, normal, specCol, specPower);

#if BRDF
	float a2 = pow(max(EPSILON, specPower), 2.5);
#else
	specPower = max(EPSILON, specPower) * 128.0;
#endif

	// Ambient color
	out_color = diffCol * u_sceneAmbientColor.rgb;

	//Tile tile = u_tiles[in_instanceId];
	#define tile u_tiles[in_instanceId]

	// Point lights
	uint lightOffset = tile.offsetCounts.x;
	uint pointLightsCount = tile.offsetCounts.y;
	for(uint i = 0U; i < pointLightsCount; ++i)
	{
		uint lightId = u_lightIndices[lightOffset++];
		PointLight light = u_pointLights[lightId];

		LIGHTING_COMMON();

		float shadow = 1.0;
		if(light.diffuseColorShadowmapId.w < 128.0)
		{
			shadow = computeShadowFactorOmni(frag2Light,
				light.diffuseColorShadowmapId.w,
				-1.0 / light.posRadius.w);
		}

		out_color += (specC + diffC)
			* (att * max(subsurface, lambert * shadow));
	}

	// Spot lights
	uint spotLightsCount = tile.offsetCounts.z;
	for(uint i = 0U; i < spotLightsCount; ++i)
	{
		uint lightId = u_lightIndices[lightOffset++];
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
	uint spotTexLightsCount = tile.offsetCounts.w;
	for(uint i = 0U; i < spotTexLightsCount; ++i)
	{
		uint lightId = u_lightIndices[lightOffset++];
		SpotLight slight = u_spotTexLights[lightId];
		Light light = slight.lightBase;

		LIGHTING_COMMON();

		float spot = computeSpotFactor(
			l, slight.outerCosInnerCos.x,
			slight.outerCosInnerCos.y,
			slight.lightDir.xyz);

		float shadowmapLayerId = light.diffuseColorShadowmapId.w;
		float shadow = computeShadowFactor(slight.texProjectionMat,
			fragPos, u_spotMapArr, shadowmapLayerId);

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
		out_color = vec3(float(pointLightsCount) * 0.05);
	}

	/*uint x = in_instanceId % 60;
	uint y = in_instanceId / 60;
	if(x == 30 && y == 20)
	{
		out_color = vec3(1.0, 0.0, 0.0);
	}*/
#endif
}

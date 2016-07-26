// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_IS_FS_COMMON_GLSL
#define ANKI_SHADERS_IS_FS_COMMON_GLSL

#include "shaders/LightFunctions.glsl"

// Common uniforms between lights
struct LightingUniforms
{
	vec4 projectionParams;
	vec4 rendererSizeTimePad1;
	vec4 nearFarClustererMagicPad1;
	mat4 viewMat;
	mat3 invViewRotation;
	uvec4 tileCountPad1;
};

// Point light
struct PointLight
{
	vec4 posRadius; // xyz: Light pos in eye space. w: The -1/radius
	vec4 diffuseColorShadowmapId; // xyz: diff color, w: shadowmap tex ID
	vec4 specularColorTexId; // xyz: spec color, w: diffuse tex ID
};

// Spot light
struct SpotLight
{
	vec4 posRadius; // xyz: Light pos in eye space. w: The -1/radius
	vec4 diffuseColorShadowmapId; // xyz: diff color, w: shadowmap tex ID
	vec4 specularColorTexId; // xyz: spec color, w: diffuse tex ID
	vec4 lightDir;
	vec4 outerCosInnerCos;
	mat4 texProjectionMat;
};

// Representation of a reflection probe
struct ReflectionProbe
{
	// Position of the prove in view space. Radius of probe squared
	vec4 positionRadiusSq;

	// Slice in u_reflectionsTex vector.
	vec4 cubemapIndexPad3;
};

layout(std140, row_major, UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING)) uniform u0_
{
	LightingUniforms u_lightingUniforms;
};

#ifdef FRAGMENT_SHADER

layout(std140, UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING + 1)) uniform u1_
{
	PointLight u_pointLights[UBO_MAX_SIZE / (3 * 4 * 4)];
};

layout(UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING + 2),
	std140,
	row_major) uniform u2_
{
	SpotLight u_spotLights[UBO_MAX_SIZE / (9 * 4 * 4)];
};

layout(std140,
	row_major,
	UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING + 3)) uniform u3_
{
	ReflectionProbe u_reflectionProbes[UBO_MAX_SIZE / (2 * 4 * 4)];
};

layout(SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 0), std430) readonly buffer s0_
{
	uint u_clusters[];
};

layout(std430, SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 1)) readonly buffer s1_
{
	uint u_lightIndices[];
};

layout(TEX_BINDING(LIGHT_SET,
	LIGHT_TEX_BINDING)) uniform highp sampler2DArrayShadow u_spotMapArr;
layout(TEX_BINDING(LIGHT_SET,
	LIGHT_TEX_BINDING + 1)) uniform highp samplerCubeArrayShadow u_omniMapArr;

layout(TEX_BINDING(LIGHT_SET,
	LIGHT_TEX_BINDING + 2)) uniform samplerCubeArray u_reflectionsTex;

layout(TEX_BINDING(
	LIGHT_SET, LIGHT_TEX_BINDING + 3)) uniform samplerCubeArray u_irradianceTex;

layout(TEX_BINDING(
	LIGHT_SET, LIGHT_TEX_BINDING + 4)) uniform sampler2D u_integrationLut;

//==============================================================================
// Get element count attached in a cluster
void getClusterInfo(in uint clusterIdx,
	out uint indexOffset,
	out uint pointLightCount,
	out uint spotLightCount,
	out uint probeCount)
{
	uint cluster = u_clusters[clusterIdx];
	indexOffset = cluster >> 16u;
	probeCount = (cluster >> 8u) & 0xFu;
	pointLightCount = (cluster >> 4u) & 0xFu;
	spotLightCount = cluster & 0xFu;
}

#endif // FRAGMENT_SHADER

#endif

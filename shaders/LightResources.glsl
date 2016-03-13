// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_LIGHT_RESOURCES_GLSL
#define ANKI_SHADERS_LIGHT_RESOURCES_GLSL

#include "shaders/Common.glsl"

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

layout(
	std140, row_major, UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING)) uniform ubo0_
{
	LightingUniforms u_lightingUniforms;
};

#ifdef FRAGMENT_SHADER

layout(std140, SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 0)) readonly buffer _s1
{
	PointLight u_pointLights[];
};

layout(SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 1),
	std140,
	row_major) readonly buffer _s2
{
	SpotLight u_spotLights[];
};

layout(SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 2), std430) readonly buffer _s3
{
	uint u_clusters[];
};

layout(std430, SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 3)) readonly buffer _s4
{
	uint u_lightIndices[];
};

layout(std140,
	row_major,
	SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 4)) readonly buffer _s5
{
	ReflectionProbe u_reflectionProbes[];
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

#endif // FRAGMENT_SHADER

#endif

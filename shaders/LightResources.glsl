// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_LIGHT_RESOURCES_GLSL
#define ANKI_SHADERS_LIGHT_RESOURCES_GLSL

#pragma anki include "shaders/Common.glsl"

// Common uniforms between lights
struct LightingUniforms
{
	vec4 projectionParams;
	vec4 sceneAmbientColor;
	vec4 rendererSizeTimePad1;
	vec4 nearFarClustererMagicPad1;
	mat4 viewMat;
	uvec4 tileCountPad1;
};

layout(std140, row_major, SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING))
	readonly buffer _s0
{
	LightingUniforms u_lightingUniforms;
};

#ifdef FRAGMENT_SHADER

// Point light
struct PointLight
{
	vec4 posRadius; // xyz: Light pos in eye space. w: The -1/radius
	vec4 diffuseColorShadowmapId; // xyz: diff color, w: shadowmap tex ID
	vec4 specularColorTexId; // xyz: spec color, w: diffuse tex ID
};

layout(std140, SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 1)) readonly buffer _s1
{
	PointLight u_pointLights[];
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

layout(SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 2), std140, row_major)
	readonly buffer _s2
{
	SpotLight u_spotLights[];
};

layout(SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 3), std430) readonly buffer _s3
{
	uint u_clusters[];
};

layout(std430, SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 4)) readonly buffer _s4
{
	uint u_lightIndices[];
};

layout(TEX_BINDING(LIGHT_SET, LIGHT_TEX_BINDING))
	uniform highp sampler2DArrayShadow u_spotMapArr;
layout(TEX_BINDING(LIGHT_SET, LIGHT_TEX_BINDING + 1))
	uniform highp samplerCubeArrayShadow u_omniMapArr;

#endif // FRAGMENT_SHADER

#endif

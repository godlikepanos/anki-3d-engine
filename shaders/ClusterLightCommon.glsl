// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_CLUSTER_LIGHT_COMMON_GLSL
#define ANKI_SHADERS_CLUSTER_LIGHT_COMMON_GLSL

#include "shaders/LightFunctions.glsl"

// Common uniforms between lights
struct LightingUniforms
{
	vec4 projectionParams;
	vec4 rendererSizeTimePad1;
	vec4 nearFarClustererMagicPad1;
	mat3 invViewRotation;
	uvec4 tileCount;
	mat4 invViewProjMat;
	mat4 prevViewProjMat;
};

// Point light
struct PointLight
{
	vec4 posRadius; // xyz: Light pos in view space. w: The -1/(radius^2)
	vec4 diffuseColorShadowmapId; // xyz: diff color, w: shadowmap tex ID
	vec4 specularColorRadius; // xyz: spec color, w: radius
};

// Spot light
struct SpotLight
{
	vec4 posRadius; // xyz: Light pos in view space. w: The -1/(radius^2)
	vec4 diffuseColorShadowmapId; // xyz: diff color, w: shadowmap tex ID
	vec4 specularColorRadius; // xyz: spec color, w: radius
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

// Decal
struct Decal
{
	vec4 diffUv;
	vec4 normRoughnessUv;
	mat4 texProjectionMat;
	vec4 blendFactors;
};

layout(ANKI_UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING), std140, row_major) uniform u0_
{
	LightingUniforms u_lightingUniforms;
};

#ifdef FRAGMENT_SHADER

layout(ANKI_UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING + 1), std140) uniform u1_
{
	PointLight u_pointLights[UBO_MAX_SIZE / (3 * 4 * 4)];
};

layout(ANKI_UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING + 2), std140, row_major) uniform u2_
{
	SpotLight u_spotLights[UBO_MAX_SIZE / (9 * 4 * 4)];
};

#ifdef LIGHT_INDIRECT
layout(std140, row_major, ANKI_UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING + 3)) uniform u3_
{
	ReflectionProbe u_reflectionProbes[UBO_MAX_SIZE / (2 * 4 * 4)];
};
#endif

#ifdef LIGHT_DECALS
layout(std140, row_major, ANKI_UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING + 4)) uniform u4_
{
	Decal u_decals[UBO_MAX_SIZE / ((4 + 16) * 4)];
};
#endif

layout(ANKI_SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 0), std430) readonly buffer s0_
{
	uint u_clusters[];
};

layout(std430, ANKI_SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 1)) readonly buffer s1_
{
	uint u_lightIndices[];
};

layout(ANKI_TEX_BINDING(LIGHT_SET, LIGHT_TEX_BINDING)) uniform highp sampler2DArrayShadow u_spotMapArr;
layout(ANKI_TEX_BINDING(LIGHT_SET, LIGHT_TEX_BINDING + 1)) uniform highp samplerCubeArrayShadow u_omniMapArr;

#ifdef LIGHT_INDIRECT
layout(ANKI_TEX_BINDING(LIGHT_SET, LIGHT_TEX_BINDING + 2)) uniform samplerCubeArray u_reflectionsTex;
layout(ANKI_TEX_BINDING(LIGHT_SET, LIGHT_TEX_BINDING + 3)) uniform samplerCubeArray u_irradianceTex;
layout(ANKI_TEX_BINDING(LIGHT_SET, LIGHT_TEX_BINDING + 4)) uniform sampler2D u_integrationLut;
#endif

#endif // FRAGMENT_SHADER

#endif

// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_CLUSTER_LIGHT_COMMON_GLSL
#define ANKI_SHADERS_CLUSTER_LIGHT_COMMON_GLSL

#include "shaders/LightFunctions.glsl"
#include "shaders/Clusterer.glsl"

// Common uniforms between lights
struct LightingUniforms
{
	vec4 projectionParams;
	vec4 rendererSizeTimeNear;
	vec4 cameraPosFar;
	ClustererMagicValues clustererMagicValues;
	uvec4 tileCount;
	mat4 invViewMat;
	mat4 invViewProjMat;
	mat4 prevViewProjMat;
	mat4 invProjMat;
};

// Point light
struct PointLight
{
	vec4 posRadius; // xyz: Light pos in view space. w: The -1/(radius^2)
	vec4 diffuseColorShadowmapId; // xyz: diff color, w: tile size in the shadow atlas
	vec4 specularColorRadius; // xyz: spec color, w: radius
	uvec4 atlasTilesPad2; // x: encodes 6 uints with atlas tile indices in the x dir. y: same for y dir.
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

//
// Common uniforms
//
#if defined(LIGHT_COMMON_UNIS)

const uint _NEXT_UBO_BINDING = LIGHT_UBO_BINDING + 1;

layout(ANKI_UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING), std140, row_major) uniform u0_
{
	LightingUniforms u_lightingUniforms;
};

#define u_near readFirstInvocationARB(u_lightingUniforms.rendererSizeTimeNear.w)
#define u_far readFirstInvocationARB(u_lightingUniforms.cameraPosFar.w)
#define u_clusterCountX readFirstInvocationARB(u_lightingUniforms.tileCount.x)
#define u_clusterCountY readFirstInvocationARB(u_lightingUniforms.tileCount.y)
#define u_clustererMagic u_lightingUniforms.clustererMagicValues
#define u_time readFirstInvocationARB(u_lightingUniforms.rendererSizeTimeNear.z)
#define u_unprojectionParams readFirstInvocationARB(u_lightingUniforms.projectionParams)
#define u_invViewMat u_lightingUniforms.invViewMat
#define u_invProjMat u_lightingUniforms.invProjMat
#define u_invViewProjMat u_lightingUniforms.invViewProjMat
#define u_prevViewProjMat u_lightingUniforms.prevViewProjMat
#define u_cameraPos u_lightingUniforms.cameraPosFar.xyz
#define u_rendererSize u_lightingUniforms.rendererSizeTimeNear.xy

#else
const uint _NEXT_UBO_BINDING = LIGHT_UBO_BINDING;
#endif

//
// Light uniforms
//
#if defined(LIGHT_LIGHTS)
const uint _NEXT_UBO_BINDING_2 = _NEXT_UBO_BINDING + 2;
const uint _NEXT_TEX_BINDING_2 = LIGHT_TEX_BINDING + 1;

layout(ANKI_UBO_BINDING(LIGHT_SET, _NEXT_UBO_BINDING), std140) uniform u1_
{
	PointLight u_pointLights[UBO_MAX_SIZE / (3 * 4 * 4)];
};

layout(ANKI_UBO_BINDING(LIGHT_SET, _NEXT_UBO_BINDING + 1), std140, row_major) uniform u2_
{
	SpotLight u_spotLights[UBO_MAX_SIZE / (9 * 4 * 4)];
};

layout(ANKI_TEX_BINDING(LIGHT_SET, LIGHT_TEX_BINDING + 0)) uniform highp sampler2D u_shadowTex;
#else
const uint _NEXT_UBO_BINDING_2 = _NEXT_UBO_BINDING;
const uint _NEXT_TEX_BINDING_2 = LIGHT_TEX_BINDING;
#endif

//
// Indirect uniforms
//
#if defined(LIGHT_INDIRECT)
const uint _NEXT_UBO_BINDING_3 = _NEXT_UBO_BINDING_2 + 1;
const uint _NEXT_TEX_BINDING_3 = _NEXT_TEX_BINDING_2 + 3;

layout(std140, row_major, ANKI_UBO_BINDING(LIGHT_SET, _NEXT_UBO_BINDING_2)) uniform u3_
{
	ReflectionProbe u_reflectionProbes[UBO_MAX_SIZE / (2 * 4 * 4)];
};

layout(ANKI_TEX_BINDING(LIGHT_SET, _NEXT_TEX_BINDING_2 + 0)) uniform samplerCubeArray u_reflectionsTex;
layout(ANKI_TEX_BINDING(LIGHT_SET, _NEXT_TEX_BINDING_2 + 1)) uniform samplerCubeArray u_irradianceTex;
layout(ANKI_TEX_BINDING(LIGHT_SET, _NEXT_TEX_BINDING_2 + 2)) uniform sampler2D u_integrationLut;
#else
const uint _NEXT_UBO_BINDING_3 = _NEXT_UBO_BINDING_2;
const uint _NEXT_TEX_BINDING_3 = _NEXT_TEX_BINDING_2;
#endif

//
// Decal uniforms
//
#if defined(LIGHT_DECALS)
layout(std140, row_major, ANKI_UBO_BINDING(LIGHT_SET, _NEXT_UBO_BINDING_3)) uniform u4_
{
	Decal u_decals[UBO_MAX_SIZE / ((4 + 16) * 4)];
};

layout(ANKI_TEX_BINDING(LIGHT_SET, _NEXT_TEX_BINDING_3 + 0)) uniform sampler2D u_diffDecalTex;
layout(ANKI_TEX_BINDING(LIGHT_SET, _NEXT_TEX_BINDING_3 + 1)) uniform sampler2D u_normalRoughnessDecalTex;
#endif

//
// Cluster uniforms
//
layout(ANKI_SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 0), std430) readonly buffer s0_
{
	uint u_clusters[];
};

layout(std430, ANKI_SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 1)) readonly buffer s1_
{
	uint u_lightIndices[];
};

#endif

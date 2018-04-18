// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
	vec4 unprojectionParams;
	vec4 rendererSizeTimeNear;
	vec4 cameraPosFar;
	ClustererMagicValues clustererMagicValues;
	uvec4 tileCount;
	mat4 viewMat;
	mat4 invViewMat;
	mat4 projMat;
	mat4 invProjMat;
	mat4 viewProjMat;
	mat4 invViewProjMat;
	mat4 prevViewProjMat;
	mat4 prevViewProjMatMulInvViewProjMat; // Used to re-project previous frames
};

// Point light
struct PointLight
{
	vec4 posRadius; // xyz: Light pos in world space. w: The 1/(radius^2)
	vec4 diffuseColorTileSize; // xyz: diff color, w: tile size in the shadow atlas
	vec2 radiusPad1; // x: radius
	uvec2 atlasTiles; // x: encodes 6 uints with atlas tile indices in the x dir. y: same for y dir.
};
const uint SIZEOF_POINT_LIGHT = 3 * SIZEOF_VEC4;

// Spot light
struct SpotLight
{
	vec4 posRadius; // xyz: Light pos in world space. w: The 1/(radius^2)
	vec4 diffuseColorShadowmapId; // xyz: diff color, w: shadowmap tex ID
	vec4 lightDirRadius; // xyz: light direction, w: radius
	vec4 outerCosInnerCos;
	mat4 texProjectionMat;
};
const uint SIZEOF_SPOT_LIGHT = 4 * SIZEOF_VEC4 + SIZEOF_MAT4;

// Representation of a reflection probe
struct ReflectionProbe
{
	// Position of the prove in view space. Radius of probe squared
	vec4 positionRadiusSq;

	// Slice in u_reflectionsTex vector.
	vec4 cubemapIndexPad3;
};
const uint SIZEOF_REFLECTION_PROBE = 2 * SIZEOF_VEC4;

// Decal
struct Decal
{
	vec4 diffUv;
	vec4 normRoughnessUv;
	mat4 texProjectionMat;
	vec4 blendFactors;
};
const uint SIZEOF_DECAL = 3 * SIZEOF_VEC4 + SIZEOF_MAT4;

//
// Common uniforms
//
#if defined(LIGHT_COMMON_UNIS)

const uint _NEXT_UBO_BINDING = LIGHT_UBO_BINDING + 1;

layout(ANKI_UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING), std140, row_major) uniform lu0_
{
	LightingUniforms u_lightingUniforms;
};

#	define u_near UNIFORM(u_lightingUniforms.rendererSizeTimeNear.w)
#	define u_far UNIFORM(u_lightingUniforms.cameraPosFar.w)
#	define u_cameraPos UNIFORM(u_lightingUniforms.cameraPosFar.xyz)
#	define u_clusterCountX UNIFORM(u_lightingUniforms.tileCount.x)
#	define u_clusterCountY UNIFORM(u_lightingUniforms.tileCount.y)
#	define u_clustererMagic u_lightingUniforms.clustererMagicValues
#	define u_time UNIFORM(u_lightingUniforms.rendererSizeTimeNear.z)
#	define u_unprojectionParams UNIFORM(u_lightingUniforms.unprojectionParams)
#	define u_rendererSize u_lightingUniforms.rendererSizeTimeNear.xy

#	define u_viewMat u_lightingUniforms.viewMat
#	define u_invViewMat u_lightingUniforms.invViewMat
#	define u_projMat u_lightingUniforms.projMat
#	define u_invProjMat u_lightingUniforms.invProjMat
#	define u_viewProjMat u_lightingUniforms.viewProjMat
#	define u_invViewProjMat u_lightingUniforms.invViewProjMat
#	define u_prevViewProjMat u_lightingUniforms.prevViewProjMat
#	define u_prevViewProjMatMulInvViewProjMat u_lightingUniforms.prevViewProjMatMulInvViewProjMat

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
	PointLight u_pointLights[UBO_MAX_SIZE / SIZEOF_POINT_LIGHT];
};

layout(ANKI_UBO_BINDING(LIGHT_SET, _NEXT_UBO_BINDING + 1), std140, row_major) uniform u2_
{
	SpotLight u_spotLights[UBO_MAX_SIZE / SIZEOF_SPOT_LIGHT];
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
	ReflectionProbe u_reflectionProbes[UBO_MAX_SIZE / SIZEOF_REFLECTION_PROBE];
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
	Decal u_decals[UBO_MAX_SIZE / SIZEOF_DECAL];
};

layout(ANKI_TEX_BINDING(LIGHT_SET, _NEXT_TEX_BINDING_3 + 0)) uniform sampler2D u_diffDecalTex;
layout(ANKI_TEX_BINDING(LIGHT_SET, _NEXT_TEX_BINDING_3 + 1)) uniform sampler2D u_specularRoughnessDecalTex;
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

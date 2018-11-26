// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <shaders/LightFunctions.glsl>

//
// Common uniforms
//
#if defined(LIGHT_COMMON_UNIS)

const U32 _NEXT_UBO_BINDING = LIGHT_UBO_BINDING + 1;

layout(ANKI_UBO_BINDING(LIGHT_SET, LIGHT_UBO_BINDING), std140, row_major) uniform lu0_
{
	LightingUniforms u_lightingUniforms;
};

#	define u_near UNIFORM(u_lightingUniforms.m_rendererSizeTimeNear.w)
#	define u_far UNIFORM(u_lightingUniforms.m_cameraPosFar.w)
#	define u_cameraPos UNIFORM(u_lightingUniforms.m_cameraPosFar.xyz)
#	define u_clusterCountX UNIFORM(u_lightingUniforms.m_clusterCount.x)
#	define u_clusterCountY UNIFORM(u_lightingUniforms.m_clusterCount.y)
#	define u_clustererMagic u_lightingUniforms.m_clustererMagicValues
#	define u_prevClustererMagic u_lightingUniforms.m_prevClustererMagicValues
#	define u_time UNIFORM(u_lightingUniforms.m_rendererSizeTimeNear.z)
#	define u_unprojectionParams UNIFORM(u_lightingUniforms.m_unprojectionParams)
#	define u_rendererSize u_lightingUniforms.m_rendererSizeTimeNear.xy
#	define u_lightVolumeLastCluster UNIFORM(u_lightingUniforms.m_lightVolumeLastClusterPad3.x)

#	define u_viewMat u_lightingUniforms.m_viewMat
#	define u_invViewMat u_lightingUniforms.m_invViewMat
#	define u_projMat u_lightingUniforms.m_projMat
#	define u_invProjMat u_lightingUniforms.m_invProjMat
#	define u_viewProjMat u_lightingUniforms.m_viewProjMat
#	define u_invViewProjMat u_lightingUniforms.m_invViewProjMat
#	define u_prevViewProjMat u_lightingUniforms.m_prevViewProjMat
#	define u_prevViewProjMatMulInvViewProjMat u_lightingUniforms.m_prevViewProjMatMulInvViewProjMat

#else
const U32 _NEXT_UBO_BINDING = LIGHT_UBO_BINDING;
#endif

//
// Light uniforms
//
#if defined(LIGHT_LIGHTS)
const U32 _NEXT_UBO_BINDING_2 = _NEXT_UBO_BINDING + 2;
const U32 _NEXT_TEX_BINDING_2 = LIGHT_TEX_BINDING + 1;

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
const U32 _NEXT_UBO_BINDING_2 = _NEXT_UBO_BINDING;
const U32 _NEXT_TEX_BINDING_2 = LIGHT_TEX_BINDING;
#endif

//
// Indirect uniforms
//
#if defined(LIGHT_INDIRECT)
const U32 _NEXT_UBO_BINDING_3 = _NEXT_UBO_BINDING_2 + 1;
const U32 _NEXT_TEX_BINDING_3 = _NEXT_TEX_BINDING_2 + 3;

layout(std140, row_major, ANKI_UBO_BINDING(LIGHT_SET, _NEXT_UBO_BINDING_2)) uniform u3_
{
	ReflectionProbe u_reflectionProbes[UBO_MAX_SIZE / SIZEOF_REFLECTION_PROBE];
};

layout(ANKI_TEX_BINDING(LIGHT_SET, _NEXT_TEX_BINDING_2 + 0)) uniform samplerCubeArray u_reflectionsTex;
layout(ANKI_TEX_BINDING(LIGHT_SET, _NEXT_TEX_BINDING_2 + 1)) uniform samplerCubeArray u_irradianceTex;
layout(ANKI_TEX_BINDING(LIGHT_SET, _NEXT_TEX_BINDING_2 + 2)) uniform sampler2D u_integrationLut;
#else
const U32 _NEXT_UBO_BINDING_3 = _NEXT_UBO_BINDING_2;
const U32 _NEXT_TEX_BINDING_3 = _NEXT_TEX_BINDING_2;
#endif

//
// Decal uniforms
//
#if defined(LIGHT_DECALS)
const U32 _NEXT_UBO_BINDING_4 = _NEXT_UBO_BINDING_3 + 1u;

layout(std140, row_major, ANKI_UBO_BINDING(LIGHT_SET, _NEXT_UBO_BINDING_3)) uniform u4_
{
	Decal u_decals[UBO_MAX_SIZE / SIZEOF_DECAL];
};

layout(ANKI_TEX_BINDING(LIGHT_SET, _NEXT_TEX_BINDING_3 + 0)) uniform sampler2D u_diffDecalTex;
layout(ANKI_TEX_BINDING(LIGHT_SET, _NEXT_TEX_BINDING_3 + 1)) uniform sampler2D u_specularRoughnessDecalTex;
#else
const U32 _NEXT_UBO_BINDING_4 = _NEXT_UBO_BINDING_3;
#endif

//
// Fog density uniforms
//
#if defined(LIGHT_FOG_DENSITY_VOLUMES)
layout(std140, row_major, ANKI_UBO_BINDING(LIGHT_SET, _NEXT_UBO_BINDING_4)) uniform u5_
{
	FogDensityVolume u_fogDensityVolumes[UBO_MAX_SIZE / SIZEOF_FOG_DENSITY_VOLUME];
};
#endif

//
// Cluster uniforms
//
layout(ANKI_SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 0), std430) readonly buffer s0_
{
	U32 u_clusters[];
};

layout(std430, ANKI_SS_BINDING(LIGHT_SET, LIGHT_SS_BINDING + 1)) readonly buffer s1_
{
	U32 u_lightIndices[];
};

// Debugging function
Vec3 lightHeatmap(U32 firstIndex, U32 maxLights, Bool decals, Bool plights, Bool slights, Bool probes, Bool fogVolumes)
{
	U32 count = 0;
	U32 idx;

	while((idx = u_lightIndices[firstIndex++]) != MAX_U32)
	{
		count += (plights) ? 1u : 0u;
	}

	while((idx = u_lightIndices[firstIndex++]) != MAX_U32)
	{
		count += (slights) ? 1u : 0u;
	}

	while((idx = u_lightIndices[firstIndex++]) != MAX_U32)
	{
		count += (probes) ? 1u : 0u;
	}

	while((idx = u_lightIndices[firstIndex++]) != MAX_U32)
	{
		count += (decals) ? 1u : 0u;
	}

	while((idx = u_lightIndices[firstIndex++]) != MAX_U32)
	{
		count += (fogVolumes) ? 1u : 0u;
	}

	F32 factor = min(1.0, F32(count) / F32(maxLights));
	return heatmap(factor);
}
// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shaders/LightFunctions.glsl>

//
// Common uniforms
//
#if defined(LIGHT_COMMON_UNIS_BINDING)

layout(set = LIGHT_SET, binding = LIGHT_COMMON_UNIS_BINDING, std140, row_major) uniform lu0_
{
	LightingUniforms u_lightingUniforms;
};

#	define u_near UNIFORM(u_lightingUniforms.m_near)
#	define u_far UNIFORM(u_lightingUniforms.m_far)
#	define u_cameraPos UNIFORM(u_lightingUniforms.m_cameraPos)
#	define u_clusterCountX UNIFORM(u_lightingUniforms.m_clusterCount.x)
#	define u_clusterCountY UNIFORM(u_lightingUniforms.m_clusterCount.y)
#	define u_clustererMagic u_lightingUniforms.m_clustererMagicValues
#	define u_prevClustererMagic u_lightingUniforms.m_prevClustererMagicValues
#	define u_time UNIFORM(u_lightingUniforms.m_time)
#	define u_unprojectionParams UNIFORM(u_lightingUniforms.m_unprojectionParams)
#	define u_rendererSize u_lightingUniforms.m_rendererSize
#	define u_lightVolumeLastCluster UNIFORM(u_lightingUniforms.m_lightVolumeLastCluster)

#	define u_viewMat u_lightingUniforms.m_viewMat
#	define u_invViewMat u_lightingUniforms.m_invViewMat
#	define u_projMat u_lightingUniforms.m_projMat
#	define u_invProjMat u_lightingUniforms.m_invProjMat
#	define u_viewProjMat u_lightingUniforms.m_viewProjMat
#	define u_invViewProjMat u_lightingUniforms.m_invViewProjMat
#	define u_prevViewProjMat u_lightingUniforms.m_prevViewProjMat
#	define u_prevViewProjMatMulInvViewProjMat u_lightingUniforms.m_prevViewProjMatMulInvViewProjMat
#	define u_dirLight u_lightingUniforms.m_dirLight
#endif

//
// Light uniforms (3)
//
#if defined(LIGHT_LIGHTS_BINDING)
layout(set = LIGHT_SET, binding = LIGHT_LIGHTS_BINDING, std140) uniform u1_
{
	PointLight u_pointLights[UBO_MAX_SIZE / ANKI_SIZEOF(PointLight)];
};

layout(set = LIGHT_SET, binding = LIGHT_LIGHTS_BINDING + 1, std140, row_major) uniform u2_
{
	SpotLight u_spotLights[UBO_MAX_SIZE / ANKI_SIZEOF(SpotLight)];
};

layout(set = LIGHT_SET, binding = LIGHT_LIGHTS_BINDING + 2) uniform highp texture2D u_shadowTex;
#endif

//
// Indirect uniforms (3)
//
#if defined(LIGHT_INDIRECT_SPECULAR_BINDING)
layout(std140, row_major, set = LIGHT_SET, binding = LIGHT_INDIRECT_SPECULAR_BINDING) uniform u3_
{
	ReflectionProbe u_reflectionProbes[UBO_MAX_SIZE / ANKI_SIZEOF(ReflectionProbe)];
};

layout(set = LIGHT_SET, binding = LIGHT_INDIRECT_SPECULAR_BINDING + 1) uniform textureCubeArray u_reflectionsTex;
layout(set = LIGHT_SET, binding = LIGHT_INDIRECT_SPECULAR_BINDING + 2) uniform texture2D u_integrationLut;
#endif

//
// Decal uniforms (3)
//
#if defined(LIGHT_DECALS_BINDING)
layout(std140, row_major, set = LIGHT_SET, binding = LIGHT_DECALS_BINDING) uniform u4_
{
	Decal u_decals[UBO_MAX_SIZE / ANKI_SIZEOF(Decal)];
};

layout(set = LIGHT_SET, binding = LIGHT_DECALS_BINDING + 1) uniform texture2D u_diffDecalTex;
layout(set = LIGHT_SET, binding = LIGHT_DECALS_BINDING + 2) uniform texture2D u_specularRoughnessDecalTex;
#endif

//
// Fog density uniforms (1)
//
#if defined(LIGHT_FOG_DENSITY_VOLUMES_BINDING)
layout(std140, row_major, set = LIGHT_SET, binding = LIGHT_FOG_DENSITY_VOLUMES_BINDING) uniform u5_
{
	FogDensityVolume u_fogDensityVolumes[UBO_MAX_SIZE / ANKI_SIZEOF(FogDensityVolume)];
};
#endif

//
// GI (2)
//
#if defined(LIGHT_GLOBAL_ILLUMINATION_BINDING)
layout(set = LIGHT_SET, binding = LIGHT_GLOBAL_ILLUMINATION_BINDING) uniform texture3D
	u_globalIlluminationTextures[MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES];

layout(set = LIGHT_SET, binding = LIGHT_GLOBAL_ILLUMINATION_BINDING + 1) uniform ugi_
{
	GlobalIlluminationProbe u_giProbes[MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES];
};
#endif

//
// Cluster uniforms
//
#if defined(LIGHT_CLUSTERS_BINDING)
layout(set = LIGHT_SET, binding = LIGHT_CLUSTERS_BINDING, std430) readonly buffer s0_
{
	U32 u_clusters[];
};

layout(set = LIGHT_SET, binding = LIGHT_CLUSTERS_BINDING + 1, std430) readonly buffer s1_
{
	U32 u_lightIndices[];
};
#endif

// Debugging function
Vec3 lightHeatmap(U32 firstIndex, U32 maxObjects, U32 typeMask)
{
	U32 count = 0;
	U32 idx;

	while((idx = u_lightIndices[firstIndex++]) != MAX_U32)
	{
		count += ((typeMask & (1u << 0u)) != 0u) ? 1u : 0u;
	}

	while((idx = u_lightIndices[firstIndex++]) != MAX_U32)
	{
		count += ((typeMask & (1u << 1u)) != 0u) ? 1u : 0u;
	}

	while((idx = u_lightIndices[firstIndex++]) != MAX_U32)
	{
		count += ((typeMask & (1u << 2u)) != 0u) ? 1u : 0u;
	}

	while((idx = u_lightIndices[firstIndex++]) != MAX_U32)
	{
		count += ((typeMask & (1u << 3u)) != 0u) ? 1u : 0u;
	}

	while((idx = u_lightIndices[firstIndex++]) != MAX_U32)
	{
		count += ((typeMask & (1u << 4u)) != 0u) ? 1u : 0u;
	}

	while((idx = u_lightIndices[firstIndex++]) != MAX_U32)
	{
		count += ((typeMask & (1u << 5u)) != 0u) ? 1u : 0u;
	}

	const F32 factor = min(1.0, F32(count) / F32(maxObjects));
	return heatmap(factor);
}

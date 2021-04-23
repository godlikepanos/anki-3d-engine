// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/LightFunctions.glsl>

//
// Common uniforms
//
#if defined(CLUSTERED_SHADING_UNIFORMS_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTERED_SHADING_UNIFORMS_BINDING, scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	ClusteredShadingUniforms u_clusterShading;
};
#endif

//
// Light uniforms (3)
//
#if defined(CLUSTER_SHADING_LIGHTS_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTER_SHADING_LIGHTS_BINDING, scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	PointLight2 u_pointLights2[MAX_VISIBLE_POINT_LIGHTS];
};

layout(set = CLUSTERED_SHADING_SET, binding = CLUSTER_SHADING_LIGHTS_BINDING + 1, scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	SpotLight2 u_spotLights2[MAX_VISIBLE_SPOT_LIGHTS];
};

layout(set = CLUSTERED_SHADING_SET, binding = CLUSTER_SHADING_LIGHTS_BINDING + 2) uniform texture2D u_shadowAtlasTex;
#endif

//
// Indirect uniforms (3)
//
#if defined(CLUSTER_SHADING_REFLECTIONS_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTER_SHADING_REFLECTIONS_BINDING,
	   scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	ReflectionProbe2 u_reflectionProbes2[MAX_VISIBLE_REFLECTION_PROBES];
};

layout(set = CLUSTERED_SHADING_SET,
	   binding = CLUSTER_SHADING_REFLECTIONS_BINDING + 1) uniform textureCubeArray u_reflectionsTex2;
layout(set = CLUSTERED_SHADING_SET,
	   binding = CLUSTER_SHADING_REFLECTIONS_BINDING + 2) uniform texture2D u_integrationLut2;
#endif

//
// Decal uniforms (3)
//
#if defined(CLUSTER_SHADING_DECALS_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTER_SHADING_DECALS_BINDING, scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	Decal2 u_decals2[MAX_VISIBLE_DECALS];
};

layout(set = CLUSTERED_SHADING_SET, binding = CLUSTER_SHADING_DECALS_BINDING + 1) uniform texture2D u_diffuseDecalTex;
layout(set = CLUSTERED_SHADING_SET,
	   binding = CLUSTER_SHADING_DECALS_BINDING + 2) uniform texture2D u_specularRoughnessDecalTex;
#endif

//
// Fog density uniforms (1)
//
#if defined(CLUSTER_SHADING_FOG_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTER_SHADING_FOG_BINDING, scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	FogDensityVolume u_fogDensityVolumes[MAX_VISIBLE_FOG_DENSITY_VOLUMES];
};
#endif

//
// GI (2)
//
#if defined(CLUSTER_SHADING_GI_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTER_SHADING_GI_BINDING) uniform texture3D
	u_globalIlluminationTextures[MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES];

layout(set = CLUSTERED_SHADING_SET, binding = CLUSTER_SHADING_GI_BINDING + 1, scalar) uniform ANKI_RANDOM_BLOCK_NAME
{
	GlobalIlluminationProbe u_giProbes[MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES];
};
#endif

//
// Cluster uniforms
//
#if defined(CLUSTER_SHADING_CLUSTERS_BINDING)
layout(set = CLUSTERED_SHADING_SET, binding = CLUSTER_SHADING_CLUSTERS_BINDING,
	   scalar) readonly buffer ANKI_RANDOM_BLOCK_NAME
{
	Cluster u_clusters[];
};
#endif

// Debugging function
Vec3 lightHeatmap2(U32 firstIndex, U32 maxObjects, U32 typeMask)
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

// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

// Enum of clusterer object types
const U32 CLUSTER_OBJECT_TYPE_POINT_LIGHT = 0u;
const U32 CLUSTER_OBJECT_TYPE_SPOT_LIGHT = 1u;
const U32 CLUSTER_OBJECT_TYPE_DECAL = 2u;
const U32 CLUSTER_OBJECT_TYPE_FOG_DENSITY_VOLUME = 3u;
const U32 CLUSTER_OBJECT_TYPE_REFLECTION_PROBE = 4u;
const U32 CLUSTER_OBJECT_TYPE_GLOBAL_ILLUMINATION_PROBE = 5u;
const U32 CLUSTER_OBJECT_TYPE_COUNT2 = 6u; ///< Point and spot lights, refl and GI probes, decals and fog volumes.

const F32 CLUSTER_OBJECT_FRUSTUM_NEAR_PLANE = 0.1f / 4.0f; ///< The near plane of various clusterer object frustums.
const U32 MAX_SHADOW_CASCADES2 = 4u;

// Limits
const U32 MAX_VISIBLE_POINT_LIGHTS = 64u;
const U32 MAX_VISIBLE_SPOT_LIGHTS = 64u;
const U32 MAX_VISIBLE_DECALS = 64u;
const U32 MAX_VISIBLE_FOG_DENSITY_VOLUMES = 16u;
const U32 MAX_VISIBLE_REFLECTION_PROBES = 16u;
const U32 MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES2 = 8u;

/// Point light.
struct PointLight2
{
	Vec3 m_position; ///< Position in world space.
	Vec3 m_diffuseColor;
	F32 m_radius; ///< Radius
	F32 m_squareRadiusOverOne; ///< 1/(radius^2).
	U32 m_shadowLayer; ///< Shadow layer used in RT shadows. Also used to show that it doesn't cast shadow.
	F32 m_shadowAtlasTileScale; ///< UV scale for all tiles.
	Vec2 m_shadowAtlasTileOffsets[6u];
};
const U32 _ANKI_SIZEOF_PointLight2 = 22u * ANKI_SIZEOF(U32);
ANKI_SHADER_STATIC_ASSERT(sizeof(PointLight2) == _ANKI_SIZEOF_PointLight2);

/// Spot light.
struct SpotLight2
{
	Vec3 m_position; ///< Position in world space.
	Vec3 m_edgePoints[4u]; ///< Edge points in world space.
	Vec3 m_diffuseColor;
	F32 m_radius; ///< Max distance.
	F32 m_squareRadiusOverOne; ///< 1/(radius^2).
	U32 m_shadowLayer; ///< Shadow layer used in RT shadows. Also used to show that it doesn't cast shadow.
	Vec3 m_direction; ///< Light direction.
	F32 m_outerCos;
	F32 m_innerCos;
	Vec2 m_padding;
	Mat4 m_textureMatrix;
};
const U32 _ANKI_SIZEOF_SpotLight2 = 28u * ANKI_SIZEOF(U32) + ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(SpotLight2) == _ANKI_SIZEOF_SpotLight2);

/// Spot light different view. This is the same structure as SpotLight but it's designed for binning.
struct SpotLightBinning
{
	Vec3 m_edgePoints[5u]; ///< Edge points in world space.
	Vec3 m_diffuseColor;
	F32 m_radius; ///< Max distance.
	F32 m_squareRadiusOverOne; ///< 1/(radius^2).
	U32 m_shadowLayer; ///< Shadow layer used in RT shadows. Also used to show that it doesn't cast shadow.
	Vec3 m_direction; ///< Light direction.
	F32 m_outerCos;
	F32 m_innerCos;
	Vec2 m_padding;
	Mat4 m_textureMatrix;
};
const U32 _ANKI_SIZEOF_SpotLightBinning = _ANKI_SIZEOF_SpotLight2;
ANKI_SHADER_STATIC_ASSERT(sizeof(SpotLightBinning) == _ANKI_SIZEOF_SpotLightBinning);
ANKI_SHADER_STATIC_ASSERT(alignof(SpotLightBinning) == alignof(SpotLight2));

/// Directional light (sun).
struct DirectionalLight2
{
	Vec3 m_diffuseColor;
	U32 m_cascadeCount; ///< If it's zero then it doesn't cast shadow.
	Vec3 m_direction;
	U32 m_active;
	F32 m_effectiveShadowDistance;
	F32 m_shadowCascadesDistancePower;
	U32 m_shadowLayer; ///< Shadow layer used in RT shadows. Also used to show that it doesn't cast shadow.
	U32 m_padding;
	Mat4 m_textureMatrices[MAX_SHADOW_CASCADES2];
};
const U32 _ANKI_SIZEOF_DirectionalLight2 = 12u * ANKI_SIZEOF(U32) + MAX_SHADOW_CASCADES2 * ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(DirectionalLight2) == _ANKI_SIZEOF_DirectionalLight2);

/// Representation of a reflection probe.
struct ReflectionProbe2
{
	Vec3 m_position; ///< Position of the probe in world space.
	F32 m_cubemapIndex; ///< Index in the cubemap array texture.
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;
};
const U32 _ANKI_SIZEOF_ReflectionProbe2 = 10u * ANKI_SIZEOF(U32);
ANKI_SHADER_STATIC_ASSERT(sizeof(ReflectionProbe2) == _ANKI_SIZEOF_ReflectionProbe2);

/// Decal2.
struct Decal2
{
	Vec4 m_diffuseUv;
	Vec4 m_normRoughnessUv;
	Vec4 m_blendFactors;
	Mat4 m_textureMatrix;
	Mat4 m_invertedTransform;
	Vec3 m_obbExtend;
	F32 m_padding;
};
const U32 _ANKI_SIZEOF_Decal2 = 4u * ANKI_SIZEOF(Vec4) + 2u * ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(Decal2) == _ANKI_SIZEOF_Decal2);

/// Fog density volume.
struct FogDensityVolume2
{
	Vec3 m_aabbMinOrSphereCenter;
	U32 m_isBox;
	Vec3 m_aabbMaxOrSphereRadiusSquared;
	F32 m_density;
};
const U32 _ANKI_SIZEOF_FogDensityVolume2 = 2u * ANKI_SIZEOF(Vec4);
ANKI_SHADER_STATIC_ASSERT(sizeof(FogDensityVolume2) == _ANKI_SIZEOF_FogDensityVolume2);

/// Global illumination probe
struct GlobalIlluminationProbe2
{
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;

	U32 m_textureIndex; ///< Index to the array of volume textures.
	F32 m_halfTexelSizeU; ///< (1.0 / textureSize(texArr[textureIndex]).x) / 2.0

	/// Used to calculate a factor that is zero when fragPos is close to AABB bounds and 1.0 at fadeDistance and less.
	F32 m_fadeDistance;
};
const U32 _ANKI_SIZEOF_GlobalIlluminationProbe2 = 9u * ANKI_SIZEOF(U32);
ANKI_SHADER_STATIC_ASSERT(sizeof(GlobalIlluminationProbe2) == _ANKI_SIZEOF_GlobalIlluminationProbe2);

/// Common matrices.
struct CommonMatrices
{
	Mat4 m_cameraTransform ANKI_CPP_CODE(= Mat4::getIdentity());
	Mat4 m_view ANKI_CPP_CODE(= Mat4::getIdentity());
	Mat4 m_projection ANKI_CPP_CODE(= Mat4::getIdentity());
	Mat4 m_viewProjection ANKI_CPP_CODE(= Mat4::getIdentity());

	Mat4 m_jitter ANKI_CPP_CODE(= Mat4::getIdentity());
	Mat4 m_projectionJitter ANKI_CPP_CODE(= Mat4::getIdentity());
	Mat4 m_viewProjectionJitter ANKI_CPP_CODE(= Mat4::getIdentity());

	Mat4 m_invertedViewProjectionJitter ANKI_CPP_CODE(= Mat4::getIdentity()); ///< To unproject in world space.
	Mat4 m_invertedViewProjection ANKI_CPP_CODE(= Mat4::getIdentity());

	Vec4 m_unprojectionParameters ANKI_CPP_CODE(= Vec4(0.0f)); ///< To unproject to view space. Jitter not considered.
};
const U32 _ANKI_SIZEOF_CommonMatrices = 9u * ANKI_SIZEOF(Mat4) + ANKI_SIZEOF(Vec4);
ANKI_SHADER_STATIC_ASSERT(sizeof(CommonMatrices) == _ANKI_SIZEOF_CommonMatrices);

/// Common uniforms for light shading passes.
struct ClusteredShadingUniforms
{
	Vec2 m_renderingSize;

	F32 m_time;
	U32 m_frame;

	Vec4 m_nearPlaneWSpace;
	F32 m_near;
	F32 m_far;
	Vec3 m_cameraPosition;

	UVec2 m_tileCounts;
	U32 m_zSplitCount;
	F32 m_zSplitCountOverFrustumLength; ///< m_zSplitCount/(far-near)
	Vec2 m_zSplitMagic; ///< It's the "a" and "b" of computeZSplitClusterIndex(). See there for details.
	F32 m_tileSize;
	U32 m_lightVolumeLastCluster;

	U32 m_pointLightCount;
	U32 m_spotLightCount;
	U32 m_decalCount;
	U32 m_fogDensityVolumeCount;
	U32 m_reflectionProbeCount;
	U32 m_giProbeCount;

	U32 m_padding;

	CommonMatrices m_matrices;
	CommonMatrices m_previousMatrices;

	DirectionalLight2 m_directionalLight;
};
const U32 _ANKI_SIZEOF_ClusteredShadingUniforms =
	28u * ANKI_SIZEOF(U32) + 2u * ANKI_SIZEOF(CommonMatrices) + ANKI_SIZEOF(DirectionalLight2);
ANKI_SHADER_STATIC_ASSERT(sizeof(ClusteredShadingUniforms) == _ANKI_SIZEOF_ClusteredShadingUniforms);

/// Information that a tile or a Z-split will contain.
struct Cluster
{
	U64 m_pointLightsMask;
	U64 m_spotLightsMask;
	U64 m_decalsMask;
	U32 m_fogDensityVolumesMask;
	U32 m_reflectionProbesMask;
	U32 m_giProbesMask;
	U32 m_padding; ///< Add some padding to be 100% sure nothing will break.
};
const U32 _ANKI_SIZEOF_Cluster = 5u * ANKI_SIZEOF(U64);
ANKI_SHADER_STATIC_ASSERT(sizeof(Cluster) == _ANKI_SIZEOF_Cluster);

ANKI_END_NAMESPACE

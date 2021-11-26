// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

#define ANKI_CLUSTERED_SHADING_USE_64BIT !ANKI_OS_ANDROID

ANKI_BEGIN_NAMESPACE

// Enum of clusterer object types
const U32 CLUSTER_OBJECT_TYPE_POINT_LIGHT = 0u;
const U32 CLUSTER_OBJECT_TYPE_SPOT_LIGHT = 1u;
const U32 CLUSTER_OBJECT_TYPE_DECAL = 2u;
const U32 CLUSTER_OBJECT_TYPE_FOG_DENSITY_VOLUME = 3u;
const U32 CLUSTER_OBJECT_TYPE_REFLECTION_PROBE = 4u;
const U32 CLUSTER_OBJECT_TYPE_GLOBAL_ILLUMINATION_PROBE = 5u;
const U32 CLUSTER_OBJECT_TYPE_COUNT = 6u; ///< Point and spot lights, refl and GI probes, decals and fog volumes.

// Limits
#if ANKI_CLUSTERED_SHADING_USE_64BIT
const U32 MAX_VISIBLE_POINT_LIGHTS = 64u;
const U32 MAX_VISIBLE_SPOT_LIGHTS = 64u;
const U32 MAX_VISIBLE_DECALS = 64u;
#else
const U32 MAX_VISIBLE_POINT_LIGHTS = 32u;
const U32 MAX_VISIBLE_SPOT_LIGHTS = 32u;
const U32 MAX_VISIBLE_DECALS = 32u;
#endif
const U32 MAX_VISIBLE_FOG_DENSITY_VOLUMES = 16u;
const U32 MAX_VISIBLE_REFLECTION_PROBES = 16u;
const U32 MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES = 8u;

// Other consts
const ANKI_RP F32 CLUSTER_OBJECT_FRUSTUM_NEAR_PLANE = 0.1f / 4.0f; ///< Near plane of various clusterer object frustums.
const U32 MAX_SHADOW_CASCADES2 = 4u;
const ANKI_RP F32 SUBSURFACE_MIN = 0.01f;

/// Point light.
struct PointLight
{
	Vec3 m_position; ///< Position in world space.
	ANKI_RP Vec3 m_diffuseColor;
	ANKI_RP F32 m_radius; ///< Radius
	ANKI_RP F32 m_squareRadiusOverOne; ///< 1/(radius^2).
	U32 m_shadowLayer; ///< Shadow layer used in RT shadows. Also used to show that it doesn't cast shadow.
	F32 m_shadowAtlasTileScale; ///< UV scale for all tiles.
	Vec2 m_shadowAtlasTileOffsets[6u];
};
const U32 _ANKI_SIZEOF_PointLight = 22u * ANKI_SIZEOF(U32);
ANKI_SHADER_STATIC_ASSERT(sizeof(PointLight) == _ANKI_SIZEOF_PointLight);

/// Spot light.
struct SpotLight
{
	Vec3 m_position; ///< Position in world space.
	Vec3 m_edgePoints[4u]; ///< Edge points in world space.
	ANKI_RP Vec3 m_diffuseColor;
	ANKI_RP F32 m_radius; ///< Max distance.
	ANKI_RP F32 m_squareRadiusOverOne; ///< 1/(radius^2).
	U32 m_shadowLayer; ///< Shadow layer used in RT shadows. Also used to show that it doesn't cast shadow.
	ANKI_RP Vec3 m_direction; ///< Light direction.
	ANKI_RP F32 m_outerCos;
	ANKI_RP F32 m_innerCos;
	Vec2 m_padding;
	Mat4 m_textureMatrix;
};
const U32 _ANKI_SIZEOF_SpotLight = 28u * ANKI_SIZEOF(U32) + ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(SpotLight) == _ANKI_SIZEOF_SpotLight);

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
const U32 _ANKI_SIZEOF_SpotLightBinning = _ANKI_SIZEOF_SpotLight;
ANKI_SHADER_STATIC_ASSERT(sizeof(SpotLightBinning) == _ANKI_SIZEOF_SpotLightBinning);
ANKI_SHADER_STATIC_ASSERT(alignof(SpotLightBinning) == alignof(SpotLight));

/// Directional light (sun).
struct DirectionalLight
{
	ANKI_RP Vec3 m_diffuseColor;
	U32 m_cascadeCount; ///< If it's zero then it doesn't cast shadow.
	ANKI_RP Vec3 m_direction;
	U32 m_active;
	ANKI_RP F32 m_effectiveShadowDistance;
	ANKI_RP F32 m_shadowCascadesDistancePower;
	U32 m_shadowLayer; ///< Shadow layer used in RT shadows. Also used to show that it doesn't cast shadow.
	U32 m_padding;
	Mat4 m_textureMatrices[MAX_SHADOW_CASCADES2];
};
const U32 _ANKI_SIZEOF_DirectionalLight = 12u * ANKI_SIZEOF(U32) + MAX_SHADOW_CASCADES2 * ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(DirectionalLight) == _ANKI_SIZEOF_DirectionalLight);

/// Representation of a reflection probe.
struct ReflectionProbe
{
	Vec3 m_position; ///< Position of the probe in world space.
	F32 m_cubemapIndex; ///< Index in the cubemap array texture.
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;
};
const U32 _ANKI_SIZEOF_ReflectionProbe = 10u * ANKI_SIZEOF(U32);
ANKI_SHADER_STATIC_ASSERT(sizeof(ReflectionProbe) == _ANKI_SIZEOF_ReflectionProbe);

/// Decal.
struct Decal
{
	Vec4 m_diffuseUv;
	Vec4 m_normRoughnessUv;
	Vec4 m_blendFactors;
	Mat4 m_textureMatrix;
	Mat4 m_invertedTransform;
	Vec3 m_obbExtend;
	F32 m_padding;
};
const U32 _ANKI_SIZEOF_Decal = 4u * ANKI_SIZEOF(Vec4) + 2u * ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(Decal) == _ANKI_SIZEOF_Decal);

/// Fog density volume.
struct FogDensityVolume
{
	Vec3 m_aabbMinOrSphereCenter;
	U32 m_isBox;
	Vec3 m_aabbMaxOrSphereRadiusSquared;
	F32 m_density;
};
const U32 _ANKI_SIZEOF_FogDensityVolume = 2u * ANKI_SIZEOF(Vec4);
ANKI_SHADER_STATIC_ASSERT(sizeof(FogDensityVolume) == _ANKI_SIZEOF_FogDensityVolume);

/// Global illumination probe
struct GlobalIlluminationProbe
{
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;

	U32 m_textureIndex; ///< Index to the array of volume textures.
	F32 m_halfTexelSizeU; ///< (1.0 / textureSize(texArr[textureIndex]).x) / 2.0

	/// Used to calculate a factor that is zero when fragPos is close to AABB bounds and 1.0 at fadeDistance and less.
	F32 m_fadeDistance;
};
const U32 _ANKI_SIZEOF_GlobalIlluminationProbe = 9u * ANKI_SIZEOF(U32);
ANKI_SHADER_STATIC_ASSERT(sizeof(GlobalIlluminationProbe) == _ANKI_SIZEOF_GlobalIlluminationProbe);

/// Common matrices.
struct CommonMatrices
{
	Mat4 m_cameraTransform ANKI_CPP_CODE(= Mat4::getIdentity());
	Mat4 m_view ANKI_CPP_CODE(= Mat4::getIdentity());
	Mat4 m_projection ANKI_CPP_CODE(= Mat4::getIdentity());
	Mat4 m_viewProjection ANKI_CPP_CODE(= Mat4::getIdentity());
	Mat3 m_viewRotation ANKI_CPP_CODE(= Mat3::getIdentity());

	F32 m_padding[3u]; // Because of the alignment requirements of some of the following members (in C++)

	Mat4 m_jitter ANKI_CPP_CODE(= Mat4::getIdentity());
	Mat4 m_projectionJitter ANKI_CPP_CODE(= Mat4::getIdentity());
	Mat4 m_viewProjectionJitter ANKI_CPP_CODE(= Mat4::getIdentity());

	Mat4 m_invertedViewProjectionJitter ANKI_CPP_CODE(= Mat4::getIdentity()); ///< To unproject in world space.
	Mat4 m_invertedViewProjection ANKI_CPP_CODE(= Mat4::getIdentity());
	Mat4 m_invertedProjectionJitter ANKI_CPP_CODE(= Mat4::getIdentity()); ///< To unproject in view space.
	Mat4 m_invertedView ANKI_CPP_CODE(= Mat4::getIdentity());

	/// It's being used to reproject a clip space position of the current frame to the previous frame. Its value should
	/// be m_jitter * m_prevFrame.m_viewProjection * m_invertedViewProjectionJitter. At first it unprojects the current
	/// position to world space, all fine here. Then it projects to the previous frame as if the previous frame was
	/// using the current frame's jitter matrix.
	Mat4 m_reprojection ANKI_CPP_CODE(= Mat4::getIdentity());

	/// To unproject to view space. Jitter not considered.
	/// @code
	/// const F32 z = m_unprojectionParameters.z / (m_unprojectionParameters.w + depth);
	/// const Vec2 xy = ndc * m_unprojectionParameters.xy * z;
	/// pos = Vec3(xy, z);
	/// @endcode
	Vec4 m_unprojectionParameters ANKI_CPP_CODE(= Vec4(0.0f));
};
const U32 _ANKI_SIZEOF_CommonMatrices =
	12u * ANKI_SIZEOF(Mat4) + ANKI_SIZEOF(Vec4) + ANKI_SIZEOF(Mat3) + ANKI_SIZEOF(F32) * 3u;
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
	U32 m_tileSize;
	U32 m_lightVolumeLastZSplit;

	/// This are some additive counts used to map a flat index to the index of the specific object.
	U32 m_objectCountsUpTo[CLUSTER_OBJECT_TYPE_COUNT];

	U32 m_padding;

	CommonMatrices m_matrices;
	CommonMatrices m_previousMatrices;

	DirectionalLight m_directionalLight;
};
const U32 _ANKI_SIZEOF_ClusteredShadingUniforms =
	28u * ANKI_SIZEOF(U32) + 2u * ANKI_SIZEOF(CommonMatrices) + ANKI_SIZEOF(DirectionalLight);
ANKI_SHADER_STATIC_ASSERT(sizeof(ClusteredShadingUniforms) == _ANKI_SIZEOF_ClusteredShadingUniforms);

// Define the type of some cluster object masks
#if !defined(__cplusplus)
#	if ANKI_CLUSTERED_SHADING_USE_64BIT
#		define ExtendedClusterObjectMask U64
#	else
#		define ExtendedClusterObjectMask U32
#	endif
#else
#	if ANKI_CLUSTERED_SHADING_USE_64BIT
using ExtendedClusterObjectMask = U64;
#	else
using ExtendedClusterObjectMask = U32;
#	endif
#endif

/// Information that a tile or a Z-split will contain.
struct Cluster
{
	ExtendedClusterObjectMask m_pointLightsMask;
	ExtendedClusterObjectMask m_spotLightsMask;
	ExtendedClusterObjectMask m_decalsMask;
	U32 m_fogDensityVolumesMask;
	U32 m_reflectionProbesMask;
	U32 m_giProbesMask;
#if ANKI_CLUSTERED_SHADING_USE_64BIT
	U32 m_padding; ///< Add some padding to be 100% sure nothing will break.
#endif
};

#if ANKI_CLUSTERED_SHADING_USE_64BIT
const U32 _ANKI_SIZEOF_Cluster = 5u * ANKI_SIZEOF(U64);
ANKI_SHADER_STATIC_ASSERT(sizeof(Cluster) == _ANKI_SIZEOF_Cluster);
#else
const U32 _ANKI_SIZEOF_Cluster = 6u * ANKI_SIZEOF(U32);
ANKI_SHADER_STATIC_ASSERT(sizeof(Cluster) == _ANKI_SIZEOF_Cluster);
#endif

ANKI_END_NAMESPACE

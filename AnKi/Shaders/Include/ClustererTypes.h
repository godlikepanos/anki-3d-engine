// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

const U32 CLUSTER_OBJECT_TYPE_COUNT = 6u; ///< Point lights, spot lights, refl probes, GI probes, decals and fog volumes
const F32 LIGHT_FRUSTUM_NEAR_PLANE = 0.1f / 4.0f; ///< The near plane on the shadow map frustums.
const U32 MAX_SHADOW_CASCADES = 4u;

// Limits
const U32 MAX_VISIBLE_POINT_LIGHTS = 64u;
const U32 MAX_VISIBLE_SPOT_LIGHTS = 64u;
const U32 MAX_VISIBLE_DECALS = 64u;
const U32 MAX_VISIBLE_FOG_DENSITY_VOLUMES = 32u;
const U32 MAX_VISIBLE_REFLECTION_PROBES = 32u;
const U32 MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES = 8u;

/// Point light.
struct PointLight
{
	Vec3 position; ///< Position in world space.
	Vec3 diffuseColor;
	F32 radius; ///< Radius
	F32 squareRadiusOverOne; ///< 1/(radius^2).
	U32 shadowLayer; ///< Shadow layer used in RT shadows.
	F32 shadowAtlasTileScale; ///< UV scale for all tiles.
	Vec2 shadowAtlasTileOffsets[6u];
};
const U32 _ANKI_SIZEOF_PointLight = 22u * ANKI_SIZEOF(U32);
ANKI_SHADER_STATIC_ASSERT(sizeof(PointLight) == _ANKI_SIZEOF_PointLight);

/// Spot light.
struct SpotLight
{
	Vec3 position; ///< Position in world space.
	Vec3 diffuseColor;
	F32 radius; ///< Max distance.
	F32 squareRadiusOverOne; ///< 1/(radius^2).
	U32 shadowLayer; ///< Shadow layer used in RT shadows.
	Vec3 direction; ///< Light direction.
	F32 outerCos;
	F32 innerCos;
	Vec2 padding;
	Mat4 textureProjectionMatrix;
};
const U32 _ANKI_SIZEOF_SpotLight = 16u * ANKI_SIZEOF(U32) + ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(SpotLight) == _ANKI_SIZEOF_SpotLight);

/// Directional light (sun).
struct DirectionalLight
{
	Vec3 diffuseColor;
	U32 cascadeCount; ///< If it's zero then it doesn't cast shadow.
	Vec3 direction;
	U32 active;
	F32 effectiveShadowDistance;
	F32 shadowCascadesDistancePower;
	U32 shadowLayer;
	U32 padding;
	Mat4 textureMatrices[MAX_SHADOW_CASCADES];
};
const U32 _ANKI_SIZEOF_DirectionalLight = 12u * ANKI_SIZEOF(U32) + MAX_SHADOW_CASCADES * ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(DirectionalLight) == _ANKI_SIZEOF_DirectionalLight);

/// Representation of a reflection probe.
struct ReflectionProbe
{
	Vec3 position; ///< Position of the probe in world space.
	F32 cubemapIndex; ///< Index in the cubemap array texture.
	Vec3 aabbMin;
	Vec3 aabbMax;
};
const U32 _ANKI_SIZEOF_ReflectionProbe = 10u * ANKI_SIZEOF(U32);
ANKI_SHADER_STATIC_ASSERT(sizeof(ReflectionProbe) == _ANKI_SIZEOF_ReflectionProbe);

/// Decal.
struct Decal
{
	Vec4 diffuseUv;
	Vec4 normRoughnessUv;
	Mat4 textureProjectionMatrix;
	Vec4 blendFactors;
};
const U32 _ANKI_SIZEOF_Decal = 3u * ANKI_SIZEOF(Vec4) + ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(Decal) == _ANKI_SIZEOF_Decal);

/// Fog density volume.
struct FogDensityVolume
{
	Vec3 aabbMinOrSphereCenter;
	U32 isBox;
	Vec3 aabbMaxOrSphereRadiusSquared;
	F32 density;
};
const U32 _ANKI_SIZEOF_FogDensityVolume = 2u * ANKI_SIZEOF(Vec4);
ANKI_SHADER_STATIC_ASSERT(sizeof(FogDensityVolume) == _ANKI_SIZEOF_FogDensityVolume);

/// Global illumination probe
struct GlobalIlluminationProbe
{
	Vec3 aabbMin;
	Vec3 aabbMax;

	U32 textureIndex; ///< Index to the array of volume textures.
	F32 halfTexelSizeU; ///< (1.0 / textureSize(texArr[textureIndex]).x) / 2.0

	/// Used to calculate a factor that is zero when fragPos is close to AABB bounds and 1.0 at fadeDistance and less.
	F32 fadeDistance;
};
const U32 _ANKI_SIZEOF_GlobalIlluminationProbe = 9u * ANKI_SIZEOF(U32);
ANKI_SHADER_STATIC_ASSERT(sizeof(GlobalIlluminationProbe) == _ANKI_SIZEOF_GlobalIlluminationProbe);

/// Common matrices.
class CommonMatrices
{
public:
	Mat4 cameraTransform;
	Mat4 view;
	Mat4 projection;
	Mat4 viewProjection;

	Mat4 jitter;
	Mat4 projectionJitter;
	Mat4 viewProjectionJitter;

	Mat4 invertedViewProjectionJitter;
};
const U32 _ANKI_SIZEOF_CommonMatrices = 9u * ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(CommonMatrices) == _ANKI_SIZEOF_CommonMatrices);

/// Common uniforms for light shading passes.
struct LightingUniforms
{
	Vec2 rendereringSize;

	F32 time;
	U32 frameCount;

	F32 near;
	F32 far;
	Vec3 cameraPosition;

	UVec3 clusterCounts;
	U32 lightVolumeLastCluster;

	U32 frameCount;

	Vec2 padding;

	CommonMatrices matrices;
	CommonMatrices previousFrameMatrices;
};
const U32 _ANKI_SIZEOF_LightingUniforms = 16u * ANKI_SIZEOF(U32) + 2u * ANKI_SIZEOF(CommonMatrices);
ANKI_SHADER_STATIC_ASSERT(sizeof(LightingUniforms) == _ANKI_SIZEOF_LightingUniforms);

/// Information that a tile or a Z-split will contain.
struct Tile
{
	U64 pointLightsMask;
	U64 spotLightsMask;
	U64 decalsMask;
	U32 fogDensityVolumesMask;
	U32 reflectionProbesMask;
	U8 giProbesMask;
};
const U32 _ANKI_SIZEOF_Tile = 3u * ANKI_SIZEOF(U64) + 2u * ANKI_SIZEOF(U32) + ANKI_SIZEOF(U8);
ANKI_SHADER_STATIC_ASSERT(sizeof(Tile) == _ANKI_SIZEOF_Tile);

ANKI_END_NAMESPACE

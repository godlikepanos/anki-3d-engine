// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/GpuSceneTypes.h>

ANKI_BEGIN_NAMESPACE

// Limits
constexpr U32 kMaxVisibleLights = 32u * 4u;
constexpr U32 kMaxVisibleDecals = 32u * 4u;
constexpr U32 kMaxVisibleFogDensityVolumes = 16u;
constexpr U32 kMaxVisibleReflectionProbes = 16u;
constexpr U32 kMaxVisibleGlobalIlluminationProbes = 8u;

// Other consts
constexpr RF32 kClusterObjectFrustumNearPlane = 0.1f / 4.0f; ///< Near plane of all clusterer object frustums.
constexpr RF32 kSubsurfaceMin = 0.01f;
constexpr U32 kMaxZsplitCount = 128u;
constexpr U32 kClusteredShadingTileSize = 64; ///< The size of the tile in clustered shading.

/// A union of all fields of spot and point lights. Since we don't have unions in HLSL we had to get creative.
struct LightUnion
{
	Vec3 m_position; ///< Position in world space.
	RF32 m_radius; ///< Radius

	RVec3 m_diffuseColor;
	U32 m_lightType; ///< 0 is point and 1 is spot

	U32 m_shadow;
	F32 m_innerCos; ///< SPOT LIGHTS
	F32 m_outerCos; ///< SPOT LIGHTS
	F32 m_padding;

	RVec3 m_direction; ///< SPOT LIGHTS: Light direction.
	F32 m_shadowAtlasTileScale; ///< POINT LIGHTS: UV scale for all tiles.

	/// When it's a point light this is an array of 6 Vec2s (but because of padding it's actually Vec4s). When it's a spot light the first 4 Vec4s are
	/// the rows of the texture matrix
	Vec4 m_spotLightMatrixOrPointLightUvViewports[6u];
};

/// Point light.
/// WARNING: Don't change the layout without looking at LightUnion.
struct PointLight
{
	Vec3 m_position; ///< Position in world space.
	RF32 m_radius; ///< Radius

	RVec3 m_diffuseColor;
	U32 m_padding1;

	U32 m_shadow;
	F32 m_padding2;
	U32 m_padding3;
	U32 m_padding4;

	RVec3 m_padding5;
	F32 m_shadowAtlasTileScale; ///< UV scale for all tiles.

	Vec4 m_shadowAtlasTileOffsets[6u]; ///< It's a array of Vec2 but because of padding round it up.
};
static_assert(sizeof(PointLight) == sizeof(LightUnion));

/// Spot light.
/// WARNING: Don't change the layout without looking at LightUnion.
struct SpotLight
{
	Vec3 m_position; ///< Position in world space.
	RF32 m_radius; ///< Radius

	RVec3 m_diffuseColor;
	U32 m_padding1;

	U32 m_shadow;
	F32 m_innerCos;
	F32 m_outerCos;
	F32 m_padding2;

	RVec3 m_direction;
	F32 m_padding3;

	Mat4 m_textureMatrix;

	Vec4 m_padding4[2];
};
static_assert(sizeof(SpotLight) == sizeof(LightUnion));

/// Representation of a reflection probe.
typedef GpuSceneReflectionProbe ReflectionProbe;

/// Decal.
typedef GpuSceneDecal Decal;

/// Fog density volume.
typedef GpuSceneFogDensityVolume FogDensityVolume;

/// Global illumination probe
typedef GpuSceneGlobalIlluminationProbe GlobalIlluminationProbe;

/// Information that a tile or a Z-split will contain.
struct Cluster
{
	U32 m_pointLightsMask[kMaxVisibleLights / 32];
	U32 m_spotLightsMask[kMaxVisibleLights / 32];
	U32 m_decalsMask[kMaxVisibleDecals / 32];
	U32 m_fogDensityVolumesMask;
	U32 m_reflectionProbesMask;
	U32 m_giProbesMask;
};

constexpr ANKI_ARRAY(U32, GpuSceneNonRenderableObjectType::kCount, kClusteredObjectSizes) = {
	sizeof(LightUnion), sizeof(Decal), sizeof(FogDensityVolume), sizeof(ReflectionProbe), sizeof(GlobalIlluminationProbe)};

constexpr ANKI_ARRAY(U32, GpuSceneNonRenderableObjectType::kCount, kMaxVisibleClusteredObjects) = {
	kMaxVisibleLights, kMaxVisibleDecals, kMaxVisibleFogDensityVolumes, kMaxVisibleReflectionProbes, kMaxVisibleGlobalIlluminationProbes};

ANKI_END_NAMESPACE

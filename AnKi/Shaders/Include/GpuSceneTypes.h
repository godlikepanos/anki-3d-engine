// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/MeshTypes.h>

ANKI_BEGIN_NAMESPACE

/// @note All offsets in bytes
struct GpuSceneRenderable
{
	U32 m_worldTransformsOffset; ///< First is the crnt transform and the 2nd the previous
	U32 m_uniformsOffset;
	U32 m_geometryOffset; ///< Points to a GpuSceneMeshLod or a GpuSceneParticleEmitter
	U32 m_boneTransformsOffset;
};
static_assert(sizeof(GpuSceneRenderable) == sizeof(Vec4) * 1);

typedef UVec4 GpuSceneRenderablePacked;

/// Used in visibility testing.
struct GpuSceneRenderableAabb
{
	Vec3 m_sphereCenter;
	F32 m_negativeSphereRadius;

	Vec3 m_aabbExtend;
	/// High 20bits point to a GpuSceneRenderable. Rest 12bits are the render state bucket idx
	U32 m_renderableIndexAndRenderStateBucket;
};
static_assert(sizeof(GpuSceneRenderableAabb) == sizeof(Vec4) * 2);

/// Represents the geometry data of a single LOD of an indexed mesh.
struct GpuSceneMeshLod
{
	U32 m_vertexOffsets[(U32)VertexStreamId::kMeshRelatedCount];
	U32 m_indexCount;
	U32 m_firstIndex; // In sizeof(indexType)

	Vec3 m_positionTranslation;
	F32 m_positionScale;
};
static_assert(sizeof(GpuSceneMeshLod) == sizeof(Vec4) * 3);

struct GpuSceneParticleEmitter
{
	U32 m_vertexOffsets[(U32)VertexStreamId::kParticleRelatedCount];
	U32 m_padding0;
};
static_assert(sizeof(GpuSceneParticleEmitter) == sizeof(Vec4) * 2);

/// Point light.
struct GpuScenePointLight
{
	Vec3 m_position; ///< Position in world space.
	RF32 m_radius; ///< Radius

	RVec3 m_diffuseColor;
	RF32 m_squareRadiusOverOne; ///< 1/(radius^2).

	U32 m_shadow;
	U32 m_uuid;
	U32 m_padding0;
	U32 m_padding1;
};
constexpr U32 kSizeof_GpuScenePointLight = 3u * sizeof(Vec4);
static_assert(sizeof(GpuScenePointLight) == kSizeof_GpuScenePointLight);

/// Spot light.
struct GpuSceneSpotLight
{
	Vec3 m_position;
	F32 m_padding0;

	Vec4 m_edgePoints[4u]; ///< Edge points in world space.

	RVec3 m_diffuseColor;
	RF32 m_radius; ///< Max distance.

	RVec3 m_direction; ///< Light direction.
	RF32 m_squareRadiusOverOne; ///< 1/(radius^2).

	U32 m_shadow;
	RF32 m_outerCos;
	RF32 m_innerCos;
	U32 m_uuid;
};
constexpr U32 kSizeof_GpuSceneSpotLight = 8u * sizeof(Vec4);
static_assert(sizeof(GpuSceneSpotLight) == kSizeof_GpuSceneSpotLight);

/// Representation of a reflection probe.
struct GpuSceneReflectionProbe
{
	Vec3 m_position; ///< Position of the probe in world space.
	U32 m_cubeTexture; ///< Bindless index of the reflection texture.

	Vec3 m_aabbMin;
	F32 m_uuid;

	Vec3 m_aabbMax;
	F32 m_padding1;
};
constexpr U32 kSizeof_GpuSceneReflectionProbe = 3u * sizeof(Vec4);
static_assert(sizeof(GpuSceneReflectionProbe) == kSizeof_GpuSceneReflectionProbe);

/// Global illumination probe
struct GpuSceneGlobalIlluminationProbe
{
	Vec3 m_aabbMin;
	F32 m_uuid;

	Vec3 m_aabbMax;
	F32 m_padding1;

	U32 m_volumeTexture; ///< Bindless index of the irradiance volume texture.
	F32 m_halfTexelSizeU; ///< (1.0 / textureSize(texArr[textureIndex]).x) / 2.0
	/// Used to calculate a factor that is zero when fragPos is close to AABB bounds and 1.0 at fadeDistance and less.
	RF32 m_fadeDistance;
	F32 m_padding2;
};
constexpr U32 kSizeof_GpuSceneGlobalIlluminationProbe = 3u * sizeof(Vec4);
static_assert(sizeof(GpuSceneGlobalIlluminationProbe) == kSizeof_GpuSceneGlobalIlluminationProbe);

/// Decal.
struct GpuSceneDecal
{
	U32 m_diffuseTexture;
	U32 m_roughnessMetalnessTexture;
	RF32 m_diffuseBlendFactor;
	RF32 m_roughnessMetalnessFactor;

	Mat4 m_textureMatrix;

	Mat4 m_invertedTransform;

	Vec3 m_obbExtend;
	F32 m_padding0;

	Vec4 m_boundingSphere;
};
constexpr U32 kSizeof_GpuSceneDecal = 3u * sizeof(Vec4) + 2u * sizeof(Mat4);
static_assert(sizeof(GpuSceneDecal) == kSizeof_GpuSceneDecal);

/// Fog density volume.
struct GpuSceneFogDensityVolume
{
	Vec3 m_aabbMinOrSphereCenter;
	U32 m_isBox;

	Vec3 m_aabbMaxOrSphereRadius;
	RF32 m_density;
};
constexpr U32 kSizeof_GpuSceneFogDensityVolume = 2u * sizeof(Vec4);
static_assert(sizeof(GpuSceneFogDensityVolume) == kSizeof_GpuSceneFogDensityVolume);

enum class GpuSceneNonRenderableObjectType : U32
{
	kPointLight,
	kSpotLight,
	kDecal,
	kFogDensityVolume,
	kReflectionProbe,
	kGlobalIlluminationProbe,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GpuSceneNonRenderableObjectType)

#define ANKI_GPU_SCENE_NON_RENDERABLE_OBJECT_TYPE_POINT_LIGHT 0
#define ANKI_GPU_SCENE_NON_RENDERABLE_OBJECT_TYPE_SPOT_LIGHT 1
#define ANKI_GPU_SCENE_NON_RENDERABLE_OBJECT_TYPE_DECAL 2
#define ANKI_GPU_SCENE_NON_RENDERABLE_OBJECT_TYPE_FOG_DENSITY_VOLUME 3
#define ANKI_GPU_SCENE_NON_RENDERABLE_OBJECT_TYPE_REFLECTION_PROBE 4
#define ANKI_GPU_SCENE_NON_RENDERABLE_OBJECT_TYPE_GLOBAL_ILLUMINATION_PROBE 5

enum class GpuSceneNonRenderableObjectTypeBit : U32
{
	kNone = 0,

	kPointLight = 1 << 0,
	kSpotLight = 1 << 1,
	kDecal = 1 << 2,
	kFogDensityVolume = 1 << 3,
	kReflectionProbe = 1 << 4,
	kGlobalIlluminationProbe = 1 << 5,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GpuSceneNonRenderableObjectTypeBit)

ANKI_END_NAMESPACE

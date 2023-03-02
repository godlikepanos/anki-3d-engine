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
	U32 m_geometryOffset;
	U32 m_boneTransformsOffset;
};
static_assert(sizeof(GpuSceneRenderable) == sizeof(Vec4) * 1);

typedef UVec4 GpuSceneRenderablePacked;

struct GpuSceneMeshLod
{
	U32 m_vertexOffsets[(U32)VertexStreamId::kMeshRelatedCount];
	U32 m_indexCount;
	U32 m_indexBufferOffset; // In sizeof(indexType)

	Vec3 m_positionTranslation;
	F32 m_positionScale;
};
static_assert(sizeof(GpuSceneMeshLod) == sizeof(Vec4) * 3);

struct GpuSceneParticleEmitter
{
	U32 m_vertexOffsets[(U32)VertexStreamId::kParticleRelatedCount];
	U32 m_padding0;
	U32 m_padding1;
};
static_assert(sizeof(GpuSceneParticleEmitter) == sizeof(Vec4) * 2);

/// Point light.
struct GpuScenePointLight
{
	Vec3 m_position; ///< Position in world space.
	RF32 m_radius; ///< Radius

	RVec3 m_diffuseColor;
	RF32 m_squareRadiusOverOne; ///< 1/(radius^2).

	Vec2 m_padding0;
	U32 m_shadowLayer; ///< Shadow layer used in RT shadows. Also used to show that it doesn't cast shadow.
	F32 m_shadowAtlasTileScale; ///< UV scale for all tiles.

	Vec4 m_shadowAtlasTileOffsets[6u]; ///< It's a array of Vec2 but because of padding round it up.
};
constexpr U32 kSizeof_GpuScenePointLight = 9u * sizeof(Vec4);
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

	U32 m_shadowLayer; ///< Shadow layer used in RT shadows. Also used to show that it doesn't cast shadow.
	RF32 m_outerCos;
	RF32 m_innerCos;
	U32 m_padding1;

	Mat4 m_textureMatrix;
};
constexpr U32 kSizeof_GpuSceneSpotLight = 12u * sizeof(Vec4);
static_assert(sizeof(GpuSceneSpotLight) == kSizeof_GpuSceneSpotLight);

struct RenderableGpuView
{
	Mat3x4 m_worldTransform;
	Mat3x4 m_previousWorldTransform;
	Vec4 m_positionScaleF32AndTranslationVec3; ///< The scale and translation that uncompress positions.
};

ANKI_END_NAMESPACE

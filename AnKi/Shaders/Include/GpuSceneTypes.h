// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/MeshTypes.h>

ANKI_BEGIN_NAMESPACE

/// @note All offsets in bytes
struct RenderableGpuView2
{
	U32 m_worldTransformsOffset; ///< First is the crnt transform and the 2nd the previous
	U32 m_aabbOffset;
	U32 m_uniformsOffset;
	U32 m_meshOffset;
	U32 m_boneTransformsOffset;
	U32 m_previousBoneTransformsOffset;
};

struct MeshGpuViewLod
{
	U32 m_vertexOffsets[(U32)VertexStreamId::kMeshRelatedCount];
	U32 m_indexCount;
	U32 m_indexOffset; // TODO Decide on its type
};
static_assert(sizeof(MeshGpuViewLod) == sizeof(Vec4) * 2);

struct MeshGpuView
{
	MeshGpuViewLod m_lods[kMaxLodCount];

	Vec3 m_positionTranslation;
	F32 m_positionScale;
};

struct UnpackedRenderableGpuViewInstance
{
	U32 m_renderableGpuViewOffset;
	U32 m_lod;
};

typedef U32 PackedRenderableGpuViewInstance;

struct RenderableGpuView
{
	Mat3x4 m_worldTransform;
	Mat3x4 m_previousWorldTransform;
	Vec4 m_positionScaleF32AndTranslationVec3; ///< The scale and translation that uncompress positions.
};

ANKI_END_NAMESPACE

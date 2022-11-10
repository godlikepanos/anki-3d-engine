// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/MeshTypes.h>

ANKI_BEGIN_NAMESPACE

struct RenderableGpuView2
{
	U32 m_worldTransformOffset;
	U32 m_previousWorldTransformOffset;
	U32 m_aabbOffset;
	U32 m_uniformsOffset;
	U32 m_meshOffset;
	U32 m_boneTransformsOffset;
};

struct MeshGpuView
{
	Vec3 m_positionScale;
	F32 m_positionTranslation;

	U32 m_vertexOffsets[kMaxLodCount][kMaxVertexStreamIds];
	U32 m_indexCounts[kMaxLodCount];
	U32 m_indexOffsets[kMaxLodCount];
};

struct RenderableGpuView
{
	Mat3x4 m_worldTransform;
	Mat3x4 m_previousWorldTransform;
	Vec4 m_positionScaleF32AndTranslationVec3; ///< The scale and translation that uncompress positions.
};

struct SkinGpuView
{
	U32 m_tmp;
};

ANKI_END_NAMESPACE

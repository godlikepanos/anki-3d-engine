// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/MeshTypes.h>

ANKI_BEGIN_NAMESPACE

struct RenderableGpuView2
{
	U32 m_worldTransformsOffset; ///< First is the crnt transform and the 2nd the previous
	U32 m_aabbOffset;
	U32 m_uniformsOffset;
	U32 m_meshOffset;
	U32 m_boneTransformsOffset;
	U32 m_previousBoneTransformsOffset;
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

ANKI_END_NAMESPACE

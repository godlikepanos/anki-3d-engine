// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/MeshTypes.h>

ANKI_BEGIN_NAMESPACE

/// Offset in DWORDs
#if defined(__cplusplus)
using DwordOffset = U32;
#else
#	define DwordOffset U32
#endif

/// @note All offsets in DWORD
struct RenderableGpuView2
{
	DwordOffset m_worldTransformsOffset; ///< First is the crnt transform and the 2nd the previous
	DwordOffset m_aabbOffset;
	DwordOffset m_uniformsOffset;
	DwordOffset m_meshOffset;
	DwordOffset m_boneTransformsOffset;
	DwordOffset m_previousBoneTransformsOffset;
};

struct MeshGpuView
{
	Vec3 m_positionTranslation;
	F32 m_positionScale;

#if ANKI_GLSL
	U32 m_vertexOffsets[kMaxLodCount][kMaxVertexStreamIds];
#else
	U32 m_vertexOffsets[kMaxLodCount][(U32)VertexStreamId::kCount];
#endif
	U32 m_indexCounts[kMaxLodCount];
	U32 m_indexOffsets[kMaxLodCount];
};

struct RenderableGpuView
{
	Vec4 m_worldTransform[3u];
	Vec4 m_previousWorldTransform[3u];
	Vec4 m_positionScaleF32AndTranslationVec3; ///< The scale and translation that uncompress positions.
};

ANKI_END_NAMESPACE

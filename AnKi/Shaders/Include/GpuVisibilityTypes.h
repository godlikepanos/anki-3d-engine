// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

struct GpuVisibilityUniforms
{
	Vec4 m_clipPlanes[6u];

	UVec3 m_padding1;
	U32 m_aabbCount;

	Vec4 m_maxLodDistances;

	Vec3 m_lodReferencePoint;
	F32 m_padding2;

	Mat4 m_viewProjectionMat;
};

struct PointLightRendererCacheEntry
{
	U32 m_shadowLayer; ///< Shadow layer used in RT shadows. Also used to show that it doesn't cast shadow.
	F32 m_shadowAtlasTileScale; ///< UV scale for all tiles.
	U32 m_uuid;
	F32 m_padding0;

	Vec4 m_shadowAtlasTileOffsets[6u]; ///< It's a array of Vec2 but because of padding round it up.
};

struct SpotLightRendererCacheEntry
{
	U32 m_shadowLayer; ///< Shadow layer used in RT shadows. Also used to show that it doesn't cast shadow.
	U32 m_uuid;
	U32 m_padding0;
	U32 m_padding1;

	Mat4 m_textureMatrix;
};

ANKI_END_NAMESPACE

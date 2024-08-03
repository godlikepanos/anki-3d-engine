// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

struct FrustumGpuVisibilityUniforms
{
	Vec4 m_clipPlanes[6u];

	Vec4 m_maxLodDistances;

	Vec3 m_lodReferencePoint;
	F32 m_padding3;

	Mat4 m_viewProjectionMat;

	Vec2 m_finalRenderTargetSize;
};

struct DistanceGpuVisibilityUniforms
{
	Vec3 m_pointOfTest;
	F32 m_testRadius;

	Vec4 m_maxLodDistances;

	Vec3 m_lodReferencePoint;
	F32 m_padding3;
};

struct GpuVisibilityNonRenderableUniforms
{
	Vec4 m_clipPlanes[6u];
};

struct GpuVisibilityAccelerationStructuresUniforms
{
	Vec4 m_clipPlanes[6u];

	Vec3 m_pointOfTest;
	F32 m_testRadius;

	Vec4 m_maxLodDistances;
};

struct GpuVisibilityHash
{
	U32 m_renderablesHash;
	U32 m_containsDeformable;
};

struct GpuVisibilityVisibleRenderableDesc
{
	U32 m_lod_2bit_renderableIndex_20bit_renderStageBucket_10bit;
};

struct GpuVisibilityVisibleMeshletDesc
{
	U32 m_renderableIndex_30bit_renderStageBucket_12bit;
	U32 m_lod_2bit_meshletIndex_30bit;
};

ANKI_END_NAMESPACE

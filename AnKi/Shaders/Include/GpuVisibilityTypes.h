// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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

ANKI_END_NAMESPACE

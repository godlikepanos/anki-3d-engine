// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

struct FrustumGpuVisibilityConsts
{
	Vec4 m_clipPlanes[6u];

	Vec4 m_maxLodDistances;

	Vec3 m_lodReferencePoint;
	F32 m_padding3;

	Mat4 m_viewProjectionMat;

	Vec2 m_finalRenderTargetSize;
};

struct DistanceGpuVisibilityConstants
{
	Vec3 m_pointOfTest;
	F32 m_testRadius;

	Vec4 m_maxLodDistances;

	Vec3 m_lodReferencePoint;
	F32 m_padding3;
};

struct GpuVisibilityNonRenderableConstants
{
	Vec4 m_clipPlanes[6u];
};

struct GpuVisibilityAccelerationStructuresConstants
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
	U32 m_lod : 2;
	U32 m_renderableIndex : 20;
	U32 m_renderStateBucket : 10;
};

struct GpuVisibilityVisibleMeshletDesc
{
	U32 m_renderableIndex : 30;
	U32 m_renderStateBucket : 12;

	U32 m_lod : 2;
	U32 m_meshletIndex : 30;
};

struct GpuVisibilityMeshletConstants
{
	Mat4 m_viewProjectionMatrix;

	Vec3 m_cameraPos;
	U32 m_padding1;

	Vec2 m_viewportSizef;
	UVec2 m_padding2;
};

enum class GpuVisibilityCounter : U32
{
	kVisibleRenderableCount,
	kMeshletsSurvivingStage1Count,
	kThreadgroupCount,
	kMeshletsCulledByHzbCount,

	kCount
};

enum class GpuVisibilityIndirectDispatches : U32
{
	k2ndStageLegacy,
	k2ndStageMeshlets,
	k3rdStageMeshlets,

	kCount
};

// Counters used in non-renderables visibility
struct GpuVisibilityNonRenderablesCounters
{
	U32 m_threadgroupCount; ///< Counts the no of threadgroups
	U32 m_visibleObjectCount; ///< Counts the visible objects
	U32 m_feedbackObjectCount; ///< Counts the visbile objects that need feedback
};

// Packs the LOD in the MSB 3bit and the rest it's an index to a GpuSceneRenderableBoundingVolume. It's actually U32 because of some shader logic
typedef U32 LodAndGpuSceneRenderableBoundingVolumeIndex;

struct GpuVisibilityLocalLightsConsts
{
	Vec3 m_cellSize;
	U32 m_maxLightIndices;

	Vec3 m_gridVolumeMin;
	F32 m_padding2;

	Vec3 m_gridVolumeSize;
	F32 m_padding3;

	UVec3 m_cellCounts;
	U32 m_cellCount;
};

ANKI_END_NAMESPACE

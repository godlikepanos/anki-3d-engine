// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

/// Common data for all materials.
struct MaterialGlobalUniforms
{
	Mat4 m_viewProjectionMatrix;
	Mat4 m_previousViewProjectionMatrix;
	Mat3x4 m_viewTransform;
	Mat3x4 m_cameraTransform;

	Vec4 m_viewport;

	U32 m_enableHzbTesting;
	U32 m_padding0;
	U32 m_padding1;
	U32 m_padding2;
};
static_assert(sizeof(MaterialGlobalUniforms) == 16 * sizeof(Vec4));

/// @brief
enum class MaterialSet : U32
{
	kBindless,
	kGlobal
};

/// Bindings in the MaterialSet::kGlobal descriptor set.
enum class MaterialBinding : U32
{
	kTrilinearRepeatSampler,
	kGlobalUniforms,
	kGpuScene,

	// Texture buffer bindings pointing to unified geom buffer:
#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType) kUnifiedGeometry_##fmt,
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.def.h>

	kMeshletBoundingVolumes, ///< Points to the unified geom buffer
	kMeshletGeometryDescriptors, ///< Points to the unified geom buffer
	kMeshletGroups,
	kRenderables,
	kMeshLods,
	kTransforms,
	kHzbTexture,
	kNearestClampSampler,

	// For FW shading:
	kLinearClampSampler,
	kShadowSampler,
	kDepthRt,
	kLightVolume,
	kClusterShadingUniforms,
	kClusterShadingLights,
	kClusters = kClusterShadingLights + 2,
};

ANKI_END_NAMESPACE

// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

/// Common data for all materials.
struct MaterialGlobalConstants
{
	Mat4 m_viewProjectionMatrix;
	Mat4 m_previousViewProjectionMatrix;
	Mat3x4 m_viewTransform;
	Mat3x4 m_cameraTransform;
};
static_assert(sizeof(MaterialGlobalConstants) == 14 * sizeof(Vec4));

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
	kGlobalConstants,
	kGpuScene,

	// Texture buffer bindings pointing to unified geom buffer:
#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType) kUnifiedGeometry_##fmt,
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.defs.h>

	kMeshlets, // Pointing to the unified geom buffer
	kTaskShaderPayloads,

	// For FW shading:
	kLinearClampSampler,
	kShadowSampler,
	kDepthRt,
	kLightVolume,
	kClusterShadingConstants,
	kClusterShadingLights,
	kClusters = kClusterShadingLights + 2,

	kCount,
	kFirst = 0
};

// Techniques
#define ANKI_RENDERING_TECHNIQUE_GBUFFER 0
#define ANKI_RENDERING_TECHNIQUE_DEPTH 1
#define ANKI_RENDERING_TECHNIQUE_FORWARD 2

ANKI_END_NAMESPACE

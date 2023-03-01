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
};
ANKI_SHADER_STATIC_ASSERT(sizeof(MaterialGlobalUniforms) == 14 * sizeof(Vec4));

constexpr U32 kMaterialSetBindless = 0u;
constexpr U32 kMaterialSetGlobal = 1u;
constexpr U32 kMaterialSetLocal = 2u;

// Begin global bindings
constexpr U32 kMaterialBindingTrilinearRepeatSampler = 0u;
constexpr U32 kMaterialBindingGlobalUniforms = 1u;
constexpr U32 kMaterialBindingGpuScene = 2u;

// For forward shading:
constexpr U32 kMaterialBindingLinearClampSampler = 3u;
constexpr U32 kMaterialBindingDepthRt = 4u;
constexpr U32 kMaterialBindingLightVolume = 5u;
constexpr U32 kMaterialBindingClusterShadingUniforms = 6u;
constexpr U32 kMaterialBindingClusterShadingLights = 7u;
constexpr U32 kMaterialBindingClusters = 8u;
constexpr U32 kMaterialBindingShadowSampler = 9u;
// End global bindings

// Begin local bindings
constexpr U32 kMaterialBindingLocalUniforms = 0u;
constexpr U32 kMaterialBindingRenderableGpuView = 1u;
constexpr U32 kMaterialBindingBoneTransforms = 2u;
constexpr U32 kMaterialBindingPreviousBoneTransforms = 3u;
constexpr U32 kMaterialBindingFirstNonStandardLocal = 4u;
// End local bindings

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
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.defs.h>

	// For FW shading:
	kLinearClampSampler,
	kShadowSampler,
	kDepthRt,
	kLightVolume,
	kClusterShadingUniforms,
	kClusterShadingLights,
	kClusters = kClusterShadingLights + 3,

	kCount,
	kFirst = 0
};

// Techniques
#define ANKI_RENDERING_TECHNIQUE_GBUFFER 0
#define ANKI_RENDERING_TECHNIQUE_GBUFFER_EZ 1
#define ANKI_RENDERING_TECHNIQUE_SHADOWS 2
#define ANKI_RENDERING_TECHNIQUE_FORWARD 3

ANKI_END_NAMESPACE

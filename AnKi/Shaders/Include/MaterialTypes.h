// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
	Vec4 m_viewTransform[3];
	Vec4 m_cameraTransform[3];
};
ANKI_SHADER_STATIC_ASSERT(sizeof(MaterialGlobalUniforms) == 14 * sizeof(Vec4));

constexpr U32 kMaterialSetBindless = 0u;
constexpr U32 kMaterialSetGlobal = 1u;
constexpr U32 kMaterialSetLocal = 2u;

// Begin global bindings
constexpr U32 kMaterialBindingTrilinearRepeatSampler = 0u;
constexpr U32 kMaterialBindingGlobalUniforms = 1u;

// For forward shading:
constexpr U32 kMaterialBindingLinearClampSampler = 2u;
constexpr U32 kMaterialBindingDepthRt = 3u;
constexpr U32 kMaterialBindingLightVolume = 4u;
constexpr U32 kMaterialBindingClusterShadingUniforms = 5u;
constexpr U32 kMaterialBindingClusterShadingLights = 6u;
constexpr U32 kMaterialBindingClusters = 9u;
constexpr U32 kMaterialBindingShadowSampler = 10u;
// End global bindings

// Begin local bindings
constexpr U32 kMaterialBindingLocalUniforms = 0u;
constexpr U32 kMaterialBindingRenderableGpuView = 1u;
constexpr U32 kMaterialBindingBoneTransforms = 2u;
constexpr U32 kMaterialBindingPreviousBoneTransforms = 3u;
constexpr U32 kMaterialBindingFirstNonStandardLocal = 4u;
// End local bindings

// Techniques
#define ANKI_RENDERING_TECHNIQUE_GBUFFER 0
#define ANKI_RENDERING_TECHNIQUE_GBUFFER_EZ 1
#define ANKI_RENDERING_TECHNIQUE_SHADOWS 2
#define ANKI_RENDERING_TECHNIQUE_FORWARD 3

ANKI_END_NAMESPACE

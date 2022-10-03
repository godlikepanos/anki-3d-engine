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
	Mat3x4 m_viewMatrix;
	Mat3x4 m_cameraTransform;
};
ANKI_SHADER_STATIC_ASSERT(sizeof(MaterialGlobalUniforms) == 14 * sizeof(Vec4));

const U32 kMaterialSetBindless = 0u;
const U32 kMaterialSetGlobal = 1u;
const U32 kMaterialSetLocal = 2u;

// Begin global bindings
const U32 kMaterialBindingTrilinearRepeatSampler = 0u;
const U32 kMaterialBindingGlobalUniforms = 1u;

// For forward shading:
const U32 kMaterialBindingLinearClampSampler = 2u;
const U32 kMaterialBindingDepthRt = 3u;
const U32 kMaterialBindingLightVolume = 4u;
const U32 kMaterialBindingClusterShadingUniforms = 5u;
const U32 kMaterialBindingClusterShadingLights = 6u;
const U32 kMaterialBindingClusters = 9u;
// End global bindings

// Begin local bindings
const U32 kMaterialBindingLocalUniforms = 0u;
const U32 kMaterialBindingRenderableGpuView = 1u;
const U32 kMaterialBindingBoneTransforms = 2u;
const U32 kMaterialBindingPreviousBoneTransforms = 3u;
const U32 kMaterialBindingFirstNonStandardLocal = 4u;
// End local bindings

ANKI_END_NAMESPACE

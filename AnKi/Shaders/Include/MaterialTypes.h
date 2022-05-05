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
	ScalarMat4 m_viewProjectionMatrix;
	ScalarMat3x4 m_viewMatrix;
	Mat3 m_viewRotationMatrix; ///< Essentially the rotation part of the view matrix.
	Mat3 m_cameraRotationMatrix;
};

const U32 MATERIAL_SET_BINDLESS = 0u;
const U32 MATERIAL_SET_GLOBAL = 1u;
const U32 MATERIAL_SET_LOCAL = 2u;

// Begin global bindings
const U32 MATERIAL_BINDING_TRILINEAR_REPEAT_SAMPLER = 0u;
const U32 MATERIAL_BINDING_GLOBAL_UNIFORMS = 1u;

// For forward shading:
const U32 MATERIAL_BINDING_LINEAR_CLAMP_SAMPLER = 2u;
const U32 MATERIAL_BINDING_DEPTH_RT = 3u;
const U32 MATERIAL_BINDING_LIGHT_VOLUME = 4u;
const U32 MATERIAL_BINDING_CLUSTER_SHADING_UNIFORMS = 5u;
const U32 MATERIAL_BINDING_CLUSTER_SHADING_LIGHTS = 6u;
const U32 MATERIAL_BINDING_CLUSTERS = 9u;
// End global bindings

// Begin local bindings
const U32 MATERIAL_BINDING_LOCAL_UNIFORMS = 0u;
const U32 MATERIAL_BINDING_RENDERABLE_GPU_VIEW = 1u;
const U32 MATERIAL_BINDING_BONE_TRANSFORMS = 2u;
const U32 MATERIAL_BINDING_PREVIOUS_BONE_TRANSFORMS = 3u;
const U32 MATERIAL_BINDING_FIRST_NON_STANDARD_LOCAL = 4u;
// End local bindings

ANKI_END_NAMESPACE

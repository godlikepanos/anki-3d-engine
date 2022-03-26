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

// Begin standard bindings
const U32 MATERIAL_BINDING_GLOBAL_SAMPLER = 0u;
const U32 MATERIAL_BINDING_RENDERABLE_GPU_VIEW = 1u;
const U32 MATERIAL_BINDING_LOCAL_UNIFORMS = 2u;
const U32 MATERIAL_BINDING_GLOBAL_UNIFORMS = 3u;
const U32 MATERIAL_BINDING_BONE_TRANSFORMS = 4u;
const U32 MATERIAL_BINDING_PREVIOUS_BONE_TRANSFORMS = 5u;
// End standard bindings

const U32 MATERIAL_BINDING_STANDARD_BINDING_COUNT = 6u;
const U32 MATERIAL_BINDING_FIRST_NON_STANDARD = MATERIAL_BINDING_STANDARD_BINDING_COUNT;

const U32 MATERIAL_SET_INTERNAL = 0u;
const U32 MATERIAL_SET_EXTERNAL = 1u;

#define ANKI_MATERIAL_UNIFORM_BUFFER(bindingNum, structName, varName) \
	layout(set = MATERIAL_SET_EXTERNAL, binding = bindingNum, row_major, scalar) uniform b_##varName \
	{ \
		structName varName; \
	};

#define ANKI_MATERIAL_STORAGE_BUFFER(bindingNum, structName, varName) \
	layout(set = MATERIAL_SET_EXTERNAL, binding = bindingNum, row_major, scalar) buffer b_##varName \
	{ \
		structName varName; \
	};

ANKI_END_NAMESPACE

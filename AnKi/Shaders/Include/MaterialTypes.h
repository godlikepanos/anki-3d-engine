// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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

	Vec4 m_viewport;
};
static_assert(sizeof(MaterialGlobalConstants) == 15 * sizeof(Vec4));

#define ANKI_MATERIAL_REGISTER_TILINEAR_REPEAT_SAMPLER 0
#define ANKI_MATERIAL_REGISTER_GLOBAL_CONSTANTS 0
#define ANKI_MATERIAL_REGISTER_GPU_SCENE 0

#define ANKI_MATERIAL_REGISTER_MESHLET_BOUNDING_VOLUMES 1 ///< Points to the unified geom buffer
#define ANKI_MATERIAL_REGISTER_MESHLET_GEOMETRY_DESCRIPTORS 2 ///< Points to the unified geom buffer
#define ANKI_MATERIAL_REGISTER_MESHLET_INSTANCES 3
#define ANKI_MATERIAL_REGISTER_RENDERABLES 4
#define ANKI_MATERIAL_REGISTER_MESH_LODS 5
#define ANKI_MATERIAL_REGISTER_PARTICLE_EMITTERS 6
#define ANKI_MATERIAL_REGISTER_PARTICLE_EMITTERS2 7
#define ANKI_MATERIAL_REGISTER_TRANSFORMS 8
#define ANKI_MATERIAL_REGISTER_NEAREST_CLAMP_SAMPLER 1
#define ANKI_MATERIAL_REGISTER_FIRST_MESHLET 9
#define ANKI_MATERIAL_REGISTER_PER_DRAW 10
#define ANKI_MATERIAL_REGISTER_PER_DRAW_OFFSET 11

// For FW shading:
#define ANKI_MATERIAL_REGISTER_LINEAR_CLAMP_SAMPLER 2
#define ANKI_MATERIAL_REGISTER_SHADOW_SAMPLER 3
#define ANKI_MATERIAL_REGISTER_CLUSTER_SHADING_CONSTANTS 1
#define ANKI_MATERIAL_REGISTER_SCENE_DEPTH 12
#define ANKI_MATERIAL_REGISTER_LIGHT_VOLUME 13
#define ANKI_MATERIAL_REGISTER_CLUSTER_SHADING_POINT_LIGHTS 14
#define ANKI_MATERIAL_REGISTER_CLUSTER_SHADING_SPOT_LIGHTS 15
#define ANKI_MATERIAL_REGISTER_SHADOW_ATLAS 16
#define ANKI_MATERIAL_REGISTER_CLUSTERS 17

#define ANKI_MATERIAL_REGISTER_UNIFIED_GEOMETRY 18
// Always last because it's variable. Texture buffer bindings pointing to unified geom buffer:
// !!WARNING!! Remember to update the UnifiedGeometryTypes.def.h if you change that one
#define ANKI_MATERIAL_REGISTER_UNIFIED_GEOMETRY_TYPED_BUFFER_START 19

ANKI_END_NAMESPACE

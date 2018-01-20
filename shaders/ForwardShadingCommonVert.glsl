// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_FORWARD_SHADING_COMMON_VERT_GLSL
#define ANKI_SHADERS_FORWARD_SHADING_COMMON_VERT_GLSL

// Common code for all vertex shaders of FS
#include "shaders/Common.glsl"

// Global resources
#define LIGHT_SET 0
#define LIGHT_SS_BINDING 0
#define LIGHT_TEX_BINDING 1
#define LIGHT_UBO_BINDING 0
#define LIGHT_MINIMAL
#include "shaders/ClusterLightCommon.glsl"
#undef LIGHT_SET
#undef LIGHT_SS_BINDING
#undef LIGHT_TEX_BINDING

// In/out
layout(location = POSITION_LOCATION) in vec3 in_position;

out gl_PerVertex
{
	vec4 gl_Position;
};

#endif

// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

// Common code for all vertex shaders of FS
#include <shaders/Common.glsl>

// Global resources
#define LIGHT_SET 0
#define LIGHT_SS_BINDING 0
#define LIGHT_TEX_BINDING 1
#define LIGHT_UBO_BINDING 0
#define LIGHT_MINIMAL
#include <shaders/ClusteredShadingCommon.glsl>
#undef LIGHT_SET
#undef LIGHT_SS_BINDING
#undef LIGHT_TEX_BINDING

// In/out
layout(location = POSITION_LOCATION) in Vec3 in_position;

out gl_PerVertex
{
	Vec4 gl_Position;
};

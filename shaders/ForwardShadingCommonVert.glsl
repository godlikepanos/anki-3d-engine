// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

// Common code for all vertex shaders of FS
#include <shaders/Common.glsl>

// Global resources
#define LIGHT_SET 0
#define LIGHT_COMMON_UNIS_BINDING 2
#define LIGHT_LIGHTS_BINDING 3
#define LIGHT_CLUSTERS_BINDING 6
#include <shaders/ClusteredShadingCommon.glsl>

// In/out
layout(location = POSITION_LOCATION) in Vec3 in_position;

out gl_PerVertex
{
	Vec4 gl_Position;
};

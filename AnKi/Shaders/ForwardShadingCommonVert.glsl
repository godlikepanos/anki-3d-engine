// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

// Common code for all vertex shaders of FS
#include <AnKi/Shaders/Common.glsl>

// In/out
layout(location = POSITION_LOCATION) in Vec3 in_position;

out gl_PerVertex
{
	Vec4 gl_Position;
};

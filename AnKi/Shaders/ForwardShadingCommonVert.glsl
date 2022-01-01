// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

// Common code for all vertex shaders of FS
#include <AnKi/Shaders/Common.glsl>
#include <AnKi/Shaders/Include/ModelTypes.h>

// In/out
layout(location = VERTEX_ATTRIBUTE_ID_POSITION) in Vec3 in_position;

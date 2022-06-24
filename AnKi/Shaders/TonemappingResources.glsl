// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Tonemapping resources

#pragma once

#include <AnKi/Shaders/Common.glsl>

#ifndef TONEMAPPING_RESOURCE_AS_WRITE_IMAGE
#	define TONEMAPPING_RESOURCE_AS_WRITE_IMAGE 0
#endif

#if TONEMAPPING_RESOURCE_AS_WRITE_IMAGE
layout(set = 0, binding = TONEMAPPING_BINDING) uniform image2D b_tonemapping;
#else
layout(set = 0, binding = TONEMAPPING_BINDING) uniform readonly image2D b_tonemapping;
#endif

void writeExposureAndLuminance(float exposure, float luminance)
{
#if TONEMAPPING_RESOURCE_AS_WRITE_IMAGE
	imageStore(b_tonemapping, IVec2(0, 0), Vec4(exposure, luminance, 0.0f, 0.0f));
#endif
}

Vec2 getExposureLuminance()
{
	return imageLoad(b_tonemapping, IVec2(0, 0)).xy;
}

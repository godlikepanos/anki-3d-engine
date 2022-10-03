// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Tonemapping resources

#pragma once

#include <AnKi/Shaders/Common.glsl>

#if !defined(TONEMAPPING_RESOURCE_AS_WRITE_IMAGE)
#	define TONEMAPPING_RESOURCE_AS_WRITE_IMAGE 0
#endif

#if TONEMAPPING_RESOURCE_AS_WRITE_IMAGE
layout(set = 0, binding = kTonemappingBinding) ANKI_RP uniform coherent image2D u_tonemappingImage;
#else
layout(set = 0, binding = kTonemappingBinding) ANKI_RP uniform readonly image2D u_tonemappingImage;
#endif

#if TONEMAPPING_RESOURCE_AS_WRITE_IMAGE
void writeExposureAndAverageLuminance(ANKI_RP F32 exposure, ANKI_RP F32 avgLuminance)
{
	imageStore(u_tonemappingImage, IVec2(0), Vec4(exposure, avgLuminance, 0.0f, 0.0f));
}
#endif

ANKI_RP Vec2 readExposureAndAverageLuminance()
{
	return imageLoad(u_tonemappingImage, IVec2(0)).xy;
}

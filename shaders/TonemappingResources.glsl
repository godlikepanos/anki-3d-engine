// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Tonemapping resources

#ifndef ANKI_SHADERS_TONEMAPPING_RESOURCES_GLSL
#define ANKI_SHADERS_TONEMAPPING_RESOURCES_GLSL

#include "shaders/Common.glsl"

#ifndef TONEMAPPING_SET
#define TONEMAPPING_SET 0
#endif

#ifndef TONEMAPPING_LOCATION
#define TONEMAPPING_LOCATION 0
#endif

#ifndef TONEMAPPING_RESOURCE
#define TONEMAPPING_RESOURCE buffer
#endif

layout(std140, ANKI_SS_BINDING(TONEMAPPING_SET, TONEMAPPING_LOCATION)) TONEMAPPING_RESOURCE tmrss0_
{
	vec4 u_averageLuminanceExposurePad2;
};

#define u_averageLuminance u_averageLuminanceExposurePad2.x
#define u_exposureThreshold0 u_averageLuminanceExposurePad2.y

#endif

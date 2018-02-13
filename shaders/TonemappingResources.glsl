// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Tonemapping resources

#ifndef ANKI_SHADERS_TONEMAPPING_RESOURCES_GLSL
#define ANKI_SHADERS_TONEMAPPING_RESOURCES_GLSL

#include "shaders/Common.glsl"

#ifndef TONEMAPPING_SET
#	define TONEMAPPING_SET 0
#endif

#ifndef TONEMAPPING_BINDING
#	define TONEMAPPING_BINDING 0
#endif

#ifndef TONEMAPPING_RESOURCE_AS_BUFFER
#	define TONEMAPPING_RESOURCE_AS_BUFFER 0
#endif

#if TONEMAPPING_RESOURCE_AS_BUFFER
layout(std140, ANKI_SS_BINDING(TONEMAPPING_SET, TONEMAPPING_BINDING)) buffer tmss0_
#else
layout(std140, ANKI_UBO_BINDING(TONEMAPPING_SET, TONEMAPPING_BINDING)) uniform tmu0_
#endif
{
	vec4 u_averageLuminanceExposurePad2;
};

#define u_averageLuminance u_averageLuminanceExposurePad2.x
#define u_exposureThreshold0 u_averageLuminanceExposurePad2.y

#endif

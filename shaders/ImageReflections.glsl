// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains resources and functions for image reflections

#ifndef ANKI_SHADERS_IMAGE_REFLECTIONS_GLSL
#define ANKI_SHADERS_IMAGE_REFLECTIONS_GLSL

#pragma anki include "shaders/Common.glsl"

// Representation of a reflection probe
struct ReflectionProbe
{
	vec3 position;
	float radius;
};

layout(std430, row_major, SS_BINDING(IMAGE_REFLECTIONS_SET,
	IMAGE_REFLECTIONS_SS_BINDING)) readonly buffer _irs0
{
	ReflectionProbe u_reflectionProbes[];
};

layout(TEX_BINDING(IMAGE_REFLECTIONS_SET, IMAGE_REFLECTIONS_TEX_BINDING))
	uniform samplerCubeArray u_reflectionsTex;

//==============================================================================
vec3 readReflection(in vec3 posVSpace, in vec3 normalVSpace)
{
	// Reflection direction
	vec3 eye = normalize(posVSpace);
	vec3 r = reflect(eye, normalVSpace);

	// Compute the collision into the sphere
}

#endif


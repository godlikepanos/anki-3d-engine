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
	// Position of the prove in view space. Radius of probe squared
	vec4 positionRadiusSq;

	// Slice in u_reflectionsTex vector.
	vec4 cubemapSlicePad3;
};

layout(std140, row_major, SS_BINDING(IMAGE_REFLECTIONS_SET,
	IMAGE_REFLECTIONS_SS_BINDING)) readonly buffer _irs0
{
	uvec4 u_reflectionProbeCountPad3;
	ReflectionProbe u_reflectionProbes[];
};

layout(TEX_BINDING(IMAGE_REFLECTIONS_SET, IMAGE_REFLECTIONS_TEX_BINDING))
	uniform samplerCubeArray u_reflectionsTex;

//==============================================================================
// Compute the cubemap texture lookup vector given the reflection vector (r)
// the radius squared of the probe (R2) and the frag pos in sphere space (f)
vec3 computeCubemapVec(in vec3 r, in float R2, in vec3 f)
{
	// Compute the collision of the r to the inner part of the sphere
	// From now on we work on the sphere's space

	// Project the center of the sphere (it's zero now since we are in sphere
	// space) in ray "f,r"
	vec3 p = f - r * dot(f, r);

	// The collision to the sphere is point x where x = p + T * r
	// Because of the pythagorean theorem: R^2 = dot(p, p) + dot(T * r, T * r)
	// solving for T, T = R / |p|
	// then x becomes x = sqrt(R^2 - dot(p, p)) * r + p;
	float pp = dot(p, p);
	pp = min(pp, R2);
	float sq = sqrt(R2 - pp);
	vec3 x = p + sq * r;
}

//==============================================================================
vec3 readReflection(in vec3 posVSpace, in vec3 normalVSpace)
{
	vec3 color = vec3(0.0);

	// Reflection direction
	vec3 eye = normalize(posVSpace);
	vec3 r = reflect(eye, normalVSpace);

	// Iterate probes
	uint count = u_reflectionProbeCountPad3.x;
	for(uint i = 0; i < count; ++i)
	{
		float R2 = u_reflectionProbes[i].positionRadiusSq.w;
		vec3 c = u_reflectionProbes[i].positionRadius.xyz;

		// Check if posVSpace is inside the sphere
		vec3 f = posVSpace - c;
		if(dot(f) < R2)
		{
			vec3 uv = computeCubemapVec(r, R2, f);
			color += texture(u_reflectionsTex, vec4(uv,
				u_reflectionProbes[i].cubemapSlicePad3.x));
		}
	}

	return color;
}

#endif


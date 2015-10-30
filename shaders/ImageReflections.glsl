// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains resources and functions for image reflections

#ifndef ANKI_SHADERS_IMAGE_REFLECTIONS_GLSL
#define ANKI_SHADERS_IMAGE_REFLECTIONS_GLSL

#pragma anki include "shaders/Common.glsl"

#if 0

/// Basically a quad in view space.
struct ReflectionProxy
{
	// xyz: plane.n, w: plane.offset.
	vec4 plane;

	// xyz: -plane.n, w: plane.offset.
	vec4 negPlane;

	// The points of the quad.
	vec4 quadPoints[4];

	// Used to check if the intersection point fall inside the quad. It's:
	// edgeCrossProd[0] = cross(plane.n, (quadPoints[1] - quadPoints[0]));
	// edgeCrossProd[1] = cross(plane.n, (quadPoints[2] - quadPoints[1]));
	// etc...
	vec4 edgeCrossProd[4];
};

// Representation of a reflection probe
struct ReflectionProbe
{
	// Position of the prove in view space. Radius of probe squared
	vec4 positionRadiusSq;

	// Slice in u_reflectionsTex vector.
	vec4 cubemapIndexPad3;
};

layout(std140, row_major, SS_BINDING(IMAGE_REFLECTIONS_SET,
	IMAGE_REFLECTIONS_PROXY_SS_BINDING)) readonly buffer _irs0
{
	uvec4 u_proxyCountReflectionProbeCountPad3;
	ReflectionProxy u_proxies[];
};

layout(std140, row_major, SS_BINDING(IMAGE_REFLECTIONS_SET,
	IMAGE_REFLECTIONS_PROBE_SS_BINDING)) readonly buffer _irs1
{
	ReflectionProbe u_reflectionProbes[];
};

layout(TEX_BINDING(IMAGE_REFLECTIONS_SET, IMAGE_REFLECTIONS_TEX_BINDING))
	uniform samplerCubeArray u_reflectionsTex;

//==============================================================================
// Test if a ray intersects with the proxy.
// p is the origin of the ray, r the ray vector, c is the intersection point.
bool testReflectionProxy(in uint proxyIdx, in vec3 p, in vec3 r, out vec3 c)
{
	ReflectionProxy proxy = u_proxies[proxyIdx];
	bool intersect;

	// Compute the inverce direction of p to the plane
	float d = dot(proxy.negPlane, vec4(p, 1.0));

	// Compute another dot
	float a = dot(proxy.plane.xyz, r);

	if(d < 0.0 && a < 0.0)
	{
		float s = d / a;
		c = p + s * r;

		// Now check each edge
		vec4 tests;
		for(uint i = 0; i < 4; ++i)
		{
			tests[i] = dot(c - proxy.quadPoints[i], proxy.edgeCrossProd[i]);
		}

		intersect = all(greaterThan(tests, vec4(0.0)));
	}
	else
	{
		intersect = false;
	}

	return intersect;
}

#endif

// Representation of a reflection probe
struct ReflectionProbe
{
	// Position of the prove in view space. Radius of probe squared
	vec4 positionRadiusSq;

	// Slice in u_reflectionsTex vector.
	vec4 cubemapIndexPad3;
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

	return normalize(x);
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
		vec3 c = u_reflectionProbes[i].positionRadiusSq.xyz;

		// Check if posVSpace is inside the sphere
		vec3 f = posVSpace - c;
		if(dot(f, f) < R2)
		{
			vec3 uv = computeCubemapVec(r, R2, f);
			color += texture(u_reflectionsTex, vec4(uv,
				u_reflectionProbes[i].cubemapIndexPad3.x));
		}
	}

	return color;
}

#endif


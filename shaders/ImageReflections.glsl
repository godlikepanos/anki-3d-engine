// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains resources and functions for image reflections

#ifndef ANKI_SHADERS_IMAGE_REFLECTIONS_GLSL
#define ANKI_SHADERS_IMAGE_REFLECTIONS_GLSL

#pragma anki include "shaders/Common.glsl"

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
	ReflectionProxy u_reflectionProxies[];
};

layout(std140, row_major, SS_BINDING(IMAGE_REFLECTIONS_SET,
	IMAGE_REFLECTIONS_PROBE_SS_BINDING)) readonly buffer _irs1
{
	mat3 u_invViewRotation;
	ReflectionProbe u_reflectionProbes[];
};

layout(TEX_BINDING(IMAGE_REFLECTIONS_SET, IMAGE_REFLECTIONS_TEX_BINDING))
	uniform samplerCubeArray u_reflectionsTex;

//==============================================================================
// Test if a ray intersects with the proxy.
// p is the origin of the ray, r the ray vector, c is the intersection point.
bool testReflectionProxy(in uint proxyIdx, in vec3 p, in vec3 r, out vec3 c)
{
	ReflectionProxy proxy = u_reflectionProxies[proxyIdx];
	bool intersect;

	// Compute the inverce direction of p to the plane
	float d = dot(proxy.negPlane, vec4(p, 1.0));

	// Compute another dot
	float a = dot(proxy.plane.xyz, r);

	if(all(lessThan(vec2(d, a), vec2(0.0))))
	{
		float s = d / a;
		c = p + s * r;

		// Now check each edge
		vec4 tests;
		for(uint i = 0; i < 4; ++i)
		{
			tests[i] = dot(c - proxy.quadPoints[i], proxy.edgeCrossProd[i].xyz);
		}

		intersect = all(greaterThan(tests, vec4(0.0)));
	}
	else
	{
		intersect = false;
	}

	return intersect;
}

//==============================================================================
// Find a point that the ray (p,r) hit a proxy. Return the intersection point.
bool findCloseProxyIntersection(in vec3 p, in vec3 r, out vec3 c)
{
	const float BIG_FLOAT = 1000000.0;
	float distSq = BIG_FLOAT;
	uint proxyCount = u_proxyCountReflectionProbeCountPad3.x;
	for(uint i = 0; i < proxyCount; ++i)
	{
		vec3 intersection;
		bool intersects = testReflectionProxy(j, posVSpace, r, intersection);
		if(intersects)
		{
			// Compute the distance^2
			vec3 a = intersection - p;
			float b = dot(a, a);

			if(b < distSq)
			{
				// Intersection point is closer than the prev one
				distSq = b;
				c = intersection;
			}
		}
	}

	return lessThan(distSq, BIG_FLOAT);
}

//==============================================================================
bool findProbe(in vec3 c, out float cubemapIndex, out vec3 probeOrigin)
{
	// Iterate probes to find the cubemap
	uint count = u_proxyCountReflectionProbeCountPad3.y;
	for(uint i = 0; i < count; ++i)
	{
		float R2 = u_reflectionProbes[i].positionRadiusSq.w;
		vec3 center = u_reflectionProbes[i].positionRadiusSq.xyz;

		// Check if posVSpace is inside the sphere
		vec3 f = c - center;
		if(dot(f, f) < R2)
		{
			// Found something
			cubemapIndex = u_reflectionProbes[i].cubemapIndexPad3.x;
			probeOrigin = u_reflectionProbes[i].positionRadiusSq.xyz;
			return true;
		}
	}

	return false;
}

//==============================================================================
vec3 readReflection(in vec3 posVSpace, in vec3 normalVSpace)
{
	vec3 color = vec3(0.0);

	// Reflection direction
	vec3 eye = normalize(posVSpace);
	vec3 r = reflect(eye, normalVSpace);

	// Check proxy
	vec3 intersection;
	if(findCloseProxyIntersection(posVSpace, r, intersection))
	{
		// Check for probes
		float cubemapIdx;
		vec3 probeOrigin;
		if(findProbe(intersection, cubemapIdx, probeOrigin))
		{
			// Cubemap UV in view space
			vec3 uv = normalize(intersection - probeOrigin);

			// Rotate UV to move it to world space
			uv = u_invViewRotation * uv;

			// Read!
			color = texture(u_reflectionsTex, vec4(uv, cubemapIdx));
		}
	}

	return color;
}

#endif


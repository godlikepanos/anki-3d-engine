// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains resources and functions for image reflections

#ifndef ANKI_SHADERS_IMAGE_REFLECTIONS_GLSL
#define ANKI_SHADERS_IMAGE_REFLECTIONS_GLSL

#include "shaders/Clusterer.glsl"

#define ACCURATE_RAYS 0

// Representation of a reflection probe
struct ReflectionProbe
{
	// Position of the prove in view space. Radius of probe squared
	vec4 positionRadiusSq;

	// Slice in u_reflectionsTex vector.
	vec4 cubemapIndexPad3;
};

layout(std140,
	row_major,
	SS_BINDING(IMAGE_REFLECTIONS_SET,
		   IMAGE_REFLECTIONS_FIRST_SS_BINDING)) readonly buffer _irs1
{
	mat3 u_invViewRotation;
	vec4 u_nearClusterMagicPad2;
	ReflectionProbe u_reflectionProbes[];
};

layout(std430,
	row_major,
	SS_BINDING(IMAGE_REFLECTIONS_SET,
		   IMAGE_REFLECTIONS_FIRST_SS_BINDING + 1)) readonly buffer _irs2
{
	uint u_reflectionProbeIndices[];
};

layout(std430,
	row_major,
	SS_BINDING(IMAGE_REFLECTIONS_SET,
		   IMAGE_REFLECTIONS_FIRST_SS_BINDING + 2)) readonly buffer _irs3
{
	uvec2 u_reflectionClusters[];
};

layout(TEX_BINDING(IMAGE_REFLECTIONS_SET,
	IMAGE_REFLECTIONS_TEX_BINDING)) uniform samplerCubeArray u_reflectionsTex;

layout(TEX_BINDING(IMAGE_REFLECTIONS_SET,
	IMAGE_REFLECTIONS_TEX_BINDING
		+ 1)) uniform samplerCubeArray u_irradianceTex;

layout(TEX_BINDING(IMAGE_REFLECTIONS_SET,
	IMAGE_REFLECTIONS_TEX_BINDING + 2)) uniform sampler2D u_integrationLut;

//==============================================================================
// Compute the cubemap texture lookup vector given the reflection vector (r)
// the radius squared of the probe (R2) and the frag pos in sphere space (f)
vec3 computeCubemapVec(in vec3 r, in float R2, in vec3 f)
{
#if ACCURATE_RAYS
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

	// Rotate UV to move it to world space
	vec3 uv = u_invViewRotation * x;

	return uv;
#else
	return u_invViewRotation * r;
#endif
}

//==============================================================================
void readIndirectInternal(in uint clusterIndex,
	in vec3 posVSpace,
	in vec3 r,
	in vec3 n,
	in float lod,
	out vec3 specIndirect,
	out vec3 diffIndirect)
{
	specIndirect = vec3(0.0);
	diffIndirect = vec3(0.0);

	// Check proxy
	uvec2 cluster = u_reflectionClusters[clusterIndex];
	uint indexOffset = cluster[0];
	uint indexCount = cluster[1];
	for(uint i = 0; i < indexCount; ++i)
	{
		uint probeIndex = u_reflectionProbeIndices[indexOffset++];
		ReflectionProbe probe = u_reflectionProbes[probeIndex];

		float R2 = probe.positionRadiusSq.w;
		vec3 center = probe.positionRadiusSq.xyz;

		// Get distance from the center of the probe
		vec3 f = posVSpace - center;

		// Cubemap UV in view space
		vec3 uv = computeCubemapVec(r, R2, f);

		// Read!
		float cubemapIndex = probe.cubemapIndexPad3.x;
		vec3 c = textureLod(u_reflectionsTex, vec4(uv, cubemapIndex), lod).rgb;

		// Combine (lerp) with previous color
		float d = dot(f, f);
		float factor = d / R2;
		factor = min(factor, 1.0);
		specIndirect = mix(c, specIndirect, factor);
		// Same as: specIndirect = c * (1.0 - factor) + specIndirect * factor

		// Do the same for diffuse
		uv = computeCubemapVec(n, R2, f);
		vec3 id = texture(u_irradianceTex, vec4(uv, cubemapIndex)).rgb;
		diffIndirect = mix(id, diffIndirect, factor);
	}
}

//==============================================================================
void readIndirect(in vec3 posVSpace,
	in vec3 r,
	in vec3 n,
	in float lod,
	out vec3 specIndirect,
	out vec3 diffIndirect)
{
	uint clusterIdx =
		computeClusterIndexUsingFragCoord(u_nearClusterMagicPad2.x,
			u_nearClusterMagicPad2.y,
			posVSpace.z,
			TILE_COUNT_X,
			TILE_COUNT_Y);

	readIndirectInternal(
		clusterIdx, posVSpace, r, n, lod, specIndirect, diffIndirect);
}

#endif

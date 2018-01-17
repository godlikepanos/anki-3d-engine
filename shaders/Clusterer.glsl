// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for the clusterer

#ifndef ANKI_SHADERS_CLUSTERER_GLSL
#define ANKI_SHADERS_CLUSTERER_GLSL

#include "shaders/Common.glsl"

uint computeClusterK(vec4 clustererMagic, vec3 worldPos)
{
	float fz = sqrt(dot(clustererMagic.xyz, worldPos) - clustererMagic.w);
	uint z = uint(fz);
	return z;
}

// Compute cluster index
uint computeClusterIndex(vec2 uv, vec4 clustererMagic, vec3 worldPos, uint clusterCountX, uint clusterCountY)
{
	uvec2 xy = uvec2(uv * vec2(clusterCountX, clusterCountY));

	return computeClusterK(clustererMagic, worldPos) * (clusterCountX * clusterCountY) + xy.y * clusterCountX + xy.x;
}

// Compute the Z of the near plane given a cluster idx
float computeClusterNear(uint k, float near, vec4 clustererMagic)
{
	// TODO
	return near;
}

float computeClusterFar(uint k, float near, vec4 clustererMagic)
{
	// TODO
	return near;
}

#endif

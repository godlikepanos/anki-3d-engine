// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for the clusterer

#ifndef ANKI_SHADERS_CLUSTERER_GLSL
#define ANKI_SHADERS_CLUSTERER_GLSL

#include "shaders/Common.glsl"

uint computeClusterK(float near, float clustererMagic, float zVSpace)
{
	float fz = sqrt((zVSpace + near) * clustererMagic);
	uint z = uint(fz);
	return z;
}

uint computeClusterKSafe(float near, float far, float clustererMagic, float zVSpace)
{
	float z = clamp(zVSpace, -far, -near);
	float kf = sqrt((z + near) * clustererMagic);
	return uint(kf);
}

// Compute cluster index
uint computeClusterIndex(
	vec2 uv, float near, float clustererMagic, float zVSpace, uint clusterCountX, uint clusterCountY)
{
	uvec2 xy = uvec2(uv * vec2(clusterCountX, clusterCountY));

	return computeClusterK(near, clustererMagic, zVSpace) * (clusterCountX * clusterCountY) + xy.y * clusterCountX
		+ xy.x;
}

// Compute the Z of the near plane given a cluster idx
float computeClusterNear(uint k, float near, float clustererMagic)
{
	return 1.0 / clustererMagic * pow(float(k), 2.0) - near;
}

float computeClusterFar(uint k, float near, float clustererMagic)
{
	return 1.0 / clustererMagic * pow(float(k + 1u), 2.0) - near;
}

#endif

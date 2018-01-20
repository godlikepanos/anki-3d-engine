// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for the clusterer

#ifndef ANKI_SHADERS_CLUSTERER_GLSL
#define ANKI_SHADERS_CLUSTERER_GLSL

#include "shaders/Common.glsl"

// See the documentation of the Clusterer class.
struct ClustererMagicValues
{
	vec4 val0;
	vec4 val1;
};

uint computeClusterK(ClustererMagicValues magic, vec3 worldPos)
{
	float fz = sqrt(dot(magic.val0.xyz, worldPos) - magic.val0.w);
	uint z = uint(fz);
	return z;
}

// Compute cluster index
uint computeClusterIndex(ClustererMagicValues magic, vec2 uv, vec3 worldPos, uint clusterCountX, uint clusterCountY)
{
	uvec2 xy = uvec2(uv * vec2(clusterCountX, clusterCountY));

	return computeClusterK(magic, worldPos) * (clusterCountX * clusterCountY) + xy.y * clusterCountX + xy.x;
}

// Compute the Z of the near plane given a cluster idx
float computeClusterNear(ClustererMagicValues magic, uint k)
{
	float fk = float(k);
	return magic.val1.x * fk * fk + magic.val1.y;
}

float computeClusterFar(ClustererMagicValues magic, uint k)
{
	return computeClusterNear(magic, k + 1u);
}

#endif

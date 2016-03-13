// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for the clusterer

#ifndef ANKI_SHADERS_CLUSTERER_GLSL
#define ANKI_SHADERS_CLUSTERER_GLSL

#include "shaders/Common.glsl"

//==============================================================================
// Compute the cluster index using the tile index.
uint computeClusterIndexUsingTileIdx(float near,
	float clustererMagic,
	float zVSpace,
	uint tileIdx,
	uint tileCountX,
	uint tileCountY)
{
	float fk = sqrt((zVSpace + near) * clustererMagic);
	uint k = uint(fk);

	return tileIdx + k * (tileCountX * tileCountY);
}

//==============================================================================
// Compute the cluster index using the a custom gl_FragCoord.xy.
uint computeClusterIndexUsingCustomFragCoord(float near,
	float clustererMagic,
	float zVSpace,
	uint tileCountX,
	uint tileCountY,
	vec2 fragCoord)
{
	// Compute tile idx
	uvec2 f = uvec2(fragCoord) >> 6;
	uint tileIdx = f.y * tileCountX + f.x;

	return computeClusterIndexUsingTileIdx(
		near, clustererMagic, zVSpace, tileIdx, tileCountX, tileCountY);
}

//==============================================================================
// Compute the cluster index using the gl_FragCoord.xy.
uint computeClusterIndexUsingFragCoord(float near,
	float clustererMagic,
	float zVSpace,
	uint tileCountX,
	uint tileCountY)
{
	return computeClusterIndexUsingCustomFragCoord(
		near, clustererMagic, zVSpace, tileCountX, tileCountY, gl_FragCoord.xy);
}

#endif

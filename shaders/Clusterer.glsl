// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for the clusterer

#ifndef ANKI_SHADERS_CLUSTERER_GLSL
#define ANKI_SHADERS_CLUSTERER_GLSL

#pragma anki include "shaders/Common.glsl"

//==============================================================================
// Compute the cluster index using the tile index.
uint computeClusterIndexUsingTileIdx(float near, float clustererMagic,
	float zVSpace, uint tileIdx)
{
	float fk = sqrt((near + zVSpace) * clustererMagic);
	uint k = uint(fk);

	return tileIdx + k;
}

//==============================================================================
// Compute the cluster index using the gl_FragCoord.xy.
uint computeClusterIndexUsingFragCoord(float near, float clustererMagic,
	float zVSpace, uint tileCountX)
{
	// Compute tile idx
	uvec2 f = uvec2(gl_FragCoord.xy) >> 6;
	uint tileIdx = f.y * tileCountX + f.x;

	return computeClusterIndexUsingTileIdx(near, clustererMagic, zVSpace,
		tileIdx);
}

#endif

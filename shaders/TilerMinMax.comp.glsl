// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type comp
#pragma anki include "shaders/Common.glsl"

const uint U32_MAX = 0xFFFFFFFFU;

const uint WORKGROUP_SIZE_X = 16;
const uint WORKGROUP_SIZE_Y = 16;

const uint PIXEL_READ_X = TILE_SIZE_X / WORKGROUP_SIZE_X;
const uint PIXEL_READ_Y = TILE_SIZE_Y / WORKGROUP_SIZE_Y;

layout(binding = 0) uniform sampler2D u_depthMap;

layout(std430, binding = 0) writeonly buffer _blk
{
	vec2 u_depthLimits[];
};

layout(
	local_size_x = WORKGROUP_SIZE_X, 
	local_size_y = WORKGROUP_SIZE_Y, 
	local_size_z = 1) in;

shared uvec2 g_minMaxDepth[PIXEL_READ_Y][PIXEL_READ_X];

//==============================================================================
void main()
{
	// Init
	for(uint y = 0; y < PIXEL_READ_Y; ++y)
	{
		for(uint x = 0; x < PIXEL_READ_X; ++x)
		{
			g_minMaxDepth[y][x] = uvec2(U32_MAX, 0U);
		}
	}

	memoryBarrierShared();
	barrier();

	// Get max/min depth
	ivec2 coord = 
		ivec2(gl_GlobalInvocationID.xy) * ivec2(PIXEL_READ_X, PIXEL_READ_Y);

	for(uint y = 0; y < PIXEL_READ_Y; ++y)
	{
		for(uint x = 0; x < PIXEL_READ_X; ++x)
		{
			float depth = texelFetchOffset(u_depthMap, coord, 0, ivec2(x, y)).r;
			uint udepth = uint(depth * float(U32_MAX));
			atomicMin(g_minMaxDepth[y][x].x, udepth);
			atomicMax(g_minMaxDepth[y][x].y, udepth);
		}
	}

	memoryBarrierShared();
	barrier();

	// Write result
	if(gl_LocalInvocationIndex == 0)
	{
		float mind = 1.0;
		float maxd = 0.0;
		for(uint y = 0; y < PIXEL_READ_Y; ++y)
		{
			for(uint x = 0; x < PIXEL_READ_X; ++x)
			{
				vec2 depthLimits = vec2(g_minMaxDepth[y][x]) / float(U32_MAX);
				mind = min(mind, depthLimits.x);
				maxd = max(maxd, depthLimits.y);
			}
		}

		uint idx = gl_WorkGroupID.y * TILES_COUNT_X + gl_WorkGroupID.x;
		u_depthLimits[idx] = vec2(mind, maxd);
	}
}



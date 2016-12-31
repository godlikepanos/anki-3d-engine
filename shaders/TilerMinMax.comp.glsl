// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

const uint U32_MAX = 0xFFFFFFFFU;

const uint WORKGROUP_SIZE_X = 16;
const uint WORKGROUP_SIZE_Y = 16;

// Every thread will read more pixels since the workgroup size is less than
// the tile size.
const uint PIXEL_READ_X = TILE_SIZE_X / WORKGROUP_SIZE_X;
const uint PIXEL_READ_Y = TILE_SIZE_Y / WORKGROUP_SIZE_Y;

layout(binding = 0) uniform sampler2D u_depthMap;

layout(std430, binding = 0) writeonly buffer _blk
{
	vec2 u_depthLimits[];
};

layout(local_size_x = WORKGROUP_SIZE_X, local_size_y = WORKGROUP_SIZE_Y, local_size_z = 1) in;

shared uint g_minDepth;
shared uint g_maxDepth;

void main()
{
	// Init
	g_minDepth = U32_MAX;
	g_maxDepth = 0U;

	memoryBarrierShared();
	barrier();

	// Get max/min depth
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy) * ivec2(PIXEL_READ_X, PIXEL_READ_Y);

	float mind = 10.0;
	float maxd = -10.0;
	for(uint y = 0; y < PIXEL_READ_Y; ++y)
	{
		for(uint x = 0; x < PIXEL_READ_X; ++x)
		{
			float depth = texelFetch(u_depthMap, coord + ivec2(x, y), 0).r;
			mind = min(mind, depth);
			maxd = max(maxd, depth);
		}
	}

	uvec2 udepth = uvec2(vec2(mind, maxd) * float(U32_MAX));
	atomicMin(g_minDepth, udepth.x);
	atomicMax(g_maxDepth, udepth.y);

	memoryBarrierShared();
	barrier();

	// Write result
	if(gl_LocalInvocationIndex == 0)
	{
		uint idx = gl_WorkGroupID.y * TILES_COUNT_X + gl_WorkGroupID.x;
		u_depthLimits[idx] = vec2(g_minDepth, g_maxDepth) / float(U32_MAX);
	}
}

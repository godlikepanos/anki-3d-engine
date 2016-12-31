// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct DrawArraysIndirectInfo
{
	uint count;
	uint instanceCount;
	uint first;
	uint baseInstance;
};

layout(ANKI_SS_BINDING(0, 0), std430) buffer ss0_
{
	uint u_queryResults[];
};

layout(ANKI_SS_BINDING(0, 1), std430) buffer ss1_
{
	DrawArraysIndirectInfo u_indirectInfo[];
};

void main()
{
	if(u_queryResults[gl_WorkGroupID.x] == 0)
	{
		u_indirectInfo[gl_WorkGroupID.x].count = 0;
	}
	else
	{
		u_indirectInfo[gl_WorkGroupID.x].count = 4;
	}

	u_indirectInfo[gl_WorkGroupID.x].instanceCount = 1;
	u_indirectInfo[gl_WorkGroupID.x].first = 0;
	u_indirectInfo[gl_WorkGroupID.x].baseInstance = 0;
}

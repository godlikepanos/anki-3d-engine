// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.glsl>

layout(set = 0, binding = 0) uniform sampler u_linearAnyClampSampler;
layout(set = 0, binding = 1) uniform ANKI_RP texture2D u_tex;

#if defined(ANKI_COMPUTE_SHADER)
const UVec2 WORKGROUP_SIZE = UVec2(16, 16);
layout(local_size_x = WORKGROUP_SIZE.x, local_size_y = WORKGROUP_SIZE.y, local_size_z = 1) in;

// Push constants hold the size of the output image
layout(push_constant, row_major, std430) uniform pc_
{
	UVec2 u_fbSize;
	UVec2 u_padding;
};

Vec2 in_uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5) / Vec2(u_fbSize);
layout(set = 0, binding = 2) writeonly uniform ANKI_RP image2D out_img;
ANKI_RP Vec3 out_color;
#else
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out ANKI_RP Vec3 out_color;
#endif

void main()
{
#if defined(ANKI_COMPUTE_SHADER)
	if(gl_GlobalInvocationID.x >= u_fbSize.x || gl_GlobalInvocationID.y >= u_fbSize.y)
	{
		// Skip pixels outside the viewport
		return;
	}
#endif

	const ANKI_RP F32 weight = 1.0 / 5.0;
	out_color = textureLod(u_tex, u_linearAnyClampSampler, in_uv, 0.0).rgb * weight;
	out_color += textureLodOffset(sampler2D(u_tex, u_linearAnyClampSampler), in_uv, 0.0, IVec2(+1, +1)).rgb * weight;
	out_color += textureLodOffset(sampler2D(u_tex, u_linearAnyClampSampler), in_uv, 0.0, IVec2(-1, -1)).rgb * weight;
	out_color += textureLodOffset(sampler2D(u_tex, u_linearAnyClampSampler), in_uv, 0.0, IVec2(+1, -1)).rgb * weight;
	out_color += textureLodOffset(sampler2D(u_tex, u_linearAnyClampSampler), in_uv, 0.0, IVec2(-1, +1)).rgb * weight;

#if defined(ANKI_COMPUTE_SHADER)
	imageStore(out_img, IVec2(gl_GlobalInvocationID.xy), Vec4(out_color, 0.0));
#endif
}

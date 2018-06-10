// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <shaders/Common.glsl>

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_tex;

#if defined(ANKI_COMPUTE_SHADER)
layout(local_size_x = WORKGROUP_SIZE.x, local_size_y = WORKGROUP_SIZE.y, local_size_z = 1) in;

// Push constants hold the size of the output image
struct PushConsts
{
	UVec4 m_outImageSizePad2;
};
ANKI_PUSH_CONSTANTS(PushConsts, u_regs);
#	define u_fbSize (u_regs.m_outImageSizePad2.xy)

Vec2 in_uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5) / Vec2(u_fbSize);
layout(ANKI_IMAGE_BINDING(0, 0)) writeonly uniform image2D out_img;
Vec3 out_color;
#else
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out Vec3 out_color;
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

	out_color = textureLod(u_tex, in_uv, 0.0).rgb;
	out_color += textureLodOffset(u_tex, in_uv, 0.0, IVec2(+1, +1)).rgb;
	out_color += textureLodOffset(u_tex, in_uv, 0.0, IVec2(-1, -1)).rgb;
	out_color += textureLodOffset(u_tex, in_uv, 0.0, IVec2(+1, -1)).rgb;
	out_color += textureLodOffset(u_tex, in_uv, 0.0, IVec2(-1, +1)).rgb;

	out_color *= (1.0 / 5.0);

#if defined(ANKI_COMPUTE_SHADER)
	imageStore(out_img, IVec2(gl_GlobalInvocationID.xy), Vec4(out_color, 0.0));
#endif
}

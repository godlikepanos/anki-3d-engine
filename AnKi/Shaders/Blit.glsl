// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Shaders/Functions.glsl>

layout(set = 0, binding = 0) uniform sampler u_linearAnyClampSampler;
layout(set = 0, binding = 1) uniform texture2D u_tex;

#if defined(ANKI_COMPUTE_SHADER)
#	define USE_COMPUTE 1
#else
#	define USE_COMPUTE 0
#endif

#if USE_COMPUTE
layout(set = 0, binding = 2) uniform writeonly image2D u_outImage;

layout(push_constant, std140) uniform b_pc
{
	Vec2 u_viewportSize;
	UVec2 u_viewportSizeU;
};

layout(local_size_x = 8, local_size_y = 8) in;
#else
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out Vec3 out_col;
#endif

void main()
{
#if USE_COMPUTE
	if(skipOutOfBoundsInvocations(UVec2(8u), u_viewportSizeU))
	{
		return;
	}

	const Vec2 uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5) / u_viewportSize;
#else
	const Vec2 uv = in_uv;
#endif

	const Vec3 color = textureLod(u_tex, u_linearAnyClampSampler, uv, 0.0).rgb;

#if USE_COMPUTE
	imageStore(u_outImage, IVec2(gl_GlobalInvocationID.xy), Vec4(color, 0.0));
#else
	out_col = color;
#endif
}

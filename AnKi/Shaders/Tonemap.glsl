// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Does tonemapping

#include <AnKi/Shaders/Functions.glsl>
#include <AnKi/Shaders/TonemappingFunctions.glsl>

layout(set = 0, binding = 0) uniform sampler u_nearestAnyClampSampler;
layout(set = 0, binding = 1) uniform ANKI_RP texture2D u_inputRt;

const U32 kTonemappingBinding = 2u;
#include <AnKi/Shaders/TonemappingResources.glsl>

#if defined(ANKI_COMPUTE_SHADER)
layout(set = 0, binding = 3) writeonly uniform image2D u_outImg;

const UVec2 kWorkgroupSize = UVec2(8, 8);
layout(local_size_x = kWorkgroupSize.x, local_size_y = kWorkgroupSize.y, local_size_z = 1) in;
#else
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out Vec3 out_color;
#endif

layout(push_constant, std140) uniform b_pc
{
	Vec2 u_viewportSizeOverOne;
	UVec2 u_viewportSize;
};

void main()
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(kWorkgroupSize, u_viewportSize))
	{
		return;
	}

	const Vec2 uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5f) * u_viewportSizeOverOne;
#else
	const Vec2 uv = in_uv;
#endif

	const ANKI_RP Vec3 hdr = textureLod(u_inputRt, u_nearestAnyClampSampler, uv, 0.0f).rgb;
	const Vec3 tonemapped = linearToSRgb(tonemap(hdr, readExposureAndAverageLuminance().x));

#if defined(ANKI_COMPUTE_SHADER)
	imageStore(u_outImg, IVec2(gl_GlobalInvocationID.xy), Vec4(tonemapped, 0.0));
#else
	out_color = tonemapped;
#endif
}

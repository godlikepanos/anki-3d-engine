// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_SPECIALIZATION_CONSTANT_UVEC2(kViewport, 0u);

#include <AnKi/Shaders/TonemappingFunctions.glsl>
#include <AnKi/Shaders/Functions.glsl>

layout(set = 0, binding = 0) uniform sampler u_linearAnyClampSampler;
layout(set = 0, binding = 1) uniform ANKI_RP texture2D u_tex; ///< Its the IS RT

layout(push_constant) uniform b_pc
{
	Vec4 u_thresholdScalePad2;
};

const U32 kTonemappingBinding = 2u;
#include <AnKi/Shaders/TonemappingResources.glsl>

#if defined(ANKI_COMPUTE_SHADER)
const UVec2 kWorkgroupSize = UVec2(16, 16);
layout(local_size_x = kWorkgroupSize.x, local_size_y = kWorkgroupSize.y, local_size_z = 1) in;

layout(set = 0, binding = 3) writeonly uniform image2D u_outImg;
#else
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out ANKI_RP Vec3 out_color;
#endif

void main()
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(kWorkgroupSize, kViewport))
	{
		return;
	}

	const Vec2 uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5) / Vec2(kViewport);
#else
	const Vec2 uv = in_uv;
#endif

	ANKI_RP F32 weight = 1.0 / 5.0;
	ANKI_RP Vec3 color = textureLod(u_tex, u_linearAnyClampSampler, uv, 0.0).rgb * weight;
	color += textureLodOffset(sampler2D(u_tex, u_linearAnyClampSampler), uv, 0.0, IVec2(+1, +1)).rgb * weight;
	color += textureLodOffset(sampler2D(u_tex, u_linearAnyClampSampler), uv, 0.0, IVec2(-1, -1)).rgb * weight;
	color += textureLodOffset(sampler2D(u_tex, u_linearAnyClampSampler), uv, 0.0, IVec2(-1, +1)).rgb * weight;
	color += textureLodOffset(sampler2D(u_tex, u_linearAnyClampSampler), uv, 0.0, IVec2(+1, -1)).rgb * weight;

	color = tonemap(color, readExposureAndAverageLuminance().y, u_thresholdScalePad2.x) * u_thresholdScalePad2.y;

#if defined(ANKI_COMPUTE_SHADER)
	imageStore(u_outImg, IVec2(gl_GlobalInvocationID.xy), Vec4(color, 0.0));
#else
	out_color = color;
#endif
}

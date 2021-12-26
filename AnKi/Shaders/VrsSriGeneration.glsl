// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki mutator SRI_TEXEL_DIMENSION 8 16 32

#include <AnKi/Shaders/Functions.glsl>
#include <AnKi/Shaders/TonemappingFunctions.glsl>

layout(set = 0, binding = 0) uniform ANKI_RP texture2D u_inputTex;

#if defined(ANKI_COMPUTE_SHADER)
const UVec2 WORKGROUP_SIZE = UVec2(SRI_TEXEL_DIMENSION, SRI_TEXEL_DIMENSION);
layout(local_size_x = WORKGROUP_SIZE.x, local_size_y = WORKGROUP_SIZE.y, local_size_z = 1) in;

layout(set = 0, binding = 1) uniform writeonly uimage2D u_sriImg;
#else
layout(location = 0) out U32 out_shadingRate;
#endif

shared Vec2 s_lumas[WORKGROUP_SIZE.y * WORKGROUP_SIZE.x];

void main()
{
	// Get luminance
	const Vec3 color = invertibleTonemap(texelFetch(u_inputTex, IVec2(gl_GlobalInvocationID.xy), 0).xyz);
	const F32 luma = computeLuminance(color);
	const F32 lumaSquared = luma * luma;

	// Store luminance
	s_lumas[gl_LocalInvocationIndex] = Vec2(luma, lumaSquared);
	memoryBarrierShared();
	barrier();

	// Gather the results into one
	ANKI_LOOP for(U32 s = (WORKGROUP_SIZE.x * WORKGROUP_SIZE.y) / 2u; s > 0u; s >>= 1u)
	{
		if(gl_LocalInvocationIndex < s)
		{
			s_lumas[gl_LocalInvocationIndex] += s_lumas[gl_LocalInvocationIndex + s];
		}

		memoryBarrierShared();
		barrier();
	}

	// Write the result
	ANKI_BRANCH if(gl_LocalInvocationIndex == 0u)
	{
		const F32 variance = abs(s_lumas[0].y - s_lumas[0].x * s_lumas[0].x);
		const F32 maxVariance = 100.0;

		const F32 factor = 1.0 - min(1.0, variance / maxVariance);
		const U32 rate = 1u << U32(factor * 2.0);

		const UVec2 inputTexelCoord = gl_WorkGroupID.xy;
		imageStore(u_sriImg, IVec2(inputTexelCoord), UVec4(encodeVrsRate(UVec2(rate))));
	}
}

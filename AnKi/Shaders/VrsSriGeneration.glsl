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

shared F32 s_lumaMin[WORKGROUP_SIZE.y * WORKGROUP_SIZE.x];
shared F32 s_lumaMax[WORKGROUP_SIZE.y * WORKGROUP_SIZE.x];

void main()
{
	// Get luminance
	const Vec3 color = invertibleTonemap(texelFetch(u_inputTex, IVec2(gl_GlobalInvocationID.xy), 0).xyz);
	const F32 luma = computeLuminance(color);

	// Store luminance
	s_lumaMin[gl_LocalInvocationIndex] = luma;
	s_lumaMax[gl_LocalInvocationIndex] = luma;
	memoryBarrierShared();
	barrier();

	// Gather the results into one
	ANKI_LOOP for(U32 s = (WORKGROUP_SIZE.x * WORKGROUP_SIZE.y) / 2u; s > 0u; s >>= 1u)
	{
		if(gl_LocalInvocationIndex < s)
		{
			s_lumaMin[gl_LocalInvocationIndex] =
				min(s_lumaMin[gl_LocalInvocationIndex], s_lumaMin[gl_LocalInvocationIndex + s]);
			s_lumaMax[gl_LocalInvocationIndex] =
				max(s_lumaMax[gl_LocalInvocationIndex], s_lumaMax[gl_LocalInvocationIndex + s]);
		}

		memoryBarrierShared();
		barrier();
	}

	// Write the result
	ANKI_BRANCH if(gl_LocalInvocationIndex == 0u)
	{
		const F32 diff = s_lumaMax[0] - s_lumaMin[0];
		const F32 maxLumaDiff = 1.0 / 32.0;

		const F32 factor = min(1.0, diff / maxLumaDiff);
		const U32 rate = 1u << (2u - U32(factor * 2.0));

		const UVec2 inputTexelCoord = gl_WorkGroupID.xy;
		imageStore(u_sriImg, IVec2(inputTexelCoord), UVec4(encodeVrsRate(UVec2(rate))));
	}
}

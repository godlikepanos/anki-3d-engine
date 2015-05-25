// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type comp
#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/Tonemapping.glsl"

#if IS_RT_MIPMAP == 0
#	error Wrong mipmap
#endif

const uint WORKGROUP_SIZE_X = 16u;
const uint WORKGROUP_SIZE_Y = 16u;
const uint WORKGROUP_SIZE = WORKGROUP_SIZE_X * WORKGROUP_SIZE_Y;

layout(
	local_size_x = WORKGROUP_SIZE_X,
	local_size_y = WORKGROUP_SIZE_Y,
	local_size_z = 1) in;

const uint MIPMAP_WIDTH = ANKI_RENDERER_WIDTH / (2u << (IS_RT_MIPMAP - 1u));
const uint MIPMAP_HEIGHT = ANKI_RENDERER_HEIGHT / (2u << (IS_RT_MIPMAP - 1u));

const uint PIXEL_READ_X = MIPMAP_WIDTH / WORKGROUP_SIZE_X;
const uint PIXEL_READ_Y = MIPMAP_HEIGHT / WORKGROUP_SIZE_Y;

layout(binding = 0) uniform sampler2D u_isRt;

layout(std140, binding = 0) buffer _blk
{
	vec4 u_averageLuminancePad3;
};

shared float g_avgLum[WORKGROUP_SIZE];

void main()
{
	// Gather the log-average luminance of a tile
	float avgLum = 0.0;
	uint yStart = gl_LocalInvocationID.y * PIXEL_READ_Y;
	uint xStart = gl_LocalInvocationID.x * PIXEL_READ_X;
	for(uint y = 0; y < PIXEL_READ_Y; ++y)
	{
		for(uint x = 0; x < PIXEL_READ_X; ++x)
		{
			vec3 color = texelFetchOffset(
				u_isRt, ivec2(xStart, yStart), IS_RT_MIPMAP, ivec2(x, y)).rgb;
			float lum = computeLuminance(color);
			//avgLum += log(lum);
			avgLum += lum / float(MIPMAP_WIDTH * MIPMAP_HEIGHT);
		}
	}

	//avgLum *= 1.0 / float(PIXEL_READ_X * PIXEL_READ_Y);
	g_avgLum[gl_LocalInvocationIndex] = avgLum;

	memoryBarrierShared();
	barrier();

	// Gather the results into one
	for(uint s = WORKGROUP_SIZE / 2u; s > 0u; s >>= 1u)
	{
		if(gl_LocalInvocationIndex < s)
		{
			g_avgLum[gl_LocalInvocationIndex] +=
				g_avgLum[gl_LocalInvocationIndex + s];
		}

		memoryBarrierShared();
		barrier();
	}

	// Write the result
	if(gl_LocalInvocationIndex == 0)
	{
		float crntLum = g_avgLum[0];
		//crntLum = exp(crntLum / float(WORKGROUP_SIZE));
		crntLum = max(crntLum, 0.04);

#if 1
		float prevLum = u_averageLuminancePad3.x;

		// Lerp between previous and new L value
		const float INTERPOLATION_FACTOR = 0.05;
		u_averageLuminancePad3.x = prevLum * (1.0 - INTERPOLATION_FACTOR)
			+ crntLum * INTERPOLATION_FACTOR;
#else
		u_averageLuminancePad3.x = crntLum;
#endif
	}
}



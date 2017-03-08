// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"
#include "shaders/Tonemapping.glsl"

const uint WORKGROUP_SIZE_X = 16u;
const uint WORKGROUP_SIZE_Y = 16u;
const uint WORKGROUP_SIZE = WORKGROUP_SIZE_X * WORKGROUP_SIZE_Y;

layout(local_size_x = WORKGROUP_SIZE_X, local_size_y = WORKGROUP_SIZE_Y, local_size_z = 1) in;

const uint PIXEL_READ_X = INPUT_TEX_SIZE.x / WORKGROUP_SIZE_X;
const uint PIXEL_READ_Y = INPUT_TEX_SIZE.y / WORKGROUP_SIZE_Y;

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_tex;

layout(std140, ANKI_SS_BINDING(0, 0)) buffer ss0_
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
			ivec2 uv = ivec2(xStart, yStart) + ivec2(x, y);
			vec3 color = texelFetch(u_tex, uv, 0).rgb;
			float lum = computeLuminance(color);
			// avgLum += log(lum);
			avgLum += lum / float(INPUT_TEX_SIZE.x * INPUT_TEX_SIZE.y);
		}
	}

	// avgLum *= 1.0 / float(PIXEL_READ_X * PIXEL_READ_Y);
	g_avgLum[gl_LocalInvocationIndex] = avgLum;

	memoryBarrierShared();
	barrier();

	// Gather the results into one
	for(uint s = WORKGROUP_SIZE / 2u; s > 0u; s >>= 1u)
	{
		if(gl_LocalInvocationIndex < s)
		{
			g_avgLum[gl_LocalInvocationIndex] += g_avgLum[gl_LocalInvocationIndex + s];
		}

		memoryBarrierShared();
		barrier();
	}

	// Write the result
	if(gl_LocalInvocationIndex == 0)
	{
		float crntLum = g_avgLum[0];
		// crntLum = exp(crntLum / float(WORKGROUP_SIZE));
		crntLum = max(crntLum, 0.04);

#if 1
		float prevLum = u_averageLuminancePad3.x;

		// Lerp between previous and new L value
		const float INTERPOLATION_FACTOR = 0.05;
		u_averageLuminancePad3.x = prevLum * (1.0 - INTERPOLATION_FACTOR) + crntLum * INTERPOLATION_FACTOR;
#else
		u_averageLuminancePad3.x = crntLum;
#endif
	}
}

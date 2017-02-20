// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Compute the irradiance given an environment map

#include "shaders/Functions.glsl"

const float INDIRECT_BUMP = 2.5; // A sort of hack

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec3 out_color;

layout(ANKI_TEX_BINDING(0, 0)) uniform samplerCubeArray u_envTex;

layout(ANKI_UBO_BINDING(0, 0)) uniform u0_
{
	// x: The face index to render to
	// y: The array index to read from the u_envTex
	uvec4 u_faceIdxArrayIdxPad2;
};

// Integrate the environment map to compute the irradiance for a single fragment
void main()
{
	uint face = u_faceIdxArrayIdxPad2.x;
	float texArrIdx = float(u_faceIdxArrayIdxPad2.y);

	// Get the r coordinate of the current face and fragment
	vec3 ri = getCubemapDirection(UV_TO_NDC(in_uv), face);

	vec3 outCol = vec3(0.0);
	float weight = EPSILON;

	// For all the faces and texels of the environment map calculate a color sum
	for(uint f = 0u; f < 6u; ++f)
	{
		for(uint i = 0u; i < CUBEMAP_SIZE; ++i)
		{
			for(uint j = 0u; j < CUBEMAP_SIZE; ++j)
			{
				vec2 uv = vec2(j, i) / float(CUBEMAP_SIZE);
				vec3 r = getCubemapDirection(UV_TO_NDC(uv), f);

				float lambert = dot(r, ri);
				if(lambert > 0.0)
				{
					vec3 col = textureLod(u_envTex, vec4(r, texArrIdx), 0.0).rgb;

					outCol += col * lambert;
					weight += lambert;
				}
			}
		}
	}

	out_color = outCol / weight;
}

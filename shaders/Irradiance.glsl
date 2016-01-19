// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Compute the irradiance given an environment map

#include "shaders/Common.glsl"

layout(location = 0) in vec2 in_uv;

layout(TEX_BINDING(0, 0)) samplerCube u_envTex;

layout(location = 0) out vec3 out_color0;
layout(location = 1) out vec3 out_color1;
layout(location = 2) out vec3 out_color2;

const mat3 CUBE_ROTATIONS[6] = mat3[](mat3(vec3(0.000000, 0.000000, -1.000000),
										  vec3(0.000000, -1.000000, 0.000000),
										  vec3(-1.000000, 0.000000, 0.000000)),

	mat3(vec3(0.000000, 0.000000, 1.000000),
										  vec3(0.000000, -1.000000, 0.000000),
										  vec3(1.000000, 0.000000, 0.000000)),

	mat3(vec3(1.000000, 0.000000, -0.000000),
										  vec3(0.000000, -0.000000, 1.000000),
										  vec3(0.000000, -1.000000, -0.000000)),

	mat3(vec3(1.000000, 0.000000, -0.000000),
										  vec3(0.000000, -0.000000, -1.000000),
										  vec3(-0.000000, 1.000000, -0.000000)),
	mat3(vec3(1.000000, 0.000000, 0.000000),
										  vec3(0.000000, -1.000000, 0.000000),
										  vec3(0.000000, 0.000000, -1.000000)),
	mat3(vec3(-1.000000, -0.000000, 0.000000),
										  vec3(0.000000, -1.000000, 0.000000),
										  vec3(0.000000, 0.000000, 1.000000)));

//==============================================================================
// Integrate the environment map to compute the irradiance for a single fragment
vec3 computeIrradiance(in uint face)
{
	// Get the r coordinate of the current face and fragment
	vec2 ndc = in_uv * 2.0 - 1.0;
	vec3 ri = CUBE_ROTATIONS[face] * normalize(vec3(ndc, -1.0));

	vec3 outCol = vec3(0.0);
	float weight = 0.0;
	
	// For all the faces and texels of the environment map calculate a color sum
	for(uint f = 0u; f < 6u; ++f)
	{
		for(uint i = 0u; i < CUBEMAP_SIZE; ++i)
		{
			for(uint j = 0u; j < CUBEMAP_SIZE; ++j)
			{
				vec2 uv = vec2(j, i) / float(CUBEMAP_SIZE);
				vec2 ndc = uv * 2.0 - 1.0;
				vec3 r = CUBE_ROTATIONS[f] * normalize(vec3(ndc, -1.0));

				vec3 col = texture(u_envTex, r).rgb;

				float lambert = max(0.0, dot(r, ri));
				outCol += col * lambert;
				weight += lambert;
			}
		}
	}

	return outCol / weight;
}

//==============================================================================
void main()
{
	out_color0 = computeIrradiance(FACE_INDEX + 0);
	out_color1 = computeIrradiance(FACE_INDEX + 1);
	out_color2 = computeIrradiance(FACE_INDEX + 2);
}

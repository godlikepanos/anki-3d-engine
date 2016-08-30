// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Compute the irradiance given an environment map

#include "shaders/Common.glsl"

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec3 out_color;

layout(ANKI_TEX_BINDING(0, 0)) uniform samplerCubeArray u_envTex;

layout(ANKI_UBO_BINDING(0, 0)) uniform u0_
{
	// x: The face index to render to
	// y: The array index to read from the u_envTex
	uvec4 u_faceIdxArrayIdxPad2;
};

const mat3 CUBE_ROTATIONS[6] = mat3[](mat3(vec3(0.0, 0.0, -1.0), vec3(0.0, -1.0, 0.0), vec3(-1.0, 0.0, 0.0)),
	mat3(vec3(0.0, 0.0, 1.0), vec3(0.0, -1.0, 0.0), vec3(1.0, 0.0, 0.0)),
	mat3(vec3(1.0, 0.0, -0.0), vec3(0.0, -0.0, 1.0), vec3(0.0, -1.0, -0.0)),
	mat3(vec3(1.0, 0.0, -0.0), vec3(0.0, -0.0, -1.0), vec3(-0.0, 1.0, -0.0)),
	mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, -1.0, 0.0), vec3(0.0, 0.0, -1.0)),
	mat3(vec3(-1.0, -0.0, 0.0), vec3(0.0, -1.0, 0.0), vec3(0.0, 0.0, 1.0)));

// Integrate the environment map to compute the irradiance for a single fragment
void main()
{
	uint face = u_faceIdxArrayIdxPad2.x;
	float texArrIdx = float(u_faceIdxArrayIdxPad2.y);

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

				vec3 col = texture(u_envTex, vec4(r, texArrIdx)).rgb;

				float lambert = max(0.0, dot(r, ri));
				outCol += col * lambert;
				weight += lambert;
			}
		}
	}

	out_color = outCol / weight;
}

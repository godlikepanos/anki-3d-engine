// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_GBUFFER_COMMON_VERT_GLSL
#define ANKI_SHADERS_GBUFFER_COMMON_VERT_GLSL

#include "shaders/Common.glsl"

//
// Input
//
layout(location = POSITION_LOCATION) in highp vec3 in_position;
layout(location = TEXTURE_COORDINATE_LOCATION) in highp vec2 in_uv;
layout(location = NORMAL_LOCATION) in mediump vec4 in_normal;
layout(location = TANGENT_LOCATION) in mediump vec4 in_tangent;

//
// Output
//
out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out highp vec2 out_uv;
layout(location = 1) out mediump vec3 out_normal;
layout(location = 2) out mediump vec4 out_tangent;
#if CALC_BITANGENT_IN_VERT
layout(location = 3) out mediump vec3 out_bitangent;
#endif

layout(location = 4) out mediump vec3 out_vertPosViewSpace;
layout(location = 5) out mediump vec3 out_eyeTangentSpace; // Parallax
layout(location = 6) out mediump vec3 out_normalTangentSpace; // Parallax

void positionUvNormalTangent(mat4 mvp, mat3 normalMat)
{
	out_uv = in_uv;
	ANKI_WRITE_POSITION(mvp * vec4(in_position, 1.0));

	out_normal = normalMat * in_normal.xyz;
	out_tangent.xyz = normalMat * in_tangent.xyz;
	out_tangent.w = in_tangent.w;

#if CALC_BITANGENT_IN_VERT
	out_bitangent = cross(out_normal, out_tangent.xyz) * in_tangent.w;
#endif
}

void vertPosViewSpace(in mat4 modelViewMat)
{
	out_vertPosViewSpace = vec3(modelViewMat * vec4(in_position, 1.0));
}

void parallax(in mat4 modelViewMat)
{
	vec3 n = out_normal;
	vec3 t = out_tangent.xyz;
#if CALC_BITANGENT_IN_VERT
	vec3 b = out_bitangent;
#else
	vec3 b = cross(n, t) * in_tangent.w;
#endif
	mat3 invTbn = transpose(mat3(t, b, n));

	vertPosViewSpace(modelViewMat);

	out_eyeTangentSpace = invTbn * out_vertPosViewSpace;
	out_normalTangentSpace = invTbn * n;
}

#endif

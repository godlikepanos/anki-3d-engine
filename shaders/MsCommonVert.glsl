// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_MS_COMMON_VERT_GLSL
#define ANKI_SHADERS_MS_COMMON_VERT_GLSL

#include "shaders/MsFsCommon.glsl"

//
// Input
//
layout(location = POSITION_LOCATION) in highp vec3 in_position;
layout(location = TEXTURE_COORDINATE_LOCATION) in highp vec2 in_uv;

#if PASS == COLOR || TESSELLATION
layout(location = NORMAL_LOCATION) in mediump vec4 in_normal;
#endif

#if PASS == COLOR
layout(location = TANGENT_LOCATION) in mediump vec4 in_tangent;
#endif

//
// Output
//
out gl_PerVertex
{
	vec4 gl_Position;
};

#if NVIDIA_LINK_ERROR_WORKAROUND
layout(location = 0) out highp vec4 out_uv;
#else
layout(location = 0) out highp vec2 out_uv;
#endif

#if PASS == COLOR || TESSELLATION
layout(location = 1) out mediump vec3 out_normal;
#endif

#if PASS == COLOR
layout(location = 2) out mediump vec4 out_tangent;
#if CALC_BITANGENT_IN_VERT
layout(location = 3) out mediump vec3 out_bitangent;
#endif

layout(location = 4) out mediump vec3 out_vertPosViewSpace;
layout(location = 5) out mediump vec3 out_eyeTangentSpace; // Parallax
layout(location = 6) out mediump vec3 out_normalTangentSpace; // Parallax

#endif

#define writePositionAndUv_DEFINED
void writePositionAndUv(in mat4 mvp)
{
#if PASS == DEPTH && LOD > 0
// No tex coords for you
#else

#if NVIDIA_LINK_ERROR_WORKAROUND
	out_uv = vec4(in_uv, 0.0, 0.0);
#else
	out_uv = in_uv;
#endif

#endif

#if TESSELLATION
	gl_Position = vec4(in_position, 1.0);
#else
	ANKI_WRITE_POSITION(mvp * vec4(in_position, 1.0));
#endif
}

#if PASS == COLOR
#define writeNormalAndTangent_DEFINED
void writeNormalAndTangent(in mat3 normalMat)
{

#if TESSELLATION

	// Passthrough
	out_normal = in_normal.xyz;
#if PASS == COLOR
	out_tangent = in_tangent;

#if CALC_BITANGENT_IN_VERT
#error TODO
#endif

#endif

#else

#if PASS == COLOR
	out_normal = normalMat * in_normal.xyz;
	out_tangent.xyz = normalMat * in_tangent.xyz;
	out_tangent.w = in_tangent.w;

#if CALC_BITANGENT_IN_VERT
	out_bitangent = cross(out_normal, out_tangent.xyz) * in_tangent.w;
#endif

#endif

// #if TESSELLATION
#endif
}
#endif

#if PASS == COLOR
#define writeVertPosViewSpace_DEFINED
void writeVertPosViewSpace(in mat4 modelViewMat)
{
	out_vertPosViewSpace = vec3(modelViewMat * vec4(in_position, 1.0));
}
#endif

#if PASS == COLOR
#define writeParallax_DEFINED
void writeParallax(in mat3 normalMat, in mat4 modelViewMat)
{
	vec3 n = normalMat * in_normal.xyz;
	vec3 t = normalMat * in_tangent.xyz;
	vec3 b = cross(n, t) * in_tangent.w;
	mat3 invTbn = transpose(mat3(t, b, n));

	writeVertPosViewSpace(modelViewMat);

	out_eyeTangentSpace = invTbn * out_vertPosViewSpace;
	out_normalTangentSpace = invTbn * n;
}
#endif

#endif

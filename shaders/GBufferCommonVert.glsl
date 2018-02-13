// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_GBUFFER_COMMON_VERT_GLSL
#define ANKI_SHADERS_GBUFFER_COMMON_VERT_GLSL

#include "shaders/Common.glsl"

//
// Uniforms
//
#if BONES
layout(ANKI_SS_BINDING(0, 0), row_major) readonly buffer ss00_
{
	mat4 u_boneTransforms[];
};
#endif

//
// Input
//
layout(location = POSITION_LOCATION) in highp vec3 in_position;
#if PASS == PASS_GB_FS
layout(location = TEXTURE_COORDINATE_LOCATION) in highp vec2 in_uv;
layout(location = NORMAL_LOCATION) in mediump vec3 in_normal;
layout(location = TANGENT_LOCATION) in mediump vec4 in_tangent;
#endif

#if BONES
layout(location = BONE_WEIGHTS_LOCATION) in mediump vec4 in_boneWeights;
layout(location = BONE_INDICES_LOCATION) in uvec4 in_boneIndices;
#endif

//
// Output
//
out gl_PerVertex
{
	vec4 gl_Position;
};

#if PASS == PASS_GB_FS
layout(location = 0) out highp vec2 out_uv;
layout(location = 1) out mediump vec3 out_normal;
layout(location = 2) out mediump vec4 out_tangent;
#	if CALC_BITANGENT_IN_VERT
layout(location = 3) out mediump vec3 out_bitangent;
#	endif
layout(location = 4) out mediump float out_distFromTheCamera; // Parallax
layout(location = 5) out mediump vec3 out_eyeTangentSpace; // Parallax
layout(location = 6) out mediump vec3 out_normalTangentSpace; // Parallax
#endif

//
// Globals
//
vec3 g_position = in_position;
#if PASS == PASS_GB_FS
highp vec2 g_uv = in_uv;
mediump vec3 g_normal = in_normal;
mediump vec4 g_tangent = in_tangent;
#endif

//
// Functions
//

// Common store function
#if PASS == PASS_GB_FS
void positionUvNormalTangent(mat4 mvp, mat3 rotationMat)
{
	out_uv = g_uv;
	gl_Position = mvp * vec4(g_position, 1.0);

	out_normal = rotationMat * g_normal.xyz;
	out_tangent.xyz = rotationMat * g_tangent.xyz;
	out_tangent.w = g_tangent.w;

#	if CALC_BITANGENT_IN_VERT
	out_bitangent = cross(out_normal, out_tangent.xyz) * out_tangent.w;
#	endif
}
#endif // PASS == PASS_GB_FS

// Store stuff for parallax mapping
#if PASS == PASS_GB_FS
void parallax(mat4 modelViewMat)
{
	vec3 n = in_normal;
	vec3 t = in_tangent.xyz;
	vec3 b = cross(n, t) * in_tangent.w;

	mat3 normalMat = mat3(modelViewMat);
	mat3 invTbn = transpose(normalMat * mat3(t, b, n));

	vec3 viewPos = (modelViewMat * vec4(g_position, 1.0)).xyz;
	out_distFromTheCamera = viewPos.z;

	out_eyeTangentSpace = invTbn * viewPos;
	out_normalTangentSpace = invTbn * n;
}
#endif // PASS == PASS_GB_FS

/// Will compute new position, normal and tangent
#if BONES
void skinning()
{
	vec3 position = vec3(0.0);
	vec3 normal = vec3(0.0);
	vec3 tangent = vec3(0.0);
	for(uint i = 0; i < 4; ++i)
	{
		uint boneIdx = in_boneIndices[i];
		if(boneIdx < 0xFFFF)
		{
			float boneWeight = in_boneWeights[i];

			position += (u_boneTransforms[boneIdx] * vec4(g_position * boneWeight, 1.0)).xyz;
#	if PASS == PASS_GB_FS
			normal += (u_boneTransforms[boneIdx] * vec4(g_normal * boneWeight, 0.0)).xyz;
			tangent += (u_boneTransforms[boneIdx] * vec4(g_tangent.xyz * boneWeight, 0.0)).xyz;
#	endif
		}
	}

	g_position = position;
#	if PASS == PASS_GB_FS
	g_tangent.xyz = tangent;
	g_normal = normal;
#	endif
}
#endif

#endif

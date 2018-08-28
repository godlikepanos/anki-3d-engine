// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <shaders/Common.glsl>

//
// Uniforms
//
#if BONES
layout(ANKI_SS_BINDING(0, 0), row_major) readonly buffer ss00_
{
	Mat4 u_boneTransforms[];
};
#endif

//
// Input
//
layout(location = POSITION_LOCATION) in highp Vec3 in_position;
#if PASS == PASS_GB
layout(location = TEXTURE_COORDINATE_LOCATION) in highp Vec2 in_uv;
layout(location = NORMAL_LOCATION) in mediump Vec3 in_normal;
layout(location = TANGENT_LOCATION) in mediump Vec4 in_tangent;
#endif

#if BONES
layout(location = BONE_WEIGHTS_LOCATION) in mediump Vec4 in_boneWeights;
layout(location = BONE_INDICES_LOCATION) in UVec4 in_boneIndices;
#endif

//
// Output
//
out gl_PerVertex
{
	Vec4 gl_Position;
};

#if PASS == PASS_GB
layout(location = 0) out highp Vec2 out_uv;
layout(location = 1) out mediump Vec3 out_normal;
layout(location = 2) out mediump Vec4 out_tangent;
#	if CALC_BITANGENT_IN_VERT
layout(location = 3) out mediump Vec3 out_bitangent;
#	endif
layout(location = 4) out mediump F32 out_distFromTheCamera; // Parallax
layout(location = 5) out mediump Vec3 out_eyeTangentSpace; // Parallax
layout(location = 6) out mediump Vec3 out_normalTangentSpace; // Parallax

#	if VELOCITY
layout(location = 7) out mediump Vec2 out_velocity; // Velocity
#	endif
#endif

//
// Globals
//
Vec3 g_position = in_position;
#if PASS == PASS_GB
highp Vec2 g_uv = in_uv;
mediump Vec3 g_normal = in_normal;
mediump Vec4 g_tangent = in_tangent;
#endif

//
// Functions
//

// Common store function
#if PASS == PASS_GB
void positionUvNormalTangent(Mat4 mvp, Mat3 rotationMat)
{
	out_uv = g_uv;
	gl_Position = mvp * Vec4(g_position, 1.0);

	out_normal = rotationMat * g_normal.xyz;
	out_tangent.xyz = rotationMat * g_tangent.xyz;
	out_tangent.w = g_tangent.w;

#	if CALC_BITANGENT_IN_VERT
	out_bitangent = cross(out_normal, out_tangent.xyz) * out_tangent.w;
#	endif
}
#endif // PASS == PASS_GB

// Store stuff for parallax mapping
#if PASS == PASS_GB
void parallax(Mat4 modelViewMat)
{
	Vec3 n = in_normal;
	Vec3 t = in_tangent.xyz;
	Vec3 b = cross(n, t) * in_tangent.w;

	Mat3 normalMat = Mat3(modelViewMat);
	Mat3 invTbn = transpose(normalMat * Mat3(t, b, n));

	Vec3 viewPos = (modelViewMat * Vec4(g_position, 1.0)).xyz;
	out_distFromTheCamera = viewPos.z;

	out_eyeTangentSpace = invTbn * viewPos;
	out_normalTangentSpace = invTbn * n;
}
#endif // PASS == PASS_GB

/// Will compute new position, normal and tangent
#if BONES
void skinning()
{
	Vec3 position = Vec3(0.0);
	Vec3 normal = Vec3(0.0);
	Vec3 tangent = Vec3(0.0);
	for(U32 i = 0; i < 4; ++i)
	{
		U32 boneIdx = in_boneIndices[i];
		if(boneIdx < 0xFFFF)
		{
			F32 boneWeight = in_boneWeights[i];

			position += (u_boneTransforms[boneIdx] * Vec4(g_position * boneWeight, 1.0)).xyz;
#	if PASS == PASS_GB
			normal += (u_boneTransforms[boneIdx] * Vec4(g_normal * boneWeight, 0.0)).xyz;
			tangent += (u_boneTransforms[boneIdx] * Vec4(g_tangent.xyz * boneWeight, 0.0)).xyz;
#	endif
		}
	}

	g_position = position;
#	if PASS == PASS_GB
	g_tangent.xyz = tangent;
	g_normal = normal;
#	endif
}
#endif

#if VELOCITY && PASS == PASS_GB
void velocity(Mat4 prevMvp)
{
	Vec4 v4 = prevMvp * Vec4(g_position, 1.0);
	Vec2 prevNdc = v4.xy / v4.w;

	Vec2 crntNdc = gl_Position.xy / gl_Position.w;

	// It's NDC_TO_UV(prevNdc) - NDC_TO_UV(crntNdc) or:
	out_velocity = (prevNdc - crntNdc) * 0.5;
}
#endif

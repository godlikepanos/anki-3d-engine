// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Defines the interfaces for the programs that write the GBuffer

#pragma once

#include <anki/shaders/Pack.glsl>

//
// Vert input
//
#if defined(ANKI_VERTEX_SHADER)
layout(location = POSITION_LOCATION) in Vec3 in_position;
#	if ANKI_PASS == PASS_GB
layout(location = TEXTURE_COORDINATE_LOCATION) in Vec2 in_uv;
layout(location = NORMAL_LOCATION) in Vec3 in_normal;
layout(location = TANGENT_LOCATION) in Vec4 in_tangent;
#	endif

#	if ANKI_BONES
layout(location = BONE_WEIGHTS_LOCATION) in Vec4 in_boneWeights;
layout(location = BONE_INDICES_LOCATION) in UVec4 in_boneIndices;
#	endif
#endif

//
// Vert out
//
#if defined(ANKI_VERTEX_SHADER)
out gl_PerVertex
{
	Vec4 gl_Position;
};

#	if ANKI_PASS == PASS_GB
layout(location = 0) out Vec2 out_uv;
layout(location = 1) out Vec3 out_normal;
layout(location = 2) out Vec3 out_tangent;
layout(location = 3) out Vec3 out_bitangent;

#		if REALLY_USING_PARALLAX
layout(location = 4) out F32 out_distFromTheCamera;
layout(location = 5) out Vec3 out_eyeTangentSpace;
layout(location = 6) out Vec3 out_normalTangentSpace;
#		endif

#		if ANKI_VELOCITY || ANKI_BONES
layout(location = 7) out Vec2 out_velocity;
#		endif
#	endif // ANKI_PASS == PASS_GB
#endif // defined(ANKI_VERTEX_SHADER)

//
// Frag input
//
#if defined(ANKI_FRAGMENT_SHADER) && ANKI_PASS == PASS_GB
layout(location = 0) in Vec2 in_uv;
layout(location = 1) in Vec3 in_normal;
layout(location = 2) in Vec3 in_tangent;
layout(location = 3) in Vec3 in_bitangent;

#	if REALLY_USING_PARALLAX
layout(location = 4) in F32 in_distFromTheCamera;
layout(location = 5) in Vec3 in_eyeTangentSpace;
layout(location = 6) in Vec3 in_normalTangentSpace;
#	endif

#	if ANKI_VELOCITY || ANKI_BONES
layout(location = 7) in Vec2 in_velocity;
#	endif
#endif

//
// Frag out
//
#if defined(ANKI_FRAGMENT_SHADER) && (ANKI_PASS == PASS_GB || ANKI_PASS == PASS_EZ)
layout(location = 0) out Vec4 out_gbuffer0;
layout(location = 1) out Vec4 out_gbuffer1;
layout(location = 2) out Vec4 out_gbuffer2;
layout(location = 3) out Vec2 out_gbuffer3;
#endif

//
// Functions
//

// Write the data to RTs
#if defined(ANKI_FRAGMENT_SHADER) && ANKI_PASS == PASS_GB
void writeGBuffer(Vec3 diffColor, Vec3 normal, Vec3 specularColor, F32 roughness, F32 subsurface, Vec3 emission,
				  F32 metallic, Vec2 velocity)
{
	GbufferInfo g;
	g.m_diffuse = diffColor;
	g.m_normal = normal;
	g.m_specular = specularColor;
	g.m_roughness = roughness;
	g.m_subsurface = subsurface;
	g.m_emission = (emission.r + emission.g + emission.b) / 3.0;
	g.m_metallic = metallic;
	g.m_velocity = velocity;
	writeGBuffer(g, out_gbuffer0, out_gbuffer1, out_gbuffer2, out_gbuffer3);
}
#endif

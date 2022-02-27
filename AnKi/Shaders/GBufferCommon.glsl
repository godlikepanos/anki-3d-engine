// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Defines the interfaces for the programs that write the GBuffer

#pragma once

#include <AnKi/Shaders/PackFunctions.glsl>
#include <AnKi/Shaders/Include/ModelTypes.h>

//
// Vert input
//
#if defined(ANKI_VERTEX_SHADER)

layout(location = VERTEX_ATTRIBUTE_ID_POSITION) in Vec3 in_position;

#	if ANKI_PASS == PASS_GB
layout(location = VERTEX_ATTRIBUTE_ID_NORMAL) in Vec3 in_normal;
layout(location = VERTEX_ATTRIBUTE_ID_TANGENT) in Vec4 in_tangent;
#	endif

#	if ANKI_PASS == PASS_GB || ALPHA_TEST
layout(location = VERTEX_ATTRIBUTE_ID_UV0) in Vec2 in_uv;
#	endif

#	if ANKI_BONES
layout(location = VERTEX_ATTRIBUTE_ID_BONE_WEIGHTS) in Vec4 in_boneWeights;
layout(location = VERTEX_ATTRIBUTE_ID_BONE_INDICES) in UVec4 in_boneIndices;
#	endif

#endif // defined(ANKI_VERTEX_SHADER)

//
// Vert out
//
#if defined(ANKI_VERTEX_SHADER)

#	if ANKI_PASS == PASS_GB || ALPHA_TEST
layout(location = 0) out Vec2 out_uv;
#	endif

#	if ANKI_PASS == PASS_GB
layout(location = 1) out ANKI_RP Vec3 out_normal;
layout(location = 2) out ANKI_RP Vec3 out_tangent;
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
#if defined(ANKI_FRAGMENT_SHADER)

#	if ANKI_PASS == PASS_GB || ALPHA_TEST
layout(location = 0) in Vec2 in_uv;
#	endif

#	if ANKI_PASS == PASS_GB
layout(location = 1) in ANKI_RP Vec3 in_normal;
layout(location = 2) in ANKI_RP Vec3 in_tangent;
layout(location = 3) in Vec3 in_bitangent;

#		if REALLY_USING_PARALLAX
layout(location = 4) in F32 in_distFromTheCamera;
layout(location = 5) in Vec3 in_eyeTangentSpace;
layout(location = 6) in Vec3 in_normalTangentSpace;
#		endif

#		if ANKI_VELOCITY || ANKI_BONES
layout(location = 7) in Vec2 in_velocity;
#		endif
#	endif // ANKI_PASS == PASS_GB

#endif // defined(ANKI_FRAGMENT_SHADER)

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
void packGBuffer(ANKI_RP Vec3 diffColor, ANKI_RP Vec3 normal, ANKI_RP Vec3 specularColor, ANKI_RP F32 roughness,
				 ANKI_RP F32 subsurface, ANKI_RP Vec3 emission, ANKI_RP F32 metallic, Vec2 velocity)
{
	GbufferInfo g;
	g.m_diffuse = diffColor;
	g.m_normal = normal;
	g.m_f0 = specularColor;
	g.m_roughness = roughness;
	g.m_subsurface = subsurface;
	g.m_emission = emission;
	g.m_metallic = metallic;
	g.m_velocity = velocity;
	packGBuffer(g, out_gbuffer0, out_gbuffer1, out_gbuffer2, out_gbuffer3);
}
#endif

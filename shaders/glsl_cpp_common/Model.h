// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <shaders/glsl_cpp_common/Common.h>

ANKI_BEGIN_NAMESPACE

const U32 UV_CHANNEL_COUNT = 2;

struct Vertex
{
	U32 m_normal; // Packed in R10G10B10A2SNorm
	U32 m_tangent; // Packed in R10G10B10A2SNorm
	HVec2 m_uvs[UV_CHANNEL_COUNT];
};

const U32 SIZEOF_VERTEX = 4 * 4;

struct Mesh
{
	U64 m_indexBufferPtr; // Points to a buffer of U16
	U64 m_positionBufferPtr; // Points to a buffer of Vec3
	U64 m_vertexBufferPtr; // Points to a buffer of Vertex
	U32 m_indexCount;
	U32 m_vertexCount;
};

const U32 TEXTURE_CHANNEL_COUNT = 8;

struct Material
{
	U32 m_textureIds[TEXTURE_CHANNEL_COUNT];
	Vec3 m_diffuseColor;
	Vec3 m_specularColor;
	Vec3 m_emissiveColor;
	F32 m_roughness;
	F32 m_metallines;
};

struct ModelInstance
{
	Mesh m_mesh;
	Material m_material;
#if defined(__cplusplus)
	F32 m_worldTransform[12];
#else
	Mat3x4 m_worldTransform;
#endif
	Mat3 m_worldRotation;
};

ANKI_END_NAMESPACE

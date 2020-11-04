// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shaders/include/Common.h>

ANKI_BEGIN_NAMESPACE

const U32 UV_CHANNEL_0 = 0;
const U32 UV_CHANNEL_COUNT = 1;

/// The main vertex that contains normals, tangents and UVs
struct MainVertex
{
	U32 m_normal; ///< Packed in a custom R11G11B10_SNorm
	U32 m_tangent; ///< Packed in a custom R10G10B11A1_SNorm format
	Vec2 m_uvs[UV_CHANNEL_COUNT];
};

const U32 _ANKI_SIZEOF_MainVertex = 4 * 4;
const U32 _ANKI_ALIGNOF_MainVertex = 4;
ANKI_SHADER_STATIC_ASSERT(_ANKI_SIZEOF_MainVertex == sizeof(MainVertex));

/// The vertex that contains the bone influences.
struct BoneInfoVertex
{
	F32 m_boneIndices[4];
	U8 m_boneWeights[4];
};

const U32 _ANKI_SIZEOF_BoneInfoVertex = 5 * 4;
const U32 _ANKI_ALIGNOF_BoneInfoVertex = 4;
ANKI_SHADER_STATIC_ASSERT(_ANKI_SIZEOF_BoneInfoVertex == sizeof(BoneInfoVertex));

/// A structure that contains all the info of a geometry.
struct MeshGpuDescriptor
{
	U64 m_indexBufferPtr; ///< Points to a buffer of U16 indices.
	U64 m_positionBufferPtr; ///< Points to a buffer of Vec3 positions.
	U64 m_mainVertexBufferPtr; ///< Points to a buffer of MainVertex.
	U64 m_boneInfoVertexBufferPtr; ///< Points to a buffer of BoneInfoVertex.
	U32 m_indexCount;
	U32 m_vertexCount;
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;
};

const U32 _ANKI_SIZEOF_MeshGpuDescriptor = 4 * ANKI_SIZEOF(U64) + 8 * ANKI_SIZEOF(F32);
const U32 _ANKI_ALIGNOF_MeshGpuDescriptor = 8;
ANKI_SHADER_STATIC_ASSERT(_ANKI_SIZEOF_MeshGpuDescriptor == sizeof(MeshGpuDescriptor));

const U32 TEXTURE_CHANNEL_DIFFUSE = 0;
const U32 TEXTURE_CHANNEL_NORMAL = 1;
const U32 TEXTURE_CHANNEL_ROUGHNESS_METALNESS = 2;
const U32 TEXTURE_CHANNEL_EMISSION = 3;
const U32 TEXTURE_CHANNEL_HEIGHT = 4;
const U32 TEXTURE_CHANNEL_AUX_0 = 5;
const U32 TEXTURE_CHANNEL_AUX_1 = 6;
const U32 TEXTURE_CHANNEL_AUX_2 = 7;

const U32 TEXTURE_CHANNEL_COUNT = 8;

struct MaterialGpuDescriptor
{
	U16 m_bindlessTextureIndices[TEXTURE_CHANNEL_COUNT];
	Vec3 m_diffuseColor;
	Vec3 m_specularColor;
	Vec3 m_emissiveColor;
	F32 m_roughness;
	F32 m_metalness;
};

const U32 _ANKI_SIZEOF_MaterialGpuDescriptor =
	TEXTURE_CHANNEL_COUNT * ANKI_SIZEOF(U16) + 3 * ANKI_SIZEOF(Vec3) + 2 * ANKI_SIZEOF(F32);
const U32 _ANKI_ALIGNOF_MaterialGpuDescriptor = 4;
ANKI_SHADER_STATIC_ASSERT(_ANKI_SIZEOF_MaterialGpuDescriptor == sizeof(MaterialGpuDescriptor));

struct ModelGpuDescriptor
{
	MeshGpuDescriptor m_mesh;
	MaterialGpuDescriptor m_material;
#if defined(__cplusplus)
	F32 m_worldTransform[12];
#else
	Mat3x4 m_worldTransform;
#endif
	Mat3 m_worldRotation;
};

ANKI_END_NAMESPACE

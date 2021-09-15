// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

// Vertex attribute locations. Not all are used
#if defined(__cplusplus)
enum class VertexAttributeId : U8
{
	POSITION,
	UV0,
	UV1,
	NORMAL,
	TANGENT,
	COLOR,
	BONE_WEIGHTS,
	BONE_INDICES,

	COUNT,
	FIRST = POSITION,

	SCALE = UV0, ///< Only for particles.
	ALPHA = UV1, ///< Only for particles.
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexAttributeId)
#else
const U32 VERTEX_ATTRIBUTE_ID_POSITION = 0u;
const U32 VERTEX_ATTRIBUTE_ID_UV0 = 1u;
const U32 VERTEX_ATTRIBUTE_ID_UV1 = 2u;
const U32 VERTEX_ATTRIBUTE_ID_NORMAL = 3u;
const U32 VERTEX_ATTRIBUTE_ID_TANGENT = 4u;
const U32 VERTEX_ATTRIBUTE_ID_COLOR = 5u;
const U32 VERTEX_ATTRIBUTE_ID_BONE_WEIGHTS = 6u;
const U32 VERTEX_ATTRIBUTE_ID_BONE_INDICES = 7u;
const U32 VERTEX_ATTRIBUTE_ID_COUNT = 8u;

const U32 VERTEX_ATTRIBUTE_ID_SCALE = VERTEX_ATTRIBUTE_ID_UV0; ///< Only for particles.
const U32 VERTEX_ATTRIBUTE_ID_ALPHA = VERTEX_ATTRIBUTE_ID_UV1; ///< Only for particles.
#endif

// Vertex buffers
#if defined(__cplusplus)
enum class VertexAttributeBufferId : U8
{
	POSITION,
	NORMAL_TANGENT_UV0,
	BONE,

	COUNT
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexAttributeBufferId)
#else
const U32 VERTEX_ATTRIBUTE_BUFFER_ID_POSITION = 0u;
const U32 VERTEX_ATTRIBUTE_BUFFER_ID_NORMAL_TANGENT_UV0 = 1u;
const U32 VERTEX_ATTRIBUTE_BUFFER_ID_BONE = 2u;
const U32 VERTEX_ATTRIBUTE_BUFFER_ID_COUNT = 3u;
#endif

/// The main vertex that contains normals, tangents and UVs.
struct MainVertex
{
	U32 m_normal; ///< Packed in a custom R11G11B10_SNorm
	U32 m_tangent; ///< Packed in a custom R10G10B11A1_SNorm format
	Vec2 m_uv0;
};

const U32 _ANKI_SIZEOF_MainVertex = 4u * 4u;
const U32 _ANKI_ALIGNOF_MainVertex = 4u;
ANKI_SHADER_STATIC_ASSERT(_ANKI_SIZEOF_MainVertex == sizeof(MainVertex));

/// The vertex that contains the bone influences.
struct BoneInfoVertex
{
	U8Vec4 m_boneIndices;
	U8Vec4 m_boneWeights;
};

const U32 _ANKI_SIZEOF_BoneInfoVertex = 8u;
const U32 _ANKI_ALIGNOF_BoneInfoVertex = 1u;
ANKI_SHADER_STATIC_ASSERT(_ANKI_SIZEOF_BoneInfoVertex == sizeof(BoneInfoVertex));

/// A structure that contains all the info of a geometry.
struct MeshGpuDescriptor
{
	Address m_indexBufferPtr; ///< Points to a buffer of U16 indices.
#if defined(__cplusplus)
	Array<Address, U(VertexAttributeBufferId::COUNT)> m_vertexBufferPtrs;
#else
	Address m_vertexBufferPtrs[VERTEX_ATTRIBUTE_BUFFER_ID_COUNT];
#endif
	U32 m_indexCount;
	U32 m_vertexCount;
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;
};

const U32 _ANKI_SIZEOF_MeshGpuDescriptor = 4u * ANKI_SIZEOF(UVec2) + 8u * ANKI_SIZEOF(F32);
const U32 _ANKI_ALIGNOF_MeshGpuDescriptor = 8u;
ANKI_SHADER_STATIC_ASSERT(_ANKI_SIZEOF_MeshGpuDescriptor == sizeof(MeshGpuDescriptor));

#if defined(__cplusplus)
enum class TextureChannelId : U8
{
	DIFFUSE,
	NORMAL,
	ROUGHNESS_METALNESS,
	EMISSION,
	HEIGHT,
	AUX_0,
	AUX_1,
	AUX_2,

	COUNT
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TextureChannelId)
#else
const U32 TEXTURE_CHANNEL_ID_DIFFUSE = 0u;
const U32 TEXTURE_CHANNEL_ID_NORMAL = 1u;
const U32 TEXTURE_CHANNEL_ID_ROUGHNESS_METALNESS = 2u;
const U32 TEXTURE_CHANNEL_ID_EMISSION = 3u;
const U32 TEXTURE_CHANNEL_ID_HEIGHT = 4u;
const U32 TEXTURE_CHANNEL_ID_AUX_0 = 5u;
const U32 TEXTURE_CHANNEL_ID_AUX_1 = 6u;
const U32 TEXTURE_CHANNEL_ID_AUX_2 = 7u;
const U32 TEXTURE_CHANNEL_ID_COUNT = 8u;
#endif

struct MaterialGpuDescriptor
{
#if defined(__cplusplus)
	Array<U16, U(TextureChannelId::COUNT)> m_bindlessTextureIndices;
#else
	U16 m_bindlessTextureIndices[TEXTURE_CHANNEL_ID_COUNT];
#endif
	Vec3 m_diffuseColor;
	Vec3 m_specularColor;
	Vec3 m_emissiveColor;
	F32 m_roughness;
	F32 m_metalness;
};

const U32 _ANKI_SIZEOF_MaterialGpuDescriptor = 8u * ANKI_SIZEOF(U16) + 3u * ANKI_SIZEOF(Vec3) + 2u * ANKI_SIZEOF(F32);
const U32 _ANKI_ALIGNOF_MaterialGpuDescriptor = 4u;
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

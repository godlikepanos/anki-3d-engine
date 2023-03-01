// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
	kPosition,
	kUv0,
	kUv1,
	kNormal,
	kTangent,
	kColor,
	kBoneWeights,
	kBoneIndices,

	kCount,
	kFirst = kPosition,

	SCALE = kUv0, ///< Only for particles.
	ALPHA = kUv1, ///< Only for particles.
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexAttributeId)
#else
constexpr U32 kVertexAttributeIdPosition = 0u;
constexpr U32 kVertexAttributeIdUv0 = 1u;
constexpr U32 kVertexAttributeIdUv1 = 2u;
constexpr U32 kVertexAttributeIdNormal = 3u;
constexpr U32 kVertexAttributeIdTangent = 4u;
constexpr U32 kVertexAttributeIdColor = 5u;
constexpr U32 kVertexAttributeIdBoneWeights = 6u;
constexpr U32 kVertexAttributeIdBoneIndices = 7u;
constexpr U32 kVertexAttributeIdCount = 8u;

constexpr U32 kVertexAttributeIdScale = kVertexAttributeIdUv0; ///< Only for particles.
constexpr U32 kVertexAttributeIdAlpha = kVertexAttributeIdUv1; ///< Only for particles.
#endif

// Vertex buffers
#if defined(__cplusplus)
enum class VertexAttributeBufferId : U8
{
	kPosition,
	kNormalTangentUv0,
	kBone,

	kCount
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexAttributeBufferId)
#else
constexpr U32 kVertexAttributeBufferIdPosition = 0u;
constexpr U32 kVertexAttributeBufferIdNormalTangentUv0 = 1u;
constexpr U32 kVertexAttributeBufferIdBone = 2u;
constexpr U32 kVertexAttributeBufferIdCount = 3u;
#endif

/// The main vertex that contains normals, tangents and UVs.
struct MainVertex
{
	U32 m_normal; ///< Packed in a custom R11G11B10_SNorm
	U32 m_tangent; ///< Packed in a custom R11G10B10A1_SNorm format
	Vec2 m_uv0;
};

constexpr U32 kSizeof_MainVertex = 4u * 4u;
constexpr U32 kAlignof_MainVertex = 4u;
ANKI_SHADER_STATIC_ASSERT(kSizeof_MainVertex == sizeof(MainVertex));

/// The vertex that contains the bone influences.
struct BoneInfoVertex
{
	U8Vec4 m_boneIndices;
	U8Vec4 m_boneWeights;
};

constexpr U32 kSizeof_BoneInfoVertex = 8u;
constexpr U32 kAlignof_BoneInfoVertex = 1u;
ANKI_SHADER_STATIC_ASSERT(kSizeof_BoneInfoVertex == sizeof(BoneInfoVertex));

/// A structure that contains all the info of a geometry.
struct MeshGpuDescriptor
{
	Address m_indexBufferPtr; ///< Points to a buffer of U16 indices.
#if defined(__cplusplus)
	Array<Address, U(VertexAttributeBufferId::kCount)> m_vertexBufferPtrs;
#else
	Address m_vertexBufferPtrs[kVertexAttributeBufferIdCount];
#endif
	U32 m_indexCount;
	U32 m_vertexCount;
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;
};

constexpr U32 kSizeof_MeshGpuDescriptor = 4u * sizeof(UVec2) + 8u * sizeof(F32);
constexpr U32 kAlignof_MeshGpuDescriptor = 8u;
ANKI_SHADER_STATIC_ASSERT(kSizeof_MeshGpuDescriptor == sizeof(MeshGpuDescriptor));

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
constexpr U32 TEXTURE_CHANNEL_ID_DIFFUSE = 0u;
constexpr U32 TEXTURE_CHANNEL_ID_NORMAL = 1u;
constexpr U32 TEXTURE_CHANNEL_ID_ROUGHNESS_METALNESS = 2u;
constexpr U32 TEXTURE_CHANNEL_ID_EMISSION = 3u;
constexpr U32 TEXTURE_CHANNEL_ID_HEIGHT = 4u;
constexpr U32 TEXTURE_CHANNEL_ID_AUX_0 = 5u;
constexpr U32 TEXTURE_CHANNEL_ID_AUX_1 = 6u;
constexpr U32 TEXTURE_CHANNEL_ID_AUX_2 = 7u;
constexpr U32 TEXTURE_CHANNEL_ID_COUNT = 8u;
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

constexpr U32 kSizeof_MaterialGpuDescriptor = 8u * sizeof(U16) + 3u * sizeof(Vec3) + 2u * sizeof(F32);
constexpr U32 kAlignof_MaterialGpuDescriptor = 4u;
ANKI_SHADER_STATIC_ASSERT(kSizeof_MaterialGpuDescriptor == sizeof(MaterialGpuDescriptor));

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

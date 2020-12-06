// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#pragma once

#include <anki/resource/Common.h>
#include <anki/Math.h>

namespace anki
{

/// @addtogroup resource
/// @{

static constexpr const char* MESH_MAGIC = "ANKIMES5";

enum class MeshBinaryFlag : U32
{
	NONE = 0,
	QUAD = 1 << 0,
	CONVEX = 1 << 1,

	ALL = QUAD | CONVEX,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(MeshBinaryFlag)

/// Vertex buffer info. The size of the buffer is m_vertexStride*MeshBinaryHeader::m_totalVertexCount aligned to 16.
class MeshBinaryVertexBuffer
{
public:
	U32 m_vertexStride; ///< The size of the vertex.

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_vertexStride", offsetof(MeshBinaryVertexBuffer, m_vertexStride), self.m_vertexStride);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, MeshBinaryVertexBuffer&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const MeshBinaryVertexBuffer&>(serializer, *this);
	}
};

/// Vertex attribute.
class MeshBinaryVertexAttribute
{
public:
	U32 m_bufferBinding;
	Format m_format; ///< If the format is NONE then the attribute is not present.
	U32 m_relativeOffset;
	F32 m_scale;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_bufferBinding", offsetof(MeshBinaryVertexAttribute, m_bufferBinding), self.m_bufferBinding);
		s.doValue("m_format", offsetof(MeshBinaryVertexAttribute, m_format), self.m_format);
		s.doValue("m_relativeOffset", offsetof(MeshBinaryVertexAttribute, m_relativeOffset), self.m_relativeOffset);
		s.doValue("m_scale", offsetof(MeshBinaryVertexAttribute, m_scale), self.m_scale);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, MeshBinaryVertexAttribute&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const MeshBinaryVertexAttribute&>(serializer, *this);
	}
};

/// MeshBinarySubMesh class.
class MeshBinarySubMesh
{
public:
	U32 m_firstIndex;
	U32 m_indexCount;
	Vec3 m_aabbMin; ///< Bounding box min.
	Vec3 m_aabbMax; ///< Bounding box max.

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_firstIndex", offsetof(MeshBinarySubMesh, m_firstIndex), self.m_firstIndex);
		s.doValue("m_indexCount", offsetof(MeshBinarySubMesh, m_indexCount), self.m_indexCount);
		s.doValue("m_aabbMin", offsetof(MeshBinarySubMesh, m_aabbMin), self.m_aabbMin);
		s.doValue("m_aabbMax", offsetof(MeshBinarySubMesh, m_aabbMax), self.m_aabbMax);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, MeshBinarySubMesh&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const MeshBinarySubMesh&>(serializer, *this);
	}
};

/// The 1st things that appears in a mesh binary. @note The index and vertex buffers are aligned to 16 bytes.
class MeshBinaryHeader
{
public:
	Array<U8, 8> m_magic;
	MeshBinaryFlag m_flags;
	Array<MeshBinaryVertexBuffer, U32(VertexAttributeLocation::COUNT)> m_vertexBuffers;
	U32 m_vertexBufferCount;
	Array<MeshBinaryVertexAttribute, U32(VertexAttributeLocation::COUNT)> m_vertexAttributes;
	IndexType m_indexType;
	Array<U8, 3> m_padding;
	U32 m_totalIndexCount;
	U32 m_totalVertexCount;
	U32 m_subMeshCount;
	Vec3 m_aabbMin; ///< Bounding box min.
	Vec3 m_aabbMax; ///< Bounding box max.

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_magic", offsetof(MeshBinaryHeader, m_magic), &self.m_magic[0], self.m_magic.getSize());
		s.doValue("m_flags", offsetof(MeshBinaryHeader, m_flags), self.m_flags);
		s.doArray("m_vertexBuffers", offsetof(MeshBinaryHeader, m_vertexBuffers), &self.m_vertexBuffers[0],
				  self.m_vertexBuffers.getSize());
		s.doValue("m_vertexBufferCount", offsetof(MeshBinaryHeader, m_vertexBufferCount), self.m_vertexBufferCount);
		s.doArray("m_vertexAttributes", offsetof(MeshBinaryHeader, m_vertexAttributes), &self.m_vertexAttributes[0],
				  self.m_vertexAttributes.getSize());
		s.doValue("m_indexType", offsetof(MeshBinaryHeader, m_indexType), self.m_indexType);
		s.doArray("m_padding", offsetof(MeshBinaryHeader, m_padding), &self.m_padding[0], self.m_padding.getSize());
		s.doValue("m_totalIndexCount", offsetof(MeshBinaryHeader, m_totalIndexCount), self.m_totalIndexCount);
		s.doValue("m_totalVertexCount", offsetof(MeshBinaryHeader, m_totalVertexCount), self.m_totalVertexCount);
		s.doValue("m_subMeshCount", offsetof(MeshBinaryHeader, m_subMeshCount), self.m_subMeshCount);
		s.doValue("m_aabbMin", offsetof(MeshBinaryHeader, m_aabbMin), self.m_aabbMin);
		s.doValue("m_aabbMax", offsetof(MeshBinaryHeader, m_aabbMax), self.m_aabbMax);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, MeshBinaryHeader&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const MeshBinaryHeader&>(serializer, *this);
	}
};

/// @}

} // end namespace anki

// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Math.h>
#include <AnKi/Gr/Common.h>

namespace anki {

/// @addtogroup resource
/// @{

inline constexpr const char* kMeshMagic = "ANKIMES6";

constexpr U32 kMeshBinaryBufferAlignment = 16;

enum class MeshBinaryFlag : U32
{
	kNone = 0,
	kQuad = 1 << 0,
	kConvex = 1 << 1,

	kAll = kQuad | kConvex,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(MeshBinaryFlag)

/// Vertex buffer info.
class MeshBinaryVertexBuffer
{
public:
	/// The size of the vertex. It's zero if the buffer is not present.
	U32 m_vertexStride;

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
	U32 m_bufferIndex;

	/// If the format is kNone then the attribute is not present.
	Format m_format;

	U32 m_relativeOffset;
	F32 m_scale;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_bufferIndex", offsetof(MeshBinaryVertexAttribute, m_bufferIndex), self.m_bufferIndex);
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
	Array<U32, kMaxLodCount> m_firstIndices;
	Array<U32, kMaxLodCount> m_indexCounts;

	/// Bounding box min.
	Vec3 m_aabbMin;

	/// Bounding box max.
	Vec3 m_aabbMax;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_firstIndices", offsetof(MeshBinarySubMesh, m_firstIndices), &self.m_firstIndices[0],
				  self.m_firstIndices.getSize());
		s.doArray("m_indexCounts", offsetof(MeshBinarySubMesh, m_indexCounts), &self.m_indexCounts[0],
				  self.m_indexCounts.getSize());
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

/// The 1st things that appears in a mesh binary.
class MeshBinaryHeader
{
public:
	Array<U8, 8> m_magic;
	MeshBinaryFlag m_flags;
	Array<MeshBinaryVertexBuffer, kMaxVertexAttributes> m_vertexBuffers;
	Array<MeshBinaryVertexAttribute, kMaxVertexAttributes> m_vertexAttributes;
	IndexType m_indexType;
	Array<U8, 3> m_padding;
	Array<U32, kMaxLodCount> m_totalIndexCounts;
	Array<U32, kMaxLodCount> m_totalVertexCounts;
	U32 m_subMeshCount;
	U32 m_lodCount;

	/// Bounding box min.
	Vec3 m_aabbMin;

	/// Bounding box max.
	Vec3 m_aabbMax;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_magic", offsetof(MeshBinaryHeader, m_magic), &self.m_magic[0], self.m_magic.getSize());
		s.doValue("m_flags", offsetof(MeshBinaryHeader, m_flags), self.m_flags);
		s.doArray("m_vertexBuffers", offsetof(MeshBinaryHeader, m_vertexBuffers), &self.m_vertexBuffers[0],
				  self.m_vertexBuffers.getSize());
		s.doArray("m_vertexAttributes", offsetof(MeshBinaryHeader, m_vertexAttributes), &self.m_vertexAttributes[0],
				  self.m_vertexAttributes.getSize());
		s.doValue("m_indexType", offsetof(MeshBinaryHeader, m_indexType), self.m_indexType);
		s.doArray("m_padding", offsetof(MeshBinaryHeader, m_padding), &self.m_padding[0], self.m_padding.getSize());
		s.doArray("m_totalIndexCounts", offsetof(MeshBinaryHeader, m_totalIndexCounts), &self.m_totalIndexCounts[0],
				  self.m_totalIndexCounts.getSize());
		s.doArray("m_totalVertexCounts", offsetof(MeshBinaryHeader, m_totalVertexCounts), &self.m_totalVertexCounts[0],
				  self.m_totalVertexCounts.getSize());
		s.doValue("m_subMeshCount", offsetof(MeshBinaryHeader, m_subMeshCount), self.m_subMeshCount);
		s.doValue("m_lodCount", offsetof(MeshBinaryHeader, m_lodCount), self.m_lodCount);
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

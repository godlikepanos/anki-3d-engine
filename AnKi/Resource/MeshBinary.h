// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Math.h>
#include <AnKi/Gr/Common.h>

namespace anki {

inline constexpr const char* kMeshMagic = "ANKIMES8";

enum class MeshBinaryFlag : U32
{
	kNone = 0,
	kConvex = 1 << 0,

	kAll = kConvex,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(MeshBinaryFlag)

// Vertex buffer info.
class MeshBinaryVertexBuffer
{
public:
	// The size of the vertex. It's zero if the buffer is not present.
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

// Vertex attribute.
class MeshBinaryVertexAttribute
{
public:
	U32 m_bufferIndex;

	// If the format is kNone then the attribute is not present.
	Format m_format;

	U32 m_relativeOffset;

	// Attribute is compressed and needs to be scaled.
	Array<F32, 4> m_scale;

	// Attribute is compressed and needs to be translated.
	Array<F32, 4> m_translation;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_bufferIndex", offsetof(MeshBinaryVertexAttribute, m_bufferIndex), self.m_bufferIndex);
		s.doValue("m_format", offsetof(MeshBinaryVertexAttribute, m_format), self.m_format);
		s.doValue("m_relativeOffset", offsetof(MeshBinaryVertexAttribute, m_relativeOffset), self.m_relativeOffset);
		s.doArray("m_scale", offsetof(MeshBinaryVertexAttribute, m_scale), &self.m_scale[0], self.m_scale.getSize());
		s.doArray("m_translation", offsetof(MeshBinaryVertexAttribute, m_translation), &self.m_translation[0], self.m_translation.getSize());
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

// Bounding volume info.
class MeshBinaryBoundingVolume
{
public:
	// Bounding box min.
	Vec3 m_aabbMin;

	// Bounding box max.
	Vec3 m_aabbMax;

	// Bounding sphere center.
	Vec3 m_sphereCenter;

	// Bounding sphere radius.
	F32 m_sphereRadius;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_aabbMin", offsetof(MeshBinaryBoundingVolume, m_aabbMin), self.m_aabbMin);
		s.doValue("m_aabbMax", offsetof(MeshBinaryBoundingVolume, m_aabbMax), self.m_aabbMax);
		s.doValue("m_sphereCenter", offsetof(MeshBinaryBoundingVolume, m_sphereCenter), self.m_sphereCenter);
		s.doValue("m_sphereRadius", offsetof(MeshBinaryBoundingVolume, m_sphereRadius), self.m_sphereRadius);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, MeshBinaryBoundingVolume&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const MeshBinaryBoundingVolume&>(serializer, *this);
	}
};

// MeshBinarySubMeshLod class.
class MeshBinarySubMeshLod
{
public:
	// Offset in sizeof(indexType) to the global indices buffer of the LOD this belongs to.
	U32 m_firstIndex;

	U32 m_indexCount;
	U32 m_firstMeshlet;
	U32 m_meshletCount;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_firstIndex", offsetof(MeshBinarySubMeshLod, m_firstIndex), self.m_firstIndex);
		s.doValue("m_indexCount", offsetof(MeshBinarySubMeshLod, m_indexCount), self.m_indexCount);
		s.doValue("m_firstMeshlet", offsetof(MeshBinarySubMeshLod, m_firstMeshlet), self.m_firstMeshlet);
		s.doValue("m_meshletCount", offsetof(MeshBinarySubMeshLod, m_meshletCount), self.m_meshletCount);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, MeshBinarySubMeshLod&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const MeshBinarySubMeshLod&>(serializer, *this);
	}
};

// The 1st thing that appears in a mesh binary.
class MeshBinaryHeader
{
public:
	Array<U8, 8> m_magic;
	MeshBinaryFlag m_flags;
	Array<MeshBinaryVertexBuffer, U32(VertexAttributeSemantic::kCount)> m_vertexBuffers;
	Array<MeshBinaryVertexAttribute, U32(VertexAttributeSemantic::kCount)> m_vertexAttributes;
	IndexType m_indexType;
	Array<U8, 3> m_padding;

	// The format of the 3 indices that make a primitive.
	Format m_meshletPrimitiveFormat;

	Array<U32, kMaxLodCount> m_indexCounts;
	Array<U32, kMaxLodCount> m_vertexCounts;
	Array<U32, kMaxLodCount> m_meshletPrimitiveCounts;
	Array<U32, kMaxLodCount> m_meshletCounts;
	U32 m_subMeshCount;
	U32 m_lodCount;
	U32 m_maxPrimitivesPerMeshlet;
	U32 m_maxVerticesPerMeshlet;
	MeshBinaryBoundingVolume m_boundingVolume;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_magic", offsetof(MeshBinaryHeader, m_magic), &self.m_magic[0], self.m_magic.getSize());
		s.doValue("m_flags", offsetof(MeshBinaryHeader, m_flags), self.m_flags);
		s.doArray("m_vertexBuffers", offsetof(MeshBinaryHeader, m_vertexBuffers), &self.m_vertexBuffers[0], self.m_vertexBuffers.getSize());
		s.doArray("m_vertexAttributes", offsetof(MeshBinaryHeader, m_vertexAttributes), &self.m_vertexAttributes[0],
				  self.m_vertexAttributes.getSize());
		s.doValue("m_indexType", offsetof(MeshBinaryHeader, m_indexType), self.m_indexType);
		s.doArray("m_padding", offsetof(MeshBinaryHeader, m_padding), &self.m_padding[0], self.m_padding.getSize());
		s.doValue("m_meshletPrimitiveFormat", offsetof(MeshBinaryHeader, m_meshletPrimitiveFormat), self.m_meshletPrimitiveFormat);
		s.doArray("m_indexCounts", offsetof(MeshBinaryHeader, m_indexCounts), &self.m_indexCounts[0], self.m_indexCounts.getSize());
		s.doArray("m_vertexCounts", offsetof(MeshBinaryHeader, m_vertexCounts), &self.m_vertexCounts[0], self.m_vertexCounts.getSize());
		s.doArray("m_meshletPrimitiveCounts", offsetof(MeshBinaryHeader, m_meshletPrimitiveCounts), &self.m_meshletPrimitiveCounts[0],
				  self.m_meshletPrimitiveCounts.getSize());
		s.doArray("m_meshletCounts", offsetof(MeshBinaryHeader, m_meshletCounts), &self.m_meshletCounts[0], self.m_meshletCounts.getSize());
		s.doValue("m_subMeshCount", offsetof(MeshBinaryHeader, m_subMeshCount), self.m_subMeshCount);
		s.doValue("m_lodCount", offsetof(MeshBinaryHeader, m_lodCount), self.m_lodCount);
		s.doValue("m_maxPrimitivesPerMeshlet", offsetof(MeshBinaryHeader, m_maxPrimitivesPerMeshlet), self.m_maxPrimitivesPerMeshlet);
		s.doValue("m_maxVerticesPerMeshlet", offsetof(MeshBinaryHeader, m_maxVerticesPerMeshlet), self.m_maxVerticesPerMeshlet);
		s.doValue("m_boundingVolume", offsetof(MeshBinaryHeader, m_boundingVolume), self.m_boundingVolume);
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

// The 2nd thing that appears in a mesh binary.
class MeshBinarySubMesh
{
public:
	Array<MeshBinarySubMeshLod, kMaxLodCount> m_lods;
	MeshBinaryBoundingVolume m_boundingVolume;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_lods", offsetof(MeshBinarySubMesh, m_lods), &self.m_lods[0], self.m_lods.getSize());
		s.doValue("m_boundingVolume", offsetof(MeshBinarySubMesh, m_boundingVolume), self.m_boundingVolume);
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

// The 3rd thing that appears in a mesh binary.
class MeshBinaryMeshlet
{
public:
	// Index of the 1st primitive.
	U32 m_firstPrimitive;

	U32 m_primitiveCount;

	// Index of the first vertex.
	U32 m_firstVertex;

	U32 m_vertexCount;
	MeshBinaryBoundingVolume m_boundingVolume;
	Vec3 m_coneApex;
	Vec3 m_coneDirection;
	F32 m_coneAngle;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_firstPrimitive", offsetof(MeshBinaryMeshlet, m_firstPrimitive), self.m_firstPrimitive);
		s.doValue("m_primitiveCount", offsetof(MeshBinaryMeshlet, m_primitiveCount), self.m_primitiveCount);
		s.doValue("m_firstVertex", offsetof(MeshBinaryMeshlet, m_firstVertex), self.m_firstVertex);
		s.doValue("m_vertexCount", offsetof(MeshBinaryMeshlet, m_vertexCount), self.m_vertexCount);
		s.doValue("m_boundingVolume", offsetof(MeshBinaryMeshlet, m_boundingVolume), self.m_boundingVolume);
		s.doValue("m_coneApex", offsetof(MeshBinaryMeshlet, m_coneApex), self.m_coneApex);
		s.doValue("m_coneDirection", offsetof(MeshBinaryMeshlet, m_coneDirection), self.m_coneDirection);
		s.doValue("m_coneAngle", offsetof(MeshBinaryMeshlet, m_coneAngle), self.m_coneAngle);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, MeshBinaryMeshlet&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const MeshBinaryMeshlet&>(serializer, *this);
	}
};

} // end namespace anki

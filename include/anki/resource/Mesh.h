// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_MESH_H
#define ANKI_RESOURCE_MESH_H

#include "anki/resource/Common.h"
#include "anki/Math.h"
#include "anki/Gr.h"
#include "anki/collision/Obb.h"

namespace anki {

class MeshLoader;

/// Vertex attributes. This should match the shaders
enum class VertexAttribute: U8
{
	POSITION,
	NORMAL,
	TANGENT,
	TEXTURE_COORD,
	TEXTURE_COORD_1,
	BONE_IDS,
	BONE_WEIGHTS,
	INDICES,
	COUNT
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexAttribute, inline)

/// Mesh Resource. It contains the geometry packed in GPU buffers.
class Mesh
{
public:
	/// Default constructor
	Mesh(ResourceAllocator<U8>& alloc);

	~Mesh();

	U32 getTextureChannelsCount() const
	{
		return m_texChannelsCount;
	}

	Bool hasWeights() const
	{
		return m_weights;
	}

	/// Used only to clone the VBO
	U32 getVerticesCount() const
	{
		return m_vertsCount;
	}

	U32 getIndicesCount() const
	{
		return m_indicesCount;
	}

	const Obb& getBoundingShape() const
	{
		return m_obb;
	}

	/// Get indices count and offset of submesh
	U32 getIndicesCountSub(U subMeshId, U32& offset) const
	{
		const SubMesh& sm = m_subMeshes[subMeshId];
		offset = sm.m_indicesOffset;
		return sm.m_indicesCount;
	}

	const Obb& getBoundingShapeSub(U subMeshId) const
	{
		return m_subMeshes[subMeshId].m_obb;
	}

	/// If returns zero then the mesh is a single uniform mesh
	U32 getSubMeshesCount() const
	{
		return m_subMeshes.getSize();
	}

	/// Get info on how to attach a GL buffer to the state
	void getBufferInfo(
		const VertexAttribute attrib, BufferHandle& buffer,
		U32& size, GLenum& type, U32& stride, U32& offset,
		Bool& normalized) const;

	/// Helper function for correct loading
	Bool isCompatible(const Mesh& other) const;

	/// Load from a .mesh file
	ANKI_USE_RESULT Error load(
		const CString& filename, ResourceInitializer& init);

protected:
	/// Per sub mesh data
	struct SubMesh
	{
		U32 m_indicesCount;
		U32 m_indicesOffset;
		Obb m_obb;
	};

	ResourceAllocator<U8> m_alloc;
	ResourceDArray<SubMesh> m_subMeshes;
	U32 m_indicesCount;
	U32 m_vertsCount;
	Obb m_obb;
	U8 m_texChannelsCount;
	Bool8 m_weights;

	BufferHandle m_vertBuff;
	BufferHandle m_indicesBuff;

	/// Create the VBOs using the mesh data
	ANKI_USE_RESULT Error createBuffers(
		const MeshLoader& loader, ResourceInitializer& init);
};

// TODO Remove that
/// A mesh that behaves as a mesh and as a collection of separate meshes.
class BucketMesh: public Mesh
{
public:
	/// Default constructor.
	BucketMesh(ResourceAllocator<U8>& alloc)
	:	Mesh(alloc)
	{}

	~BucketMesh()
	{}

	/// Load from a .mmesh file
	ANKI_USE_RESULT Error load(
		const CString& filename, ResourceInitializer& init);
};

} // end namespace anki

#endif

// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/Math.h>
#include <anki/Gr.h>
#include <anki/collision/Obb.h>

namespace anki
{

// Forward
class MeshLoader;

/// @addtogroup resource
/// @{

/// Mesh Resource. It contains the geometry packed in GPU buffers.
class Mesh : public ResourceObject
{
public:
	/// Default constructor
	Mesh(ResourceManager* manager);

	~Mesh();

	U32 getTextureChannelsCount() const
	{
		return m_texChannelsCount;
	}

	Bool hasBoneWeights() const
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

	BufferPtr getVertexBuffer() const
	{
		return m_vertBuff;
	}

	BufferPtr getIndexBuffer() const
	{
		return m_indicesBuff;
	}

	/// Helper function for correct loading
	Bool isCompatible(const Mesh& other) const;

	/// Load from a mesh file
	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

protected:
	/// Per sub mesh data
	struct SubMesh
	{
		U32 m_indicesCount;
		U32 m_indicesOffset;
		Obb m_obb;
	};

	DynamicArray<SubMesh> m_subMeshes;
	U32 m_indicesCount;
	U32 m_vertsCount;
	Obb m_obb;
	U8 m_texChannelsCount;
	Bool8 m_weights;

	BufferPtr m_vertBuff;
	BufferPtr m_indicesBuff;
};
/// @}

} // end namespace anki

// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
class MeshResource : public ResourceObject
{
public:
	/// Default constructor
	MeshResource(ResourceManager* manager);

	~MeshResource();

	/// Helper function for correct loading
	Bool isCompatible(const MeshResource& other) const;

	/// Load from a mesh file
	ANKI_USE_RESULT Error load(const ResourceFilename& filename, Bool async);

	const Obb& getBoundingShape() const
	{
		return m_obb;
	}

	/// Get submesh info.
	void getSubMeshInfo(U subMeshId, U32& indexOffset, U32& indexCount, Obb* obb) const
	{
		const SubMesh& sm = m_subMeshes[subMeshId];
		indexOffset = sm.m_indicesOffset;
		indexCount = sm.m_indicesCount;
		if(obb)
		{
			*obb = sm.m_obb;
		}
	}

	/// If returns zero then the mesh is a single uniform mesh
	U32 getSubMeshCount() const
	{
		return m_subMeshes.getSize();
	}

	void getIndexBufferInfo(BufferPtr& buff, U32& buffOffset, U32& indexCount, Format& indexFormat) const
	{
		buff = m_indexBuff;
		buffOffset = 0;
		indexCount = m_indexCount;
		indexFormat = m_indexFormat;
	}

	U32 getVertexBufferCount() const
	{
		return m_vertBuffCount;
	}

	void getVertexBufferInfo(U32 buffIdx, BufferPtr& buff, U32& offset, U32& stride) const
	{
		buff = m_vertBuff;
		// TODO
	}

	void getVerteAttributeInfo(VertexAttributeLocation attrib, U32& bufferIdx, Format& format, U32& relativeOffset)
	{
		// TODO
	}

	U32 getTextureChannelsCount() const
	{
		return m_texChannelsCount;
	}

	Bool hasBoneWeights() const
	{
		return m_weights;
	}

protected:
	class LoadTask;
	class LoadContext;

	/// Per sub mesh data
	struct SubMesh
	{
		U32 m_indicesCount;
		U32 m_indicesOffset;
		Obb m_obb;
	};
	DynamicArray<SubMesh> m_subMeshes;

	// Index stuff
	U32 m_indexCount = 0;
	BufferPtr m_indexBuff;
	Format m_indexFormat = Format::NONE;

	// Vertex stuff
	U32 m_vertCount = 0;
	BufferPtr m_vertBuff;
	U8 m_vertBuffCount = 0;
	U8 m_texChannelsCount = 0;

	// Other
	Obb m_obb;
	Bool8 m_weights = false;

	static ANKI_USE_RESULT Error load(LoadContext& ctx);
};
/// @}

} // end namespace anki

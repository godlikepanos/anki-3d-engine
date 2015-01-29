// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_MESH_LOADER_H
#define ANKI_RESOURCE_MESH_LOADER_H

#include "anki/resource/Common.h"
#include "anki/Math.h"

namespace anki {

/// Mesh data. This class loads the mesh file and the Mesh class loads it to
/// the GPU.
class MeshLoader
{
public:
	/// Type of the components.
	enum class ComponentFormat: U32
	{
		NONE,

		R8,
		R8G8,
		R8G8B8,
		R8G8B8A8,

		R16,
		R16G16,
		R16G16B16,
		R16G16B16A16,

		R32,
		R32G32,
		R32G32B32,
		R32G32B32A32,

		R10G10B10A2,

		COUNT
	};

	enum class FormatTransform: U32
	{
		NONE,

		UNORM,
		SNORM,
		UINT,
		SINT,
		FLOAT,

		COUNT
	};

	struct Format
	{
		ComponentFormat m_components = ComponentFormat::NONE;
		FormatTransform m_transform = FormatTransform::NONE;
	};

	struct Header
	{
		Array<U8, 8> m_magic; ///< Magic word.
		U32 m_flags;
		U32 m_flags2;
		Format m_positionsFormat;
		Format m_normalsFormat;
		Format m_tangentsFormat;
		Format m_colorsFormat; ///< Vertex color.
		Format m_uvsFormat;
		Format m_boneWeightsFormat;
		Format m_boneIndicesFormat;
		Format m_indicesFormat; ///< Vertex indices.

		U32 m_totalIndicesCount;
		U32 m_totalVerticesCount;
		/// Number of UV sets. Eg one for normal diffuse and another for 
		/// lightmaps.
		U32 m_uvsChannelCount;
		U32 m_subMeshCount;

		U8 m_padding[32];
	};

	static_assert(sizeof(Header) == 128, "Check size of struct");

	struct SubMesh
	{
		U32 m_firstIndex = 0;
		U32 m_indicesCount = 0;
	};

	~MeshLoader();

	ANKI_USE_RESULT Error load(
		TempResourceAllocator<U8> alloc,
		const CString& filename);

	const Header& getHeader() const
	{
		ANKI_ASSERT(isLoaded());
		return m_header;
	}

	const U8* getVertexData() const
	{
		ANKI_ASSERT(isLoaded());
		return &m_verts[0];
	}

	PtrSize getVertexDataSize() const
	{
		ANKI_ASSERT(isLoaded());
		return m_verts.getSizeInBytes();
	}

	PtrSize getVertexSize() const
	{
		ANKI_ASSERT(isLoaded());
		return m_vertSize;
	}

	const U8* getIndexData() const
	{
		ANKI_ASSERT(isLoaded());
		return &m_indices[0];
	}

	PtrSize getIndexDataSize() const
	{
		ANKI_ASSERT(isLoaded());
		return m_indices.getSizeInBytes();
	}

	Bool hasBoneInfo() const
	{
		ANKI_ASSERT(isLoaded());
		return 
			m_header.m_boneWeightsFormat.m_components != ComponentFormat::NONE;
	}

private:
	template<typename T>
	using MDArray = DArray<T>;

	TempResourceAllocator<U8> m_alloc;
	Header m_header;

	MDArray<U8> m_verts;
	MDArray<U8> m_indices;
	MDArray<SubMesh> m_subMeshes;
	U8 m_vertSize = 0;

	Bool isLoaded() const
	{
		return m_verts.getSize() > 0;
	}

	ANKI_USE_RESULT Error loadInternal(const CString& filename);

	static ANKI_USE_RESULT Error checkFormat(
		const Format& fmt, 
		const CString& attrib,
		Bool cannotBeEmpty);
};

} // end namespace anki

#endif


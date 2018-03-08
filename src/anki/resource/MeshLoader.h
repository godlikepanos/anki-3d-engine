// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>
#include <anki/Math.h>
#include <anki/util/Enum.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// Information to decode mesh binary files.
class MeshBinaryFile
{
public:
	static constexpr const char* MAGIC = "ANKIMES4";

	enum class VertexAttrubuteType : U32
	{
		POSITION,
		NORMAL,
		TANGENT,
		COLOR,
		UV,
		UV2,
		BONE_WEIGHTS,
		BONE_INDICES,

		COUNT
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexAttrubuteType, friend)

	enum class Flag : U32
	{
		NONE = 0,
		QUAD = 1 << 0,
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(Flag, friend)

	struct VertexBuffer
	{
		U32 m_vertexStride;
	};

	struct VertexAttribute
	{
		U32 m_bufferBinding;
		Format m_format;
		U32 m_relativeOffset;
		F32 m_scale;
	};

	struct SubMesh
	{
		U32 m_firstIndex;
		U32 m_indexCount;
	};

	struct Header
	{
		char m_magic[8]; ///< Magic word.
		Flag m_flags;

		Array<VertexBuffer, U32(VertexAttrubuteType::COUNT)> m_vertexBuffers;

		Array<VertexAttribute, U32(VertexAttrubuteType::COUNT)> m_vertexAttributes;

		Format m_indicesFormat; ///< Can be R32_UI or R16_UI.

		U32 m_totalIndexCount;
		U32 m_totalVertexCount;
		U32 m_subMeshCount;
	};
};

/// Mesh data. This class loads the mesh file and the Mesh class loads it to the CPU.
class MeshLoader
{
public:
	/// Type of the components.
	enum class ComponentFormat : U32
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

	enum class FormatTransform : U32
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

	enum class Flag : U32
	{
		NONE = 0,
		QUADS = 1 << 0
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(Flag, friend);

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
		/// Number of UV sets. Eg one for normal diffuse and another for lightmaps.
		U32 m_uvsChannelCount;
		U32 m_subMeshCount;

		U8 m_padding[32];
	};

	static_assert(sizeof(Header) == 128, "Check size of struct");

	class SubMesh
	{
	public:
		U32 m_firstIndex = 0;
		U32 m_indicesCount = 0;
	};

	MeshLoader(ResourceManager* manager);

	MeshLoader(ResourceManager* manager, GenericMemoryPoolAllocator<U8> alloc)
		: m_manager(manager)
		, m_alloc(alloc)
	{
	}

	~MeshLoader();

	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

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
		return m_header.m_boneWeightsFormat.m_components != ComponentFormat::NONE;
	}

private:
	template<typename T>
	using MDynamicArray = DynamicArray<T>;

	ResourceManager* m_manager;
	GenericMemoryPoolAllocator<U8> m_alloc;
	Header m_header;

	MDynamicArray<U8> m_verts;
	MDynamicArray<U8> m_indices;
	MDynamicArray<SubMesh> m_subMeshes;
	U8 m_vertSize = 0;

	Bool isLoaded() const
	{
		return m_verts.getSize() > 0;
	}

	static ANKI_USE_RESULT Error checkFormat(const Format& fmt, const CString& attrib, Bool cannotBeEmpty);
};
/// @}

} // end namespace anki

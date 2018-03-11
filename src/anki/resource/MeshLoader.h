// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>
#include <anki/resource/ResourceFilesystem.h>
#include <anki/Math.h>
#include <anki/util/Enum.h>
#include <anki/util/WeakArray.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// Information to decode mesh binary files.
class MeshBinaryFile
{
public:
	static constexpr const char* MAGIC = "ANKIMES4";

	enum class Flag : U32
	{
		NONE = 0,
		QUAD = 1 << 0,

		ALL = QUAD,
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

		Array<VertexBuffer, U32(VertexAttributeLocation::COUNT)> m_vertexBuffers;

		Array<VertexAttribute, U32(VertexAttributeLocation::COUNT)> m_vertexAttributes;

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
	MeshLoader(ResourceManager* manager);

	MeshLoader(ResourceManager* manager, GenericMemoryPoolAllocator<U8> alloc)
		: m_manager(manager)
		, m_alloc(alloc)
	{
	}

	~MeshLoader();

	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

	ANKI_USE_RESULT Error storeIndexBuffer(void* ptr, PtrSize size);

	ANKI_USE_RESULT Error storeVertexBuffer(U32 bufferIdx, void* ptr, PtrSize size);

	const MeshBinaryFile::Header& getHeader() const
	{
		ANKI_ASSERT(isLoaded());
		return m_header;
	}

	Bool hasBoneInfo() const
	{
		ANKI_ASSERT(isLoaded());
		return m_header.m_vertexAttributes[VertexAttributeLocation::BONE_INDICES].m_format != Format::NONE;
	}

private:
	ResourceManager* m_manager;
	GenericMemoryPoolAllocator<U8> m_alloc;

	ResourceFilePtr m_file;

	MeshBinaryFile::Header m_header;

	DynamicArray<MeshBinaryFile::SubMesh> m_subMeshes;
	U32 m_vertBufferCount = 0;

	U32 m_loadedChunk = 0; ///< Because the store methods need to be called in sequence.

	Bool isLoaded() const
	{
		return m_file.get() != nullptr;
	}

	U32 getIndexBufferSize() const
	{
		return m_header.m_totalIndexCount * ((m_header.m_indicesFormat == Format::R16_UINT) ? 2 : 4);
	}

	ANKI_USE_RESULT Error checkHeader() const;
	ANKI_USE_RESULT Error checkFormat(VertexAttributeLocation type, ConstWeakArray<Format> supportedFormats) const;
};
/// @}

} // end namespace anki

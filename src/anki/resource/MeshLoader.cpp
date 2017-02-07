// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/MeshLoader.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/ResourceFilesystem.h>

namespace anki
{

MeshLoader::MeshLoader(ResourceManager* manager)
	: MeshLoader(manager, manager->getTempAllocator())
{
}

MeshLoader::~MeshLoader()
{
	// WARNING: Watch the order of deallocation. Reverse of the deallocation to have successful cleanups
	m_verts.destroy(m_alloc);
	m_indices.destroy(m_alloc);
	m_subMeshes.destroy(m_alloc);
}

Error MeshLoader::load(const ResourceFilename& filename)
{
	auto& alloc = m_alloc;

	// Load header
	ResourceFilePtr file;
	ANKI_CHECK(m_manager->getFilesystem().openFile(filename, file));
	ANKI_CHECK(file->read(&m_header, sizeof(m_header)));

	//
	// Check header
	//
	if(memcmp(&m_header.m_magic[0], "ANKIMES3", 8) != 0)
	{
		ANKI_RESOURCE_LOGE("Wrong magic word");
		return ErrorCode::USER_DATA;
	}

	if(checkFormat(m_header.m_positionsFormat, "positions", true)
		|| checkFormat(m_header.m_normalsFormat, "normals", true)
		|| checkFormat(m_header.m_tangentsFormat, "tangents", true)
		|| checkFormat(m_header.m_colorsFormat, "colors", false)
		|| checkFormat(m_header.m_uvsFormat, "UVs", true)
		|| checkFormat(m_header.m_boneWeightsFormat, "bone weights", false)
		|| checkFormat(m_header.m_boneIndicesFormat, "bone ids", false)
		|| checkFormat(m_header.m_indicesFormat, "indices format", true))
	{
		return ErrorCode::USER_DATA;
	}

	// Check positions
	if(m_header.m_positionsFormat.m_components != ComponentFormat::R32G32B32
		|| m_header.m_positionsFormat.m_transform != FormatTransform::FLOAT)
	{
		ANKI_RESOURCE_LOGE("Incorrect/unsupported positions format");
		return ErrorCode::USER_DATA;
	}

	// Check normals
	if(m_header.m_normalsFormat.m_components != ComponentFormat::R10G10B10A2
		|| m_header.m_normalsFormat.m_transform != FormatTransform::SNORM)
	{
		ANKI_RESOURCE_LOGE("Incorrect/unsupported normals format");
		return ErrorCode::USER_DATA;
	}

	// Check tangents
	if(m_header.m_tangentsFormat.m_components != ComponentFormat::R10G10B10A2
		|| m_header.m_tangentsFormat.m_transform != FormatTransform::SNORM)
	{
		ANKI_RESOURCE_LOGE("Incorrect/unsupported tangents format");
		return ErrorCode::USER_DATA;
	}

	// Check colors
	if(m_header.m_colorsFormat.m_components != ComponentFormat::NONE
		|| m_header.m_colorsFormat.m_transform != FormatTransform::NONE)
	{
		ANKI_RESOURCE_LOGE("Incorrect/unsupported color format");
		return ErrorCode::USER_DATA;
	}

	// Check UVs
	if(m_header.m_uvsFormat.m_components != ComponentFormat::R16G16
		|| m_header.m_uvsFormat.m_transform != FormatTransform::FLOAT)
	{
		ANKI_RESOURCE_LOGE("Incorrect/unsupported UVs format");
		return ErrorCode::USER_DATA;
	}

	Bool hasBoneInfo = false;
	if(m_header.m_boneWeightsFormat.m_components != ComponentFormat::NONE)
	{
		// Has bone info

		hasBoneInfo = true;

		// Bone weights
		if(m_header.m_boneWeightsFormat.m_components != ComponentFormat::R8G8B8A8
			|| m_header.m_boneWeightsFormat.m_transform != FormatTransform::UNORM)
		{
			ANKI_RESOURCE_LOGE("Incorrect/unsupported UVs format");
			return ErrorCode::USER_DATA;
		}

		// Bone indices
		if(m_header.m_boneIndicesFormat.m_components != ComponentFormat::R16G16B16A16
			|| m_header.m_boneIndicesFormat.m_transform != FormatTransform::UINT)
		{
			ANKI_RESOURCE_LOGE("Incorrect/unsupported UVs format");
			return ErrorCode::USER_DATA;
		}
	}
	else
	{
		// No bone info

		// Bone weights
		if(m_header.m_boneWeightsFormat.m_components != ComponentFormat::NONE
			|| m_header.m_boneWeightsFormat.m_transform != FormatTransform::NONE)
		{
			ANKI_RESOURCE_LOGE("Incorrect/unsupported UVs format");
			return ErrorCode::USER_DATA;
		}

		// Bone indices
		if(m_header.m_boneIndicesFormat.m_components != ComponentFormat::NONE
			|| m_header.m_boneIndicesFormat.m_transform != FormatTransform::NONE)
		{
			ANKI_RESOURCE_LOGE("Incorrect/unsupported UVs format");
			return ErrorCode::USER_DATA;
		}
	}

	// Check indices
	U indicesPerFace = ((m_header.m_flags & Flag::QUADS) == Flag::QUADS) ? 4 : 3;
	if(m_header.m_totalIndicesCount < indicesPerFace || (m_header.m_totalIndicesCount % indicesPerFace) != 0
		|| m_header.m_totalIndicesCount > MAX_U16
		|| m_header.m_indicesFormat.m_components != ComponentFormat::R16
		|| m_header.m_indicesFormat.m_transform != FormatTransform::UINT)
	{
		// Only 16bit indices are supported for now
		ANKI_RESOURCE_LOGE("Incorrect/unsuported index info");
		return ErrorCode::USER_DATA;
	}

	// Check other
	if(m_header.m_totalVerticesCount == 0)
	{
		ANKI_RESOURCE_LOGE("Incorrect/unsuported vertex count");
		return ErrorCode::USER_DATA;
	}

	if(m_header.m_uvsChannelCount != 1)
	{
		ANKI_RESOURCE_LOGE("Incorrect/unsuported UVs channel count");
		return ErrorCode::USER_DATA;
	}

	if(m_header.m_subMeshCount == 0)
	{
		ANKI_RESOURCE_LOGE("Incorrect/unsuported submesh count");
		return ErrorCode::USER_DATA;
	}

	//
	// Read submesh info
	//
	m_subMeshes.create(alloc, m_header.m_subMeshCount);
	ANKI_CHECK(file->read(&m_subMeshes[0], m_subMeshes.getSizeInBytes()));

	// Checks
	U idxSum = 0;
	for(U i = 0; i < m_subMeshes.getSize(); i++)
	{
		const SubMesh& sm = m_subMeshes[0];
		if(sm.m_firstIndex != idxSum || sm.m_indicesCount < 3)
		{
			ANKI_RESOURCE_LOGE("Incorrect sub mesh info");
			return ErrorCode::USER_DATA;
		}

		idxSum += sm.m_indicesCount;
	}

	if(idxSum != m_header.m_totalIndicesCount)
	{
		ANKI_RESOURCE_LOGE("Incorrect sub mesh info");
		return ErrorCode::USER_DATA;
	}

	//
	// Read indices
	//
	m_indices.create(alloc, m_header.m_totalIndicesCount * sizeof(U16));
	ANKI_CHECK(file->read(&m_indices[0], m_indices.getSizeInBytes()));

	//
	// Read vertices
	//
	m_vertSize = 3 * sizeof(F32) // pos
		+ 1 * sizeof(U32) // norm
		+ 1 * sizeof(U32) // tang
		+ 2 * sizeof(U16) // uvs
		+ ((hasBoneInfo) ? (4 * sizeof(U8) + 4 * sizeof(U16)) : 0);

	m_verts.create(alloc, m_header.m_totalVerticesCount * m_vertSize);
	ANKI_CHECK(file->read(&m_verts[0], m_verts.getSizeInBytes()));

	return ErrorCode::NONE;
}

Error MeshLoader::checkFormat(const Format& fmt, const CString& attrib, Bool cannotBeEmpty)
{
	if(fmt.m_components >= ComponentFormat::COUNT)
	{
		ANKI_RESOURCE_LOGE("Incorrect component format for %s", &attrib[0]);
		return ErrorCode::USER_DATA;
	}

	if(fmt.m_transform >= FormatTransform::COUNT)
	{
		ANKI_RESOURCE_LOGE("Incorrect format transform for %s", &attrib[0]);
		return ErrorCode::USER_DATA;
	}

	if(cannotBeEmpty)
	{
		if(fmt.m_components == ComponentFormat::NONE)
		{
			ANKI_RESOURCE_LOGE("Format cannot be zero for %s", &attrib[0]);
			return ErrorCode::USER_DATA;
		}

		if(fmt.m_transform == FormatTransform::NONE)
		{
			ANKI_RESOURCE_LOGE("Transform cannot be zero for %s", &attrib[0]);
			return ErrorCode::USER_DATA;
		}
	}

	return ErrorCode::NONE;
}

} // end namespace anki

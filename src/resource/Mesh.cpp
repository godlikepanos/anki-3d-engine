// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Mesh.h"
#include "anki/resource/ResourceManager.h"
#include "anki/resource/MeshLoader2.h"
#include "anki/util/Functions.h"
#include "anki/misc/Xml.h"

namespace anki {

//==============================================================================
// Mesh                                                                        =
//==============================================================================

//==============================================================================
Mesh::Mesh(ResourceAllocator<U8>&)
{}

//==============================================================================
Mesh::~Mesh()
{
	m_subMeshes.destroy(m_alloc);
}

//==============================================================================
Bool Mesh::isCompatible(const Mesh& other) const
{
	return hasWeights() == other.hasWeights() 
		&& getSubMeshesCount() == other.getSubMeshesCount(); 
}

//==============================================================================
Error Mesh::load(const CString& filename, ResourceInitializer& init)
{
	Error err = ErrorCode::NONE;

	MeshLoader2 loader;
	err = loader.load(init.m_tempAlloc, filename);
	if(err) return err;

	const MeshLoader2::Header& header = loader.getHeader();
	m_indicesCount = header.m_totalIndicesCount;

	PtrSize vertexSize = loader.getVertexSize();
	m_obb.setFromPointCloud(loader.getVertexData(), header.m_totalVerticesCount,
		vertexSize, loader.getVertexDataSize());
	ANKI_ASSERT(m_indicesCount > 0);
	ANKI_ASSERT(m_indicesCount % 3 == 0 && "Expecting triangles");

	// Set the non-VBO members
	m_vertsCount = header.m_totalVerticesCount;
	ANKI_ASSERT(m_vertsCount > 0);

	m_texChannelsCount = header.m_uvsChannelCount;
	m_weights = loader.hasBoneInfo();

	err = createBuffers(loader, init);

	return err;
}

//==============================================================================
Error Mesh::createBuffers(const MeshLoader2& loader,
	ResourceInitializer& init)
{
	Error err = ErrorCode::NONE;

	GlDevice& gl = init.m_resources._getGlDevice();
	GlCommandBufferHandle cmdb;
	err = cmdb.create(&gl);
	if(err) return err;

	// Create vertex buffer
	GlClientBufferHandle clientVertBuff;
	err = clientVertBuff.create(cmdb, loader.getVertexDataSize(), nullptr);
	if(err) return err;
	memcpy(clientVertBuff.getBaseAddress(), loader.getVertexData(), 
		loader.getVertexDataSize());

	err = m_vertBuff.create(cmdb, GL_ARRAY_BUFFER, clientVertBuff, 0);
	if(err) return err;

	// Create index buffer
	GlClientBufferHandle clientIndexBuff;
	err = clientIndexBuff.create(cmdb, loader.getIndexDataSize(), nullptr);
	if(err) return err;
	memcpy(clientIndexBuff.getBaseAddress(), loader.getIndexData(),
		loader.getIndexDataSize());

	err = m_indicesBuff.create(
		cmdb, GL_ELEMENT_ARRAY_BUFFER, clientIndexBuff, 0);
	if(err) return err;

	cmdb.finish(); /// XXX

	return err;
}

//==============================================================================
void Mesh::getBufferInfo(const VertexAttribute attrib, 
	GlBufferHandle& v, U32& size, GLenum& type, 
	U32& stride, U32& offset, Bool& normalized) const
{
	stride = sizeof(Vec3) + sizeof(U32) + sizeof(U32) 
		+ m_texChannelsCount * sizeof(HVec2) 
		+ ((m_weights) ? (sizeof(U8) * 4 + sizeof(HVec4)) : 0);

	// Set all to zero
	v = GlBufferHandle();
	size = 0;
	type = GL_NONE;
	offset = 0;
	normalized = false;

	switch(attrib)
	{
	case VertexAttribute::POSITION:
		v = m_vertBuff;
		size = 3;
		type = GL_FLOAT;
		offset = 0;
		break;
	case VertexAttribute::NORMAL:
		v = m_vertBuff;
		size = 4;
		type = GL_INT_2_10_10_10_REV;
		offset = sizeof(Vec3);
		normalized = true;
		break;
	case VertexAttribute::TANGENT:
		v = m_vertBuff;
		size = 4;
		type = GL_INT_2_10_10_10_REV;
		offset = sizeof(Vec3) + sizeof(U32);
		normalized = true;
		break;
	case VertexAttribute::TEXTURE_COORD:
		if(m_texChannelsCount > 0)
		{
			v = m_vertBuff;
			size = 2;
			type = GL_HALF_FLOAT;
			offset = sizeof(Vec3) + sizeof(U32) + sizeof(U32);
		}
		break;
	case VertexAttribute::TEXTURE_COORD_1:
		if(m_texChannelsCount > 1)
		{
			v = m_vertBuff;
			size = 2;
			type = GL_HALF_FLOAT;
			offset = sizeof(Vec3) + sizeof(U32) + sizeof(U32) 
				+ sizeof(HVec2);
		}
		break;
	case VertexAttribute::BONE_WEIGHTS:
		if(m_weights)
		{
			v = m_vertBuff;
			size = 4;
			type = GL_UNSIGNED_BYTE;
			offset = sizeof(Vec3) + sizeof(U32) + sizeof(U32) 
				+ m_texChannelsCount * sizeof(HVec2);
			normalized = true;
		}
		break;
	case VertexAttribute::BONE_IDS:
		if(m_weights)
		{
			v = m_vertBuff;
			size = 4;
			type = GL_HALF_FLOAT;
			offset = sizeof(Vec3) + sizeof(U32) + sizeof(U32) 
				+ m_texChannelsCount * sizeof(HVec2) + sizeof(U8) * 4;
		}
		break;
	case VertexAttribute::INDICES:
		v = m_indicesBuff;
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}
}

//==============================================================================
// BucketMesh                                                                  =
//==============================================================================

//==============================================================================
Error BucketMesh::load(const CString& filename, ResourceInitializer& init)
{
	Error err = ErrorCode::NONE;

#if 0
	m_alloc = init.m_alloc;

	XmlDocument doc;
	ANKI_CHECK(doc.loadFile(filename, init.m_tempAlloc));

	XmlElement rootEl;
	ANKI_CHECK(doc.getChildElement("bucketMesh", rootEl));
	XmlElement meshesEl;
	ANKI_CHECK(rootEl.getChildElement("meshes", meshesEl));
	XmlElement meshEl;
	ANKI_CHECK(meshesEl.getChildElement("mesh", meshEl));

	// Count sub meshes
	U i = 0;
	do
	{
		++i;
		ANKI_CHECK(meshEl.getNextSiblingElement("mesh", meshEl));
	} while(meshEl);

	ANKI_CHECK(m_subMeshes.create(m_alloc, i));

	ANKI_CHECK(meshesEl.getChildElement("mesh", meshEl));

	m_vertsCount = 0;
	m_indicesCount = 0;

	MeshLoader fullLoader(init.m_tempAlloc);
	do
	{
		CString subMeshFilename;
		ANKI_CHECK(meshEl.getText(subMeshFilename));

		// Load the submesh and if not the first load the append the 
		// vertices to the fullMesh
		MeshLoader* loader;
		MeshLoader subLoader(init.m_tempAlloc);
		if(i != 0)
		{
			// Sanity check
			if(i > ANKI_GL_MAX_SUB_DRAWCALLS)
			{
				ANKI_LOGE("Max number of submeshes exceeded");
				return ErrorCode::USER_DATA;
			}

			// Load
			ANKI_CHECK(subLoader.load(subMeshFilename));
			loader = &subLoader;

			// Sanity checks
			if(m_weights != (loader->getWeights().getSize() > 1))
			{
				ANKI_LOGE("All sub meshes should have or not "
					"have vertex weights");
				return ErrorCode::USER_DATA;
			}

			if(m_texChannelsCount 
				!= loader->getTextureChannelsCount())
			{
				ANKI_LOGE("All sub meshes should have the "
					"same number of texture channels");
				return ErrorCode::USER_DATA;
			}

			// Append
			ANKI_CHECK(fullLoader.append(subLoader));
		}
		else
		{
			// Load
			ANKI_CHECK(fullLoader.load(subMeshFilename));
			loader = &fullLoader;

			// Set properties
			m_weights = loader->getWeights().getSize() > 1;
			m_texChannelsCount = loader->getTextureChannelsCount();
		}

		// Push back the new submesh
		SubMesh& submesh = m_subMeshes[i];

		submesh.m_indicesCount = loader->getIndices().getSize();
		submesh.m_indicesOffset = m_indicesCount * sizeof(U16);

		const auto& positions = loader->getPositions();
		submesh.m_obb.setFromPointCloud(&positions[0], positions.getSize(),
			sizeof(Vec3), positions.getSizeInBytes());

		// Set the global numbers
		m_vertsCount += loader->getPositions().getSize();
		m_indicesCount += loader->getIndices().getSize();

		++i;
		// Move to next
		ANKI_CHECK(meshEl.getNextSiblingElement("mesh", meshEl));
	} while(meshEl);

	// Create the bucket mesh
	ANKI_CHECK(createBuffers(fullLoader, init));

	const auto& positions = fullLoader.getPositions();
	m_obb.setFromPointCloud(&positions[0], positions.getSize(),
		sizeof(Vec3), positions.getSizeInBytes());
#endif

	return err;
}

} // end namespace anki

// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Mesh.h"
#include "anki/resource/ResourceManager.h"
#include "anki/resource/MeshLoader.h"
#include "anki/util/Functions.h"
#include "anki/misc/Xml.h"

namespace anki {

//==============================================================================
// Mesh                                                                        =
//==============================================================================

//==============================================================================
Mesh::Mesh(ResourceAllocator<U8>& alloc)
:	m_subMeshes(alloc)
{}

//==============================================================================
Mesh::~Mesh()
{}

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

	MeshLoader loader(init.m_tempAlloc);
	ANKI_CHECK(loader.load(filename))

	m_indicesCount = loader.getIndices().size();

	const auto& positions = loader.getPositions();
	m_obb.setFromPointCloud(&positions[0], positions.size(),
		sizeof(Vec3), positions.getSizeInBytes());
	ANKI_ASSERT(m_indicesCount > 0);
	ANKI_ASSERT(m_indicesCount % 3 == 0 && "Expecting triangles");

	// Set the non-VBO members
	m_vertsCount = loader.getPositions().size();
	ANKI_ASSERT(m_vertsCount > 0);

	m_texChannelsCount = loader.getTextureChannelsCount();
	m_weights = loader.getWeights().size() > 1;

	ANKI_CHECK(createBuffers(loader, init));

	return err;
}

//==============================================================================
U32 Mesh::calcVertexSize() const
{
	U32 a = sizeof(Vec3) + sizeof(HVec3) + sizeof(HVec4) 
		+ m_texChannelsCount * sizeof(HVec2);
	if(m_weights)
	{
		a += sizeof(MeshLoader::VertexWeight);
	}

	alignRoundUp(sizeof(F32), a);
	return a;
}

//==============================================================================
Error Mesh::createBuffers(const MeshLoader& loader,
	ResourceInitializer& init)
{
	ANKI_ASSERT(m_vertsCount == loader.getPositions().size()
		&& m_vertsCount == loader.getNormals().size()
		&& m_vertsCount == loader.getTangents().size());

	Error err = ErrorCode::NONE;

	// Calculate VBO size
	U32 vertexsize = calcVertexSize();
	U32 vbosize = vertexsize * m_vertsCount;

	// Create a temp buffer and populate it
	TempResourceVector<U8> buff(vbosize, 0, init.m_tempAlloc);

	U8* ptra = &buff[0];
	for(U i = 0; i < m_vertsCount; i++)
	{
		U8* ptr = ptra;
		ANKI_ASSERT(ptr + vertexsize <= &buff[0] + vbosize);

		memcpy(ptr, &loader.getPositions()[i], sizeof(Vec3));
		ptr += sizeof(Vec3);

		memcpy(ptr, &loader.getNormals()[i], sizeof(HVec3));
		ptr += sizeof(HVec3);

		memcpy(ptr, &loader.getTangents()[i], sizeof(HVec4));
		ptr += sizeof(HVec4);

		for(U j = 0; j < m_texChannelsCount; j++)
		{
			memcpy(ptr, &loader.getTextureCoordinates(j)[i], sizeof(HVec2));
			ptr += sizeof(HVec2);
		}

		if(m_weights)
		{
			memcpy(ptr, &loader.getWeights()[i], 
				sizeof(MeshLoader::VertexWeight));
			ptr += sizeof(MeshLoader::VertexWeight);
		}

		ptra += vertexsize;
	}

	// Create GL buffers
	GlDevice& gl = init.m_resources._getGlDevice();
	GlCommandBufferHandle cmdb;
	ANKI_CHECK(cmdb.create(&gl));
	
	GlClientBufferHandle clientVertBuff;
	ANKI_CHECK(clientVertBuff.create(cmdb, vbosize, &buff[0]));
	ANKI_CHECK(m_vertBuff.create(cmdb, GL_ARRAY_BUFFER, clientVertBuff, 0));

	GlClientBufferHandle clientIndexBuff;
	ANKI_CHECK(clientIndexBuff.create(
		cmdb, 
		getVectorSizeInBytes(loader.getIndices()), 
		(void*)&loader.getIndices()[0]));
	ANKI_CHECK(m_indicesBuff.create(
		cmdb, GL_ELEMENT_ARRAY_BUFFER, clientIndexBuff, 0));

	cmdb.finish();

	return err;
}

//==============================================================================
void Mesh::getBufferInfo(const VertexAttribute attrib, 
	GlBufferHandle& v, U32& size, GLenum& type, 
	U32& stride, U32& offset) const
{
	stride = calcVertexSize();

	// Set all to zero
	v = GlBufferHandle();
	size = 0;
	type = GL_NONE;
	offset = 0;

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
		size = 3;
		type = GL_HALF_FLOAT;
		offset = sizeof(Vec3);
		break;
	case VertexAttribute::TANGENT:
		v = m_vertBuff;
		size = 4;
		type = GL_HALF_FLOAT;
		offset = sizeof(Vec3) + sizeof(HVec3);
		break;
	case VertexAttribute::TEXTURE_COORD:
		if(m_texChannelsCount > 0)
		{
			v = m_vertBuff;
			size = 2;
			type = GL_HALF_FLOAT;
			offset = sizeof(Vec3) + sizeof(HVec3) + sizeof(HVec4);
		}
		break;
	case VertexAttribute::TEXTURE_COORD_1:
		if(m_texChannelsCount > 1)
		{
			v = m_vertBuff;
			size = 2;
			type = GL_HALF_FLOAT;
			offset = sizeof(Vec3) + sizeof(HVec3) + sizeof(HVec4) 
				+ sizeof(HVec2);
		}
		break;
	case VertexAttribute::BONE_COUNT:
		if(m_weights)
		{
			v = m_vertBuff;
			size = 1;
			type = GL_UNSIGNED_SHORT;
			offset = sizeof(Vec3) + sizeof(HVec3) + sizeof(HVec4) 
				+ m_texChannelsCount * sizeof(HVec2);
		}
		break;
	case VertexAttribute::BONE_IDS:
		if(m_weights)
		{
			v = m_vertBuff;
			size = 4;
			type = GL_UNSIGNED_SHORT;
			offset = sizeof(Vec3) + sizeof(HVec3) + sizeof(HVec4) 
				+ m_texChannelsCount * sizeof(HVec2) + sizeof(U16);
		}
		break;
	case VertexAttribute::BONE_WEIGHTS:
		if(m_weights)
		{
			v = m_vertBuff;
			size = 4;
			type = GL_HALF_FLOAT;
			offset = sizeof(Vec3) + sizeof(HVec3) + sizeof(HVec4) 
				+ m_texChannelsCount * sizeof(HVec2) + sizeof(U16) 
				+ sizeof(U16) * 4;
		}
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

	XmlDocument doc;
	ANKI_CHECK(doc.loadFile(filename, init.m_tempAlloc));

	XmlElement rootEl;
	ANKI_CHECK(doc.getChildElement("bucketMesh", rootEl));
	XmlElement meshesEl;
	ANKI_CHECK(rootEl.getChildElement("meshes", meshesEl));
	XmlElement meshEl;
	ANKI_CHECK(meshesEl.getChildElement("mesh", meshEl));

	m_vertsCount = 0;
	m_subMeshes.reserve(4);
	m_indicesCount = 0;

	MeshLoader fullLoader(init.m_tempAlloc);
	U i = 0;
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
			if(m_weights != (loader->getWeights().size() > 1))
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
			fullLoader.append(subLoader);
		}
		else
		{
			// Load
			ANKI_CHECK(fullLoader.load(subMeshFilename));
			loader = &fullLoader;

			// Set properties
			m_weights = loader->getWeights().size() > 1;
			m_texChannelsCount = loader->getTextureChannelsCount();
		}

		// Push back the new submesh
		SubMesh submesh;

		submesh.m_indicesCount = loader->getIndices().size();
		submesh.m_indicesOffset = m_indicesCount * sizeof(U16);

		const auto& positions = loader->getPositions();
		submesh.m_obb.setFromPointCloud(&positions[0], positions.size(),
			sizeof(Vec3), positions.getSizeInBytes());

		m_subMeshes.push_back(submesh);

		// Set the global numbers
		m_vertsCount += loader->getPositions().size();
		m_indicesCount += loader->getIndices().size();

		// Move to next
		ANKI_CHECK(meshEl.getNextSiblingElement("mesh", meshEl));
		++i;
	} while(meshEl);

	// Create the bucket mesh
	ANKI_CHECK(createBuffers(fullLoader, init));

	const auto& positions = fullLoader.getPositions();
	m_obb.setFromPointCloud(&positions[0], positions.size(),
		sizeof(Vec3), positions.getSizeInBytes());

	return err;
}

} // end namespace anki

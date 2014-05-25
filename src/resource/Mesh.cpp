#include "anki/resource/Mesh.h"
#include "anki/resource/Material.h"
#include "anki/resource/MeshLoader.h"
#include "anki/util/Functions.h"
#include "anki/misc/Xml.h"

namespace anki {

//==============================================================================
// Mesh                                                                        =
//==============================================================================

//==============================================================================
Bool Mesh::isCompatible(const Mesh& other) const
{
	return hasWeights() == other.hasWeights() 
		&& getSubMeshesCount() == other.getSubMeshesCount(); 
}

//==============================================================================
void Mesh::load(const char* filename)
{
	try
	{
		MeshLoader loader(filename);

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

		createBuffers(loader);
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to load mesh") << e;
	}
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
void Mesh::createBuffers(const MeshLoader& loader)
{
	ANKI_ASSERT(m_vertsCount == loader.getPositions().size()
		&& m_vertsCount == loader.getNormals().size()
		&& m_vertsCount == loader.getTangents().size());

	// Calculate VBO size
	U32 vertexsize = calcVertexSize();
	U32 vbosize = vertexsize * m_vertsCount;

	// Create a temp buffer and populate it
	Vector<U8> buff(vbosize, 0);

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
	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);
	
	GlClientBufferHandle clientVertBuff(jobs, vbosize, &buff[0]);
	m_vertBuff = GlBufferHandle(jobs, GL_ARRAY_BUFFER, clientVertBuff, 0);

	GlClientBufferHandle clientIndexBuff(
		jobs, 
		getVectorSizeInBytes(loader.getIndices()), 
		(void*)&loader.getIndices()[0]);
	m_indicesBuff = GlBufferHandle(
		jobs, GL_ELEMENT_ARRAY_BUFFER, clientIndexBuff, 0);

	jobs.finish();
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
void BucketMesh::load(const char* filename)
{
	try
	{
		XmlDocument doc;
		doc.loadFile(filename);

		XmlElement rootEl = doc.getChildElement("bucketMesh");
		XmlElement meshesEl = rootEl.getChildElement("meshes");
		XmlElement meshEl = meshesEl.getChildElement("mesh");

		m_vertsCount = 0;
		m_subMeshes.reserve(4);
		m_indicesCount = 0;

		MeshLoader fullLoader;
		U i = 0;
		do
		{
			std::string subMeshFilename = meshEl.getText();

			// Load the submesh and if not the first load the append the 
			// vertices to the fullMesh
			MeshLoader* loader;
			MeshLoader subLoader;
			if(i != 0)
			{
				// Sanity check
				if(i > ANKI_GL_MAX_SUB_DRAWCALLS)
				{
					throw ANKI_EXCEPTION("Max number of submeshes exceeded");
				}

				// Load
				subLoader.load(subMeshFilename.c_str());
				loader = &subLoader;

				// Sanity checks
				if(m_weights != (loader->getWeights().size() > 1))
				{
					throw ANKI_EXCEPTION("All sub meshes should have or not "
						"have vertex weights");
				}

				if(m_texChannelsCount 
					!= loader->getTextureChannelsCount())
				{
					throw ANKI_EXCEPTION("All sub meshes should have the "
						"same number of texture channels");
				}

				// Append
				fullLoader.append(subLoader);
			}
			else
			{
				// Load
				fullLoader.load(subMeshFilename.c_str());
				loader = &fullLoader;

				// Set properties
				m_weights = loader->getWeights().size() > 1;
				m_texChannelsCount = 
					loader->getTextureChannelsCount();
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
			meshEl = meshEl.getNextSiblingElement("mesh");
			++i;
		} while(meshEl);

		// Create the bucket mesh
		createBuffers(fullLoader);

		const auto& positions = fullLoader.getPositions();
		m_obb.setFromPointCloud(&positions[0], positions.size(),
			sizeof(Vec3), positions.getSizeInBytes());
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to load bucket mesh") << e;
	}
}

} // end namespace anki

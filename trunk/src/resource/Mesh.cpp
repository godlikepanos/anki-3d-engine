#include "anki/resource/Mesh.h"
#include "anki/resource/Material.h"
#include "anki/resource/MeshLoader.h"
#include "anki/util/Functions.h"
#include "anki/misc/Xml.h"

namespace anki {

//==============================================================================
// MeshBase                                                                    =
//==============================================================================

//==============================================================================
Bool MeshBase::isCompatible(const MeshBase& other) const
{
	return hasWeights() == other.hasWeights() 
		&& getSubMeshesCount() == other.getSubMeshesCount(); 
}

//==============================================================================
// Mesh                                                                        =
//==============================================================================

//==============================================================================
void Mesh::load(const char* filename)
{
	try
	{
		MeshLoader loader(filename);

		meshProtected.indicesCount = loader.getIndices().size();
		meshProtected.obb.set(loader.getPositions());
		ANKI_ASSERT(meshProtected.indicesCount > 0);
		ANKI_ASSERT(meshProtected.indicesCount % 3 == 0 
			&& "Expecting triangles");

		// Set the non-VBO members
		meshProtected.vertsCount = loader.getPositions().size();
		ANKI_ASSERT(meshProtected.vertsCount > 0);

		meshProtected.texChannelsCount = loader.getTextureChannelsCount();
		meshProtected.weights = loader.getWeights().size() > 1;

		createVbos(loader);
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Mesh loading failed: " + filename) << e;
	}
}

//==============================================================================
U32 Mesh::calcVertexSize() const
{
	U32 a = sizeof(Vec3) + sizeof(HVec3) + sizeof(HVec4) 
		+ meshProtected.texChannelsCount * sizeof(HVec2);
	if(meshProtected.weights)
	{
		a += sizeof(MeshLoader::VertexWeight);
	}

	alignRoundUp(sizeof(F32), a);
	return a;
}

//==============================================================================
void Mesh::createVbos(const MeshLoader& loader)
{
	ANKI_ASSERT(meshProtected.vertsCount == loader.getPositions().size()
		&& meshProtected.vertsCount == loader.getNormals().size()
		&& meshProtected.vertsCount == loader.getTangents().size());

	// Calculate VBO size
	U32 vertexsize = calcVertexSize();
	U32 vbosize = vertexsize * meshProtected.vertsCount;

	// Create a temp buffer and populate it
	Vector<U8> buff(vbosize, 0);

	U8* ptra = &buff[0];
	for(U i = 0; i < meshProtected.vertsCount; i++)
	{
		U8* ptr = ptra;
		ANKI_ASSERT(ptr + vertexsize <= &buff[0] + vbosize);

		memcpy(ptr, &loader.getPositions()[i], sizeof(Vec3));
		ptr += sizeof(Vec3);

		memcpy(ptr, &loader.getNormals()[i], sizeof(HVec3));
		ptr += sizeof(HVec3);

		memcpy(ptr, &loader.getTangents()[i], sizeof(HVec4));
		ptr += sizeof(HVec4);

		for(U j = 0; j < meshProtected.texChannelsCount; j++)
		{
			memcpy(ptr, &loader.getTextureCoordinates(j)[i], sizeof(HVec2));
			ptr += sizeof(HVec2);
		}

		if(meshProtected.weights)
		{
			memcpy(ptr, &loader.getWeights()[i], 
				sizeof(MeshLoader::VertexWeight));
			ptr += sizeof(MeshLoader::VertexWeight);
		}

		ptra += vertexsize;
	}

	// Create VBO
	vbo.create(
		GL_ARRAY_BUFFER,
		vbosize,
		&buff[0],
		GL_STATIC_DRAW);

	/// Create the indices VBO
	indicesVbo.create(
		GL_ELEMENT_ARRAY_BUFFER,
		getVectorSizeInBytes(loader.getIndices()),
		&loader.getIndices()[0],
		GL_STATIC_DRAW);
}

//==============================================================================
void Mesh::getVboInfo(const VertexAttribute attrib, const Vbo*& v, U32& size,
	GLenum& type, U32& stride, U32& offset) const
{
	stride = calcVertexSize();

	// Set all to zero
	v = nullptr;
	size = 0;
	type = GL_NONE;
	offset = 0;

	switch(attrib)
	{
	case VA_POSITION:
		v = &vbo;
		size = 3;
		type = GL_FLOAT;
		offset = 0;
		break;
	case VA_NORMAL:
		v = &vbo;
		size = 3;
		type = GL_HALF_FLOAT;
		offset = sizeof(Vec3);
		break;
	case VA_TANGENT:
		v = &vbo;
		size = 4;
		type = GL_HALF_FLOAT;
		offset = sizeof(Vec3) + sizeof(HVec3);
		break;
	case VA_TEXTURE_COORD:
		if(meshProtected.texChannelsCount > 0)
		{
			v = &vbo;
			size = 2;
			type = GL_HALF_FLOAT;
			offset = sizeof(Vec3) + sizeof(HVec3) + sizeof(HVec4);
		}
		break;
	case VA_TEXTURE_COORD_1:
		if(meshProtected.texChannelsCount > 1)
		{
			v = &vbo;
			size = 2;
			type = GL_HALF_FLOAT;
			offset = sizeof(Vec3) + sizeof(HVec3) + sizeof(HVec4) 
				+ sizeof(HVec2);
		}
		break;
	case VA_BONE_COUNT:
		if(meshProtected.weights)
		{
			v = &vbo;
			size = 1;
			type = GL_UNSIGNED_SHORT;
			offset = sizeof(Vec3) + sizeof(HVec3) + sizeof(HVec4) 
				+ meshProtected.texChannelsCount * sizeof(HVec2);
		}
		break;
	case VA_BONE_IDS:
		if(meshProtected.weights)
		{
			v = &vbo;
			size = 4;
			type = GL_UNSIGNED_SHORT;
			offset = sizeof(Vec3) + sizeof(HVec3) + sizeof(HVec4) 
				+ meshProtected.texChannelsCount * sizeof(HVec2) + sizeof(U16);
		}
		break;
	case VA_BONE_WEIGHTS:
		if(meshProtected.weights)
		{
			v = &vbo;
			size = 4;
			type = GL_HALF_FLOAT;
			offset = sizeof(Vec3) + sizeof(HVec3) + sizeof(HVec4) 
				+ meshProtected.texChannelsCount * sizeof(HVec2) + sizeof(U16) 
				+ sizeof(U16) * 4;
		}
	case VA_INDICES:
		v = &indicesVbo;
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

		meshProtected.vertsCount = 0;
		meshProtected.subMeshes.reserve(4); // XXX
		meshProtected.indicesCount = 0;

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
				if(i > ANKI_MAX_MULTIDRAW_PRIMITIVES)
				{
					throw ANKI_EXCEPTION("Max number of submeshes exceeded");
				}

				// Load
				subLoader.load(subMeshFilename.c_str());
				loader = &subLoader;

				// Sanity checks
				if(meshProtected.weights != (loader->getWeights().size() > 1))
				{
					throw ANKI_EXCEPTION("All sub meshes should have or not "
						"have vertex weights");
				}

				if(meshProtected.texChannelsCount 
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
				meshProtected.weights = loader->getWeights().size() > 1;
				meshProtected.texChannelsCount = 
					loader->getTextureChannelsCount();
			}

			// Push back the new submesh
			SubMesh submesh;

			submesh.indicesCount = loader->getIndices().size();
			submesh.indicesOffset = meshProtected.indicesCount * sizeof(U16);
			submesh.obb.set(loader->getPositions());

			meshProtected.subMeshes.push_back(submesh);

			// Set the global numbers
			meshProtected.vertsCount += loader->getPositions().size();
			meshProtected.indicesCount += loader->getIndices().size();

			// Move to next
			meshEl = meshEl.getNextSiblingElement("mesh");
			++i;
		} while(meshEl);

		// Create the bucket mesh
		createVbos(fullLoader);
		meshProtected.obb.set(fullLoader.getPositions());
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("BucketMesh loading failed: " + filename) << e;
	}
}

} // end namespace anki

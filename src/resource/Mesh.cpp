#include "anki/resource/Mesh.h"
#include "anki/resource/Material.h"
#include "anki/resource/MeshLoader.h"
#include "anki/gl/Vbo.h"
#include "anki/util/Functions.h"
#include <fstream>

namespace anki {

//==============================================================================
void Mesh::load(const char* filename)
{
	MeshLoader meshData(filename);

	// Set the non-VBO members
	vertsCount = meshData.getPositions().size();
	ANKI_ASSERT(vertsCount > 0);

	indicesCount.push_back(meshData.getLodsCount());
	for(U lod = 0; lod < indicesCount.size(); lod++)
	{
		indicesCount[lod] = meshData.getIndices(lod).size();
		ANKI_ASSERT(indicesCount[lod] > 0);
		ANKI_ASSERT(indicesCount[lod] % 3 == 0 && "Expecting triangles");
	}

	weights = meshData.getWeights().size() > 1;
	texChannelsCount = meshData.getTextureChannelsCount();

	try
	{
		createVbos(meshData);

		visibilityShape.set(meshData.getPositions());
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Mesh loading failed: " + filename) << e;
	}
}

//==============================================================================
U32 Mesh::calcVertexSize() const
{
	U32 a = (3 + 3 + 4 + texChannelsCount * 2) * sizeof(F32);
	if(weights)
	{
		a += sizeof(MeshLoader::VertexWeight);
	}
	return a;
}

//==============================================================================
void Mesh::createVbos(const MeshLoader& meshData)
{
	// Calculate VBO size
	U32 vertexsize = calcVertexSize();
	U32 vbosize = vertexsize * vertsCount;

	// Create a temp buffer and populate it
	Vector<U8> buff;
	buff.resize(vbosize);

	U8* ptr = &buff[0];
	for(U i = 0; i < vertsCount; i++)
	{
		ANKI_ASSERT(ptr + vertexsize <= &buff[0] + vbosize);

		*(Vec3*)ptr = meshData.getPositions()[i];
		ptr += sizeof(Vec3);

		*(Vec3*)ptr = meshData.getNormals()[i];
		ptr += sizeof(Vec3);

		*(Vec4*)ptr = meshData.getTangents()[i];
		ptr += sizeof(Vec4);

		for(U j = 0; j < texChannelsCount; j++)
		{
			*(Vec2*)ptr = meshData.getTexureCoordinates(j)[i];
			ptr += sizeof(Vec2);
		}

		if(weights)
		{
			*(MeshLoader::VertexWeight*)ptr = meshData.getWeights()[i];
			ptr += sizeof(MeshLoader::VertexWeight);
		}
	}

	// Create VBO
	vbo.create(
		GL_ARRAY_BUFFER,
		vbosize,
		&buff[0],
		GL_STATIC_DRAW);

	/// Create the indices VBOs
	indicesVbos.resize(meshData.getLodsCount());
	U lod = 0;
	for(Vbo& v : indicesVbos)
	{
		v.create(
			GL_ELEMENT_ARRAY_BUFFER,
			getVectorSizeInBytes(meshData.getIndices(lod)),
			&meshData.getIndices(lod)[0],
			GL_STATIC_DRAW);

		++lod;
	}
}

//==============================================================================
void Mesh::getVboInfo(
	const VertexAttribute attrib, const U32 lod, const U32 texChannel,
	Vbo* v, U32& size, GLenum& type, U32& stride, U32& offset)
{
	stride = calcVertexSize();

	switch(attrib)
	{
	case VA_POSITIONS:
		v = &vbo;
		size = 3;
		type = GL_FLOAT;
		offset = 0;
		break;
	case VA_NORMALS:
		v = &vbo;
		size = 3;
		type = GL_FLOAT;
		offset = sizeof(Vec3);
		break;
	case VA_TANGENTS:
		v = &vbo;
		size = 4;
		type = GL_FLOAT;
		offset = sizeof(Vec3) * 2;
		break;
	case VA_TEXTURE_COORDS:
		v = &vbo;
		size = 2;
		type = GL_FLOAT;
		offset = sizeof(Vec3) * 2 + sizeof(Vec4) + texChannel * sizeof(Vec2);
		break;
	case VA_WEIGHTS_BONE_COUNT:
		v = &vbo;
		size = 1;
		type = GL_UNSIGNED_INT;
		offset = sizeof(Vec3) * 2 + sizeof(Vec4) 
			+ texChannelsCount * sizeof(Vec2);
		break;
	case VA_WEIGHTS_BONE_IDS:
		v = &vbo;
		size = 4;
		type = GL_UNSIGNED_INT;
		offset = sizeof(Vec3) * 2 + sizeof(Vec4) 
			+ texChannelsCount * sizeof(Vec2) + sizeof(U32);
		break;
	case VA_WEIGHTS_BONE_WEIGHTS:
		v = &vbo;
		size = 4;
		type = GL_FLOAT;
		offset = sizeof(Vec3) * 2 + sizeof(Vec4) 
			+ texChannelsCount * sizeof(Vec2) + sizeof(U32) + sizeof(U32) * 4;
	case VA_INDICES:
		v = &indicesVbos[lod];
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}
}

} // end namespace anki

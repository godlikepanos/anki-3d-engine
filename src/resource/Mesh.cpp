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

	vertsCount = meshData.getVertCoords().size();
	indicesCount.push_back(meshData.getVertIndeces().size());
	weights = meshData.getVertWeights().size() > 1;
	texChannelsCount = 1;

	try
	{
		// Sanity checks
		//
		if(meshData.getVertIndeces().size() < 1
			|| meshData.getVertCoords().size() < 1
			|| meshData.getVertNormals().size() < 1)
		{
			throw ANKI_EXCEPTION("Empty one of the required vectors");
		}

		createVbos(meshData);

		visibilityShape.set(meshData.getVertCoords());
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Mesh loading failed: " + filename) << e;
	}
}

//==============================================================================
void Mesh::createVbos(const MeshLoader& meshData)
{
	// Calculate VBO size
	U32 vbosize = (3 + 3 + 4 + texChannelsCount * 2) * sizeof(F32);

	if(weights)
	{
		vbosize += sizeof(MeshLoader::VertexWeight);
	}

	vbosize *= vertsCount;

	// Create a temp buffer and populate it
	Vector<U8> buff;
	buff.resize(vbosize);

	U8* ptr = &buff[0];
	for(U i = 0; i < vertsCount; i++)
	{
		*(Vec3*)ptr = meshData.getVertCoords()[i];
		ptr += sizeof(Vec3);

		*(Vec3*)ptr = meshData.getVertNormalsCoords()[i];
		ptr += sizeof(Vec3);

		*(Vec4*)ptr = meshData.getVertTangentsCoords()[i];
		ptr += sizeof(Vec4);

		for(U j = 0; j < texChannelsCound; j++)
		{
			*(Vec2*)ptr = meshData.getTexCoords(j)[i];
			ptr += sizeof(Vec2);
		}

		if(weights)
		{
			*(MeshLoader::VertexWeight*)ptr = meshData.getVertWeights()[i];
			ptr += sizeof(MeshLoader::VertexWeight);
		}
	}

	// Create VBO
	vbo.create(
		GL_ELEMENT_ARRAY_BUFFER,
		vbosize,
		&buff[0],
		GL_STATIC_DRAW);

	/// XXX
}

} // end namespace

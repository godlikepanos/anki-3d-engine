#include "anki/resource/Mesh.h"
#include "anki/resource/Material.h"
#include "anki/resource/MeshData.h"
#include "anki/gl/Vbo.h"
#include "anki/util/Util.h"
#include <fstream>
#include <boost/lexical_cast.hpp>


//==============================================================================
// load                                                                        =
//==============================================================================
void Mesh::load(const char* filename)
{
	MeshData meshData(filename);

	vertIdsNum = meshData.getVertIndeces().size();
	vertsNum = meshData.getVertCoords().size();

	try
	{
		//
		// Sanity checks
		//
		if(meshData.getVertIndeces().size() < 1 ||
			meshData.getVertCoords().size() < 1 ||
			meshData.getVertNormals().size() < 1)
		{
			throw ANKI_EXCEPTION("Empty one of the required vectors");
		}

		createVbos(meshData);

		visibilityShape.set(meshData.getVertCoords());
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Mesh \"" + filename + "\": " + e.what());
	}
}


//==============================================================================
// createVbos                                                                  =
//==============================================================================
void Mesh::createVbos(const MeshData& meshData)
{
	vbos[VBO_VERT_INDECES].create(
		GL_ELEMENT_ARRAY_BUFFER,
		util::getVectorSizeInBytes(meshData.getVertIndeces()),
		&meshData.getVertIndeces()[0],
		GL_STATIC_DRAW);

	vbos[VBO_VERT_POSITIONS].create(
		GL_ARRAY_BUFFER,
		util::getVectorSizeInBytes(meshData.getVertCoords()),
		&meshData.getVertCoords()[0],
		GL_STATIC_DRAW);

	vbos[VBO_VERT_NORMALS].create(
		GL_ARRAY_BUFFER,
		util::getVectorSizeInBytes(meshData.getVertNormals()),
		&meshData.getVertNormals()[0],
		GL_STATIC_DRAW);

	if(meshData.getVertTangents().size() > 1)
	{
		vbos[VBO_VERT_TANGENTS].create(
			GL_ARRAY_BUFFER,
			util::getVectorSizeInBytes(meshData.getVertTangents()),
			&meshData.getVertTangents()[0],
			GL_STATIC_DRAW);
	}

	if(meshData.getTexCoords().size() > 1)
	{
		vbos[VBO_TEX_COORDS].create(
			GL_ARRAY_BUFFER,
			util::getVectorSizeInBytes(meshData.getTexCoords()),
			&meshData.getTexCoords()[0],
			GL_STATIC_DRAW);
	}

	if(meshData.getVertWeights().size() > 1)
	{
		vbos[VBO_VERT_WEIGHTS].create(
			GL_ARRAY_BUFFER,
			util::getVectorSizeInBytes(meshData.getVertWeights()),
			&meshData.getVertWeights()[0],
			GL_STATIC_DRAW);
	}
}

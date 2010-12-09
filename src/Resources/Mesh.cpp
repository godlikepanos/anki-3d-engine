#include <fstream>
#include <boost/lexical_cast.hpp>
#include "Mesh.h"
#include "Material.h"
#include "MeshData.h"
#include "Vbo.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Mesh::Mesh():
	Resource(RT_MESH),
	Object(NULL)
{
	memset(&vbos[0], NULL, sizeof(vbos));
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Mesh::load(const char* filename)
{
	MeshData meshData(filename);

	vertIdsNum = meshData.getVertIndeces().size();

	try
	{
		//
		// Sanity checks
		//
		if(meshData.getVertIndeces().size() < 1 || meshData.getVertCoords().size() < 1 ||
			 meshData.getVertNormals().size() < 1 || meshData.getVertTangents().size() < 1)
		{
			throw EXCEPTION("Empty one of the required vectors");
		}

		createVbos(meshData);
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Mesh \"" + filename + "\": " + e.what());
	}
}


//======================================================================================================================
// createVbos                                                                                                          =
//======================================================================================================================
void Mesh::createVbos(const MeshData& meshData)
{
	vbos[VBO_VERT_INDECES] = new Vbo(GL_ELEMENT_ARRAY_BUFFER, meshData.getVertIndeces().getSizeInBytes(),
	                                 &meshData.getVertIndeces()[0], GL_STATIC_DRAW, this);
	vbos[VBO_VERT_POSITIONS] = new Vbo(GL_ARRAY_BUFFER, meshData.getVertCoords().getSizeInBytes(),
	                                   &meshData.getVertCoords()[0], GL_STATIC_DRAW, this);
	vbos[VBO_VERT_NORMALS] = new Vbo(GL_ARRAY_BUFFER, meshData.getVertNormals().getSizeInBytes(),
	                                 &meshData.getVertNormals()[0], GL_STATIC_DRAW, this);

	if(meshData.getVertTangents().size() > 1)
	{
		vbos[VBO_VERT_TANGENTS] = new Vbo(GL_ARRAY_BUFFER, meshData.getVertTangents().getSizeInBytes(),
		                                  &meshData.getVertTangents()[0], GL_STATIC_DRAW, this);
	}
	else
	{
		vbos[VBO_VERT_TANGENTS] = NULL;
	}

	if(meshData.getTexCoords().size() > 1)
	{
		vbos[VBO_TEX_COORDS] = new Vbo(GL_ARRAY_BUFFER, meshData.getTexCoords().getSizeInBytes(),
		                               &meshData.getTexCoords()[0], GL_STATIC_DRAW, this);
	}
	else
	{
		vbos[VBO_TEX_COORDS] = NULL;
	}

	if(meshData.getVertWeights().size() > 1)
	{
		vbos[VBO_VERT_WEIGHTS] = new Vbo(GL_ARRAY_BUFFER, meshData.getVertWeights().getSizeInBytes(),
		                                 &meshData.getVertWeights()[0], GL_STATIC_DRAW, this);
	}
	else
	{
		vbos[VBO_VERT_WEIGHTS] = NULL;
	}
}

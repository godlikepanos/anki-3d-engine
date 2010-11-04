#include <fstream>
#include <boost/lexical_cast.hpp>
#include "Mesh.h"
#include "Material.h"
#include "MeshData.h"


#define BUFFER_OFFSET(i) ((char *)NULL + (i))


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Mesh::load(const char* filename)
{
	MeshData meshData(filename);

	vertIdsNum = meshData.getVertIndeces().size();

	try
	{
		material.loadRsrc(meshData.getMaterialName().c_str());

		if(isRenderable())
		{
			//
			// Sanity checks
			//
			if(meshData.getVertIndeces().size() < 1 || meshData.getVertCoords().size() < 1 ||
			   meshData.getVertNormals().size() < 1)
			{
				throw EXCEPTION("Empty required arrays");
			}

			// shader needs text coords and mesh does not have any
			if(material->stdAttribVars[Material::SAV_TEX_COORDS] != NULL && meshData.getTexCoords().size() < 1)
			{
				throw EXCEPTION("The shader program (\"" + material->shaderProg->getRsrcName() +
												"\") needs texture coord information that the mesh doesn't have");
			}

			// shader has HW skinning and mesh does not have vert weights
			if(material->hasHWSkinning() && meshData.getVertWeights().size() < 1)
			{
				throw EXCEPTION("The shader program (\"" + material->shaderProg->getRsrcName() +
												"\") needs vertex weights that the mesh doesn't have");
			}


			createVbos(meshData);
			createVao(vao, *material.get());
			createVao(depthVao, *material->dpMtl.get());
		}
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
	vbos.vertIndeces = new Vbo(GL_ELEMENT_ARRAY_BUFFER, meshData.getVertIndeces().getSizeInBytes(),
	                           &meshData.getVertIndeces()[0], GL_STATIC_DRAW, this);
	vbos.vertCoords = new Vbo(GL_ARRAY_BUFFER, meshData.getVertCoords().getSizeInBytes(),
	                          &meshData.getVertCoords()[0], GL_STATIC_DRAW, this);
	vbos.vertNormals = new Vbo(GL_ARRAY_BUFFER, meshData.getVertNormals().getSizeInBytes(),
	                           &meshData.getVertNormals()[0], GL_STATIC_DRAW, this);

	if(meshData.getVertTangents().size() > 1)
	{
		vbos.vertTangents = new Vbo(GL_ARRAY_BUFFER, meshData.getVertTangents().getSizeInBytes(),
		                            &meshData.getVertTangents()[0], GL_STATIC_DRAW, this);
	}

	if(meshData.getTexCoords().size() > 1)
	{
		vbos.texCoords = new Vbo(GL_ARRAY_BUFFER, meshData.getTexCoords().getSizeInBytes(),
		                         &meshData.getTexCoords()[0], GL_STATIC_DRAW, this);
	}

	if(meshData.getVertWeights().size() > 1)
	{
		vbos.vertWeights = new Vbo(GL_ARRAY_BUFFER, meshData.getVertWeights().getSizeInBytes(),
		                           &meshData.getVertWeights()[0], GL_STATIC_DRAW, this);
	}
}


//======================================================================================================================
// createVao                                                                                                           =
//======================================================================================================================
void Mesh::createVao(Vao* vao, Material& mtl)
{
	vao = new Vao(this);

	if(mtl.stdAttribVars[Material::SAV_POSITION] != NULL)
	{
		vao->attachArrayBufferVbo(*vbos.vertCoords, *mtl.stdAttribVars[Material::SAV_POSITION], 3, GL_FLOAT,
		                          GL_FALSE, 0, NULL);
	}

	if(mtl.stdAttribVars[Material::SAV_NORMAL] != NULL)
	{
		vao->attachArrayBufferVbo(*vbos.vertNormals, *mtl.stdAttribVars[Material::SAV_NORMAL], 3, GL_FLOAT,
		                          GL_FALSE, 0, NULL);
	}

	if(mtl.stdAttribVars[Material::SAV_TANGENT] != NULL)
	{
		vao->attachArrayBufferVbo(*vbos.vertTangents, *mtl.stdAttribVars[Material::SAV_TANGENT], 4, GL_FLOAT,
		                          GL_FALSE, 0, NULL);
	}

	if(mtl.stdAttribVars[Material::SAV_TEX_COORDS] != NULL)
	{
		vao->attachArrayBufferVbo(*vbos.texCoords, *mtl.stdAttribVars[Material::SAV_TEX_COORDS], 2, GL_FLOAT,
		                          GL_FALSE, 0, NULL);
	}

	if(mtl.stdAttribVars[Material::SAV_VERT_WEIGHT_BONES_NUM] != NULL)
	{
		vao->attachArrayBufferVbo(*vbos.vertWeights, *mtl.stdAttribVars[Material::SAV_VERT_WEIGHT_BONES_NUM], 1,
		                          GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(0));
	}

	if(mtl.stdAttribVars[Material::SAV_VERT_WEIGHT_BONE_IDS] != NULL)
	{
		vao->attachArrayBufferVbo(*vbos.vertWeights, *mtl.stdAttribVars[Material::SAV_VERT_WEIGHT_BONE_IDS], 4,
		                          GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(4));
	}

	if(mtl.stdAttribVars[Material::SAV_VERT_WEIGHT_WEIGHTS] != NULL)
	{
		vao->attachArrayBufferVbo(*vbos.vertWeights, *mtl.stdAttribVars[Material::SAV_VERT_WEIGHT_WEIGHTS], 4,
		                          GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(20));
	}

	vao->attachElementArrayBufferVbo(*vbos.vertIndeces);
}


//======================================================================================================================
// isRenderable                                                                                                        =
//======================================================================================================================
bool Mesh::isRenderable() const
{
	return material.get() != NULL;
}

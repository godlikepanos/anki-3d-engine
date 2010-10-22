#include <fstream>
#include <boost/lexical_cast.hpp>
#include "Mesh.h"
#include "Material.h"
#include "MeshData.h"


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
			createVbos(meshData);

			//
			// Sanity checks
			//

			// shader needs text coords and mesh does not have any
			if(material->stdAttribVars[Material::SAV_TEX_COORDS] != NULL && !vbos.texCoords.isCreated())
			{
				throw EXCEPTION("The shader program (\"" + material->shaderProg->getRsrcName() +
												"\") needs texture coord information that the mesh (\"" +
												getRsrcName() + "\") doesn't have");
			}

			// shader has HW skinning and mesh does not have vert weights
			if(material->hasHWSkinning() && !vbos.vertWeights.isCreated())
			{
				throw EXCEPTION("The shader program (\"" + material->shaderProg->getRsrcName() +
												"\") needs vertex weights that the mesh (\"" +
												getRsrcName() + "\") doesn't have");
			}
		}
	}
	catch(Exception& e)
	{
		throw EXCEPTION("Mesh \"" + filename + "\": " + e.what());
	}
}


//======================================================================================================================
// createVbos                                                                                                          =
//======================================================================================================================
void Mesh::createVbos(const MeshData& meshData)
{
	vbos.vertIndeces.create(GL_ELEMENT_ARRAY_BUFFER, meshData.getVertIndeces().getSizeInBytes(),
	                        &meshData.getVertIndeces()[0], GL_STATIC_DRAW);
	vbos.vertCoords.create(GL_ARRAY_BUFFER, meshData.getVertCoords().getSizeInBytes(),
	                       &meshData.getVertCoords()[0], GL_STATIC_DRAW);
	vbos.vertNormals.create(GL_ARRAY_BUFFER, meshData.getVertNormals().getSizeInBytes(),
	                        &meshData.getVertNormals()[0], GL_STATIC_DRAW);

	if(meshData.getVertTangents().size() > 1)
	{
		vbos.vertTangents.create(GL_ARRAY_BUFFER, meshData.getVertTangents().getSizeInBytes(),
		                         &meshData.getVertTangents()[0], GL_STATIC_DRAW);
	}

	if(meshData.getTexCoords().size() > 1)
	{
		vbos.texCoords.create(GL_ARRAY_BUFFER, meshData.getTexCoords().getSizeInBytes(),
		                      &meshData.getTexCoords()[0], GL_STATIC_DRAW);
	}

	if(meshData.getVertWeights().size() > 1)
	{
		vbos.vertWeights.create(GL_ARRAY_BUFFER, meshData.getVertWeights().getSizeInBytes(),
		                        &meshData.getVertWeights()[0], GL_STATIC_DRAW);
	}
}


//======================================================================================================================
// isRenderable                                                                                                        =
//======================================================================================================================
bool Mesh::isRenderable() const
{
	return material.get() != NULL;
}

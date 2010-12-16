#include "ModelNodePatch.h"
#include "Material.h"
#include "MeshData.h"
#include "ModelPatch.h"


#define BUFFER_OFFSET(i) ((char *)NULL + (i))


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
ModelNodePatch::ModelNodePatch(const ModelPatch& modelPatch_, bool isSkinPatch):
	modelPatch(modelPatch_)
{
	if(!isSkinPatch)
	{
		for(uint i = 0; i < Mesh::VBOS_NUM; i++)
		{
			vbos[i] = &modelPatch.getMesh().getVbo((Mesh::Vbos)i);
		}

		createVao(modelPatch.getMaterial(), vbos, mainVao);
		createVao(modelPatch.getDpMaterial(), vbos, dpVao);
	}
	else
	{
		//
		// Create the TF VBOs
		//
		tfVbos[TF_VBO_POSITIONS].create(GL_ARRAY_BUFFER,
		                                modelPatch.getMesh().getVbo(Mesh::VBO_VERT_POSITIONS).getSizeInBytes(),
		                                NULL,
		                                GL_STATIC_DRAW);
	}
}


//======================================================================================================================
// createVao                                                                                                           =
//======================================================================================================================
void ModelNodePatch::createVao(const Material& mtl, const boost::array<const Vbo*, Mesh::VBOS_NUM>& vbos, Vao& vao)
{
	vao.create();

	if(mtl.getStdAttribVar(Material::SAV_POSITION) != NULL)
	{
		vao.attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_POSITIONS], *mtl.getStdAttribVar(Material::SAV_POSITION),
		                         3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getStdAttribVar(Material::SAV_NORMAL) != NULL)
	{
		vao.attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_NORMALS], *mtl.getStdAttribVar(Material::SAV_NORMAL),
		                         3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getStdAttribVar(Material::SAV_TANGENT) != NULL)
	{
		vao.attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_TANGENTS], *mtl.getStdAttribVar(Material::SAV_TANGENT),
		                         4, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getStdAttribVar(Material::SAV_TEX_COORDS) != NULL)
	{
		vao.attachArrayBufferVbo(*vbos[Mesh::VBO_TEX_COORDS], *mtl.getStdAttribVar(Material::SAV_TEX_COORDS),
		                         2, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	/*if(mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_BONES_NUM) != NULL)
	{
		vao.attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_WEIGHTS],
		                         *mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_BONES_NUM), 1,
		                         GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(0));
	}

	if(mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_BONE_IDS) != NULL)
	{
		vao.attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_WEIGHTS],
		                         *mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_BONE_IDS), 4,
		                         GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(4));
	}

	if(mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_WEIGHTS) != NULL)
	{
		vao.attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_WEIGHTS],
		                         *mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_WEIGHTS), 4,
		                         GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(20));
	}*/

	vao.attachElementArrayBufferVbo(*vbos[Mesh::VBO_VERT_INDECES]);
}

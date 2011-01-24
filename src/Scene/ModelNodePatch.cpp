#include <boost/array.hpp>
#include "ModelNodePatch.h"
#include "Material.h"
#include "MeshData.h"
#include "ModelPatch.h"
#include "ModelNode.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
ModelNodePatch::ModelNodePatch(const ModelNode& modelNode, const ModelPatch& modelPatch_):
	SceneNodePatch(modelNode),
	modelPatchRsrc(modelPatch_)
{
	boost::array<const Vbo*, Mesh::VBOS_NUM> vboArr;

	for(uint i = 0; i < Mesh::VBOS_NUM; i++)
	{
		vboArr[i] = &modelPatchRsrc.getMesh().getVbo((Mesh::Vbos)i);
	}

	createVao(modelPatchRsrc.getCpMtl(), vboArr, cpVao);
	createVao(modelPatchRsrc.getDpMtl(), vboArr, dpVao);
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

	vao.attachElementArrayBufferVbo(*vbos[Mesh::VBO_VERT_INDECES]);
}

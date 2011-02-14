#include "SkinPatchNode.h"
#include "SkinNode.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
SkinPatchNode::SkinPatchNode(const ModelPatch& modelPatch, SkinNode* parent):
	PatchNode(modelPatch, parent)
{
	boost::array<const Vbo*, Mesh::VBOS_NUM> vboArr;

	for(uint i = 0; i < Mesh::VBOS_NUM; i++)
	{
		vboArr[i] = &rsrc.getMesh().getVbo((Mesh::Vbos)i);
	}

	const Mesh& mesh = rsrc.getMesh();


	if(mesh.getVbo(Mesh::VBO_VERT_POSITIONS).isCreated())
	{
		tfVbos[TFV_POSITIONS].create(GL_ARRAY_BUFFER, mesh.getVbo(Mesh::VBO_VERT_POSITIONS).getSizeInBytes(),
	                               NULL, GL_STATIC_DRAW);
		vboArr[Mesh::VBO_VERT_POSITIONS] = &tfVbos[TFV_POSITIONS];
	}

	if(mesh.getVbo(Mesh::VBO_VERT_NORMALS).isCreated())
	{
		tfVbos[TFV_NORMALS].create(GL_ARRAY_BUFFER, mesh.getVbo(Mesh::VBO_VERT_NORMALS).getSizeInBytes(),
	                               NULL, GL_STATIC_DRAW);

		vboArr[Mesh::VBO_VERT_NORMALS] = &tfVbos[TFV_NORMALS];
	}

	if(mesh.getVbo(Mesh::VBO_VERT_TANGENTS).isCreated())
	{
		tfVbos[TFV_TANGENTS].create(GL_ARRAY_BUFFER, mesh.getVbo(Mesh::VBO_VERT_TANGENTS).getSizeInBytes(),
	                               NULL, GL_STATIC_DRAW);

		vboArr[Mesh::VBO_VERT_TANGENTS] = &tfVbos[TFV_TANGENTS];
	}


	createVao(rsrc.getCpMtl(), vboArr, cpVao);
	createVao(rsrc.getDpMtl(), vboArr, dpVao);
}

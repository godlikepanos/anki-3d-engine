#include "SkinPatchNode.h"
#include "SkinNode.h"
#include "SkinsDeformer.h"


#define BUFFER_OFFSET(i) ((char *)NULL + (i))


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

	//
	// Create the VAOs
	//

	tfVao.create();

	// Positions
	if(mesh.getVbo(Mesh::VBO_VERT_POSITIONS).isCreated())
	{
		tfVbos[TFV_POSITIONS].create(GL_ARRAY_BUFFER, mesh.getVbo(Mesh::VBO_VERT_POSITIONS).getSizeInBytes(),
	                               NULL, GL_STATIC_DRAW);
		vboArr[Mesh::VBO_VERT_POSITIONS] = &tfVbos[TFV_POSITIONS];

		tfVao.attachArrayBufferVbo(tfVbos[TFV_POSITIONS], SkinsDeformer::TFSPA_POSITION, 3, GL_FLOAT, false, 0, NULL);
	}

	// Normals
	if(mesh.getVbo(Mesh::VBO_VERT_NORMALS).isCreated())
	{
		tfVbos[TFV_NORMALS].create(GL_ARRAY_BUFFER, mesh.getVbo(Mesh::VBO_VERT_NORMALS).getSizeInBytes(),
	                               NULL, GL_STATIC_DRAW);

		vboArr[Mesh::VBO_VERT_NORMALS] = &tfVbos[TFV_NORMALS];

		tfVao.attachArrayBufferVbo(tfVbos[TFV_NORMALS], SkinsDeformer::TFSPA_NORMAL, 3, GL_FLOAT, false, 0, NULL);
	}

	// Tangents
	if(mesh.getVbo(Mesh::VBO_VERT_TANGENTS).isCreated())
	{
		tfVbos[TFV_TANGENTS].create(GL_ARRAY_BUFFER, mesh.getVbo(Mesh::VBO_VERT_TANGENTS).getSizeInBytes(),
	                               NULL, GL_STATIC_DRAW);

		vboArr[Mesh::VBO_VERT_TANGENTS] = &tfVbos[TFV_TANGENTS];

		tfVao.attachArrayBufferVbo(tfVbos[TFV_TANGENTS], SkinsDeformer::TFSPA_TANGENT, 4, GL_FLOAT, false, 0, NULL);
	}

	// Attach some extra stuff to the tfVao
	ASSERT(mesh.getVbo(Mesh::VBO_VERT_WEIGHTS).isCreated());

	tfVao.attachArrayBufferVbo(mesh.getVbo(Mesh::VBO_VERT_WEIGHTS), SkinsDeformer::TFSPA_VERT_WEIGHT_BONES_NUM, 1,
	                           GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(0));


	tfVao.attachArrayBufferVbo(mesh.getVbo(Mesh::VBO_VERT_WEIGHTS), SkinsDeformer::TFSPA_VERT_WEIGHT_BONE_IDS, 4,
	                           GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(4));

	tfVao.attachArrayBufferVbo(mesh.getVbo(Mesh::VBO_VERT_WEIGHTS),SkinsDeformer::TFSPA_VERT_WEIGHT_WEIGHTS, 4,
	                           GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(20));


	// Rendering VAOs
	createVao(rsrc.getCpMtl(), vboArr, cpVao);
	createVao(rsrc.getDpMtl(), vboArr, dpVao);
}

#include "anki/scene/SkinPatchNode.h"
#include "anki/scene/SkinNode.h"
#include "anki/resource/MeshData.h"


namespace anki {


#define BUFFER_OFFSET(i) ((char *)NULL + (i))


//==============================================================================
// Constructor                                                                 =
//==============================================================================
SkinPatchNode::SkinPatchNode(const ModelPatch& modelPatch_, SkinNode& parent)
:	PatchNode(modelPatch_, SNF_NONE, parent)
{
	VboArray vboArr;

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
		tfVbos[TFV_POSITIONS].create(GL_ARRAY_BUFFER,
			mesh.getVbo(Mesh::VBO_VERT_POSITIONS).getSizeInBytes(),
			NULL,
			GL_STATIC_DRAW);

		vboArr[Mesh::VBO_VERT_POSITIONS] = &tfVbos[TFV_POSITIONS];

		tfVao.attachArrayBufferVbo(mesh.getVbo(Mesh::VBO_VERT_POSITIONS),
			POSITION_LOC,
			3,
			GL_FLOAT,
			false,
			0,
			NULL);
	}

	// Normals
	if(mesh.getVbo(Mesh::VBO_VERT_NORMALS).isCreated())
	{
		tfVbos[TFV_NORMALS].create(GL_ARRAY_BUFFER,
			mesh.getVbo(Mesh::VBO_VERT_NORMALS).getSizeInBytes(),
			NULL,
			GL_STATIC_DRAW);

		vboArr[Mesh::VBO_VERT_NORMALS] = &tfVbos[TFV_NORMALS];

		tfVao.attachArrayBufferVbo(mesh.getVbo(Mesh::VBO_VERT_NORMALS),
			NORMAL_LOC,
			3,
			GL_FLOAT,
			false,
			0,
			NULL);
	}

	// Tangents
	if(mesh.getVbo(Mesh::VBO_VERT_TANGENTS).isCreated())
	{
		tfVbos[TFV_TANGENTS].create(GL_ARRAY_BUFFER,
			mesh.getVbo(Mesh::VBO_VERT_TANGENTS).getSizeInBytes(),
			NULL,
			GL_STATIC_DRAW);

		vboArr[Mesh::VBO_VERT_TANGENTS] = &tfVbos[TFV_TANGENTS];

		tfVao.attachArrayBufferVbo(mesh.getVbo(Mesh::VBO_VERT_TANGENTS),
			TANGENT_LOC,
			4,
			GL_FLOAT,
			false,
			0,
			NULL);
	}

	// Attach some extra stuff to the tfVao
	ANKI_ASSERT(mesh.getVbo(Mesh::VBO_VERT_WEIGHTS).isCreated());

	tfVao.attachArrayBufferVbo(mesh.getVbo(Mesh::VBO_VERT_WEIGHTS),
		VERT_WEIGHT_BONES_NUM_LOC,
		1,
		GL_FLOAT,
		GL_FALSE,
		sizeof(MeshData::VertexWeight),
		BUFFER_OFFSET(0));


	tfVao.attachArrayBufferVbo(mesh.getVbo(Mesh::VBO_VERT_WEIGHTS),
		VERT_WEIGHT_BONE_IDS_LOC,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(MeshData::VertexWeight),
		BUFFER_OFFSET(4));

	tfVao.attachArrayBufferVbo(mesh.getVbo(Mesh::VBO_VERT_WEIGHTS),
		VERT_WEIGHT_WEIGHTS_LOC,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(MeshData::VertexWeight),
		BUFFER_OFFSET(20));

	// Create the rendering VAOs
	for(uint i = 0; i < PASS_TYPES_NUM; i++)
	{
		createVao(rsrc.getMaterial(), vboArr, vaos[i]);
	}
}


} // end namespace

#include "PatchNode.h"
#include "rsrc/Material.h"
#include "MaterialRuntime.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
PatchNode::PatchNode(const ModelPatch& modelPatch, SceneNode* parent)
:	RenderableNode(true, parent),
	rsrc(modelPatch),
	mtlRun(new MaterialRuntime(rsrc.getMaterial()))
{}


//==============================================================================
// createVao                                                                   =
//==============================================================================
void PatchNode::createVao(const Material& mtl, const boost::array<const Vbo*,
	Mesh::VBOS_NUM>& vbos, Vao& vao)
{
	vao.create();

	if(mtl.buildinVariableExits(MaterialBuildinVariable::MV_POSITION))
	{
		ASSERT(vbos[Mesh::VBO_VERT_POSITIONS] != NULL);

		vao.attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_POSITIONS],
			0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.buildinVariableExits(MaterialBuildinVariable::MV_NORMAL))
	{
		ASSERT(vbos[Mesh::VBO_VERT_NORMALS] != NULL);

		vao.attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_NORMALS],
			1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.buildinVariableExits(MaterialBuildinVariable::MV_TANGENT))
	{
		ASSERT(vbos[Mesh::VBO_VERT_TANGENTS] != NULL);

		vao.attachArrayBufferVbo(*vbos[Mesh::VBO_VERT_TANGENTS],
			2, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.buildinVariableExits(MaterialBuildinVariable::MV_TEX_COORDS))
	{
		vao.attachArrayBufferVbo(*vbos[Mesh::VBO_TEX_COORDS],
			3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	vao.attachElementArrayBufferVbo(*vbos[Mesh::VBO_VERT_INDECES]);
}

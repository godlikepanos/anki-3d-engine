#include <boost/array.hpp>
#include "ModelPatchNode.h"
#include "Material.h"
#include "MeshData.h"
#include "ModelPatch.h"
#include "ModelNode.h"
#include "ModelNode.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
ModelPatchNode::ModelPatchNode(const ModelPatch& modelPatch_, ModelNode* parent):
	PatchNode(modelPatch_, parent)
{
	boost::array<const Vbo*, Mesh::VBOS_NUM> vboArr;

	for(uint i = 0; i < Mesh::VBOS_NUM; i++)
	{
		vboArr[i] = &rsrc.getMesh().getVbo((Mesh::Vbos)i);
	}

	createVao(rsrc.getCpMtl(), vboArr, cpVao);
	createVao(rsrc.getDpMtl(), vboArr, dpVao);
}

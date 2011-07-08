#include <boost/array.hpp>
#include "ModelPatchNode.h"
#include "Resources/Material.h"
#include "Resources/MeshData.h"
#include "Resources/ModelPatch.h"
#include "ModelNode.h"
#include "ModelNode.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
ModelPatchNode::ModelPatchNode(const ModelPatch& modelPatch_,
	ModelNode* parent)
:	PatchNode(modelPatch_, parent),
	modelPatch(modelPatch_)
{
	boost::array<const Vbo*, Mesh::VBOS_NUM> vboArr;

	for(uint i = 0; i < Mesh::VBOS_NUM; i++)
	{
		vboArr[i] = &rsrc.getMesh().getVbo((Mesh::Vbos)i);
	}

	createVao(rsrc.getCpMtl(), vboArr, cpVao);
	createVao(rsrc.getDpMtl(), vboArr, dpVao);
}


//==============================================================================
// moveUpdate                                                                  =
//==============================================================================
void ModelPatchNode::moveUpdate()
{
	visibilityShapeWSpace =
		modelPatch.getMesh().getVisibilityShape().getTransformed(
		getWorldTransform());
}

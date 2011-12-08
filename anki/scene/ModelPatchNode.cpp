#include <boost/array.hpp>
#include "anki/scene/ModelPatchNode.h"
#include "anki/resource/Material.h"
#include "anki/resource/MeshData.h"
#include "anki/scene/ModelNode.h"
#include "anki/scene/ModelNode.h"


namespace anki {


//==============================================================================
ModelPatchNode::ModelPatchNode(const ModelPatch& modelPatch, ModelNode& parent)
	: PatchNode(modelPatch, SNF_NONE, parent)
{}



//==============================================================================
void ModelPatchNode::moveUpdate()
{
	visibilityShapeWSpace =
		rsrc.getMesh().getVisibilityShape().getTransformed(
		getWorldTransform());
}


} // end namespace

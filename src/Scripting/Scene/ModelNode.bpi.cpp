#include "ScriptingCommon.h"
#include "ModelNode.h"


WRAP(ModelNode)
{
	WRAP_CONTAINER(Vec<ModelPatchNode*>)

	class_<ModelNode, bases<SceneNode>, noncopyable>("ModelNode", no_init)
		.def("getModelPatchNodes", (Vec<ModelPatchNode*>& (ModelNode::*)())(&ModelNode::getModelPatchNodes),
		     return_value_policy<reference_existing_object>())
	;
}

#include "ScriptCommon.h"
#include "scene/ModelNode.h"


WRAP(ModelNode)
{
	WRAP_CONTAINER(std::vector<ModelPatchNode*>)

	class_<ModelNode, bases<SceneNode>, noncopyable>("ModelNode", no_init)
	;
}

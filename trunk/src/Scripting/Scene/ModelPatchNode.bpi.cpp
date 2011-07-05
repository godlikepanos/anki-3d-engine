#include "ScriptingCommon.h"
#include "Scene/ModelPatchNode.h"


WRAP(ModelPatchNode)
{
	class_<ModelPatchNode, bases<PatchNode>, noncopyable>("ModelPatchNode",
		no_init)
	;
}

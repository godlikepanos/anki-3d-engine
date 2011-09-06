#include "ScriptCommon.h"
#include "scene/ModelPatchNode.h"


WRAP(ModelPatchNode)
{
	class_<ModelPatchNode, bases<PatchNode>, noncopyable>("ModelPatchNode",
		no_init)
	;
}

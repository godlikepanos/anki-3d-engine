#include "anki/script/Common.h"
#include "anki/scene/ModelPatchNode.h"


WRAP(ModelPatchNode)
{
	class_<ModelPatchNode, bases<PatchNode>, noncopyable>("ModelPatchNode",
		no_init)
	;
}

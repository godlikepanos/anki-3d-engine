#include "anki/script/Common.h"
#include "anki/scene/PatchNode.h"


WRAP(PatchNode)
{
	class_<PatchNode, noncopyable>("PatchNode", no_init)
		.def("getMaterialRuntime", (MaterialRuntime& (PatchNode::*)())(
			&PatchNode::getMaterialRuntime),
			return_value_policy<reference_existing_object>())
	;
}

#include "ScriptingCommon.h"
#include "Scene/PatchNode.h"


WRAP(PatchNode)
{
	class_<PatchNode, noncopyable>("PatchNode", no_init)
		.def("getCpMtlRun", (MaterialRuntime& (PatchNode::*)())(&PatchNode::getCpMtlRun),
		     return_value_policy<reference_existing_object>())
		.def("getDpMtlRun", (MaterialRuntime& (PatchNode::*)())(&PatchNode::getDpMtlRun),
		     return_value_policy<reference_existing_object>())
	;
}

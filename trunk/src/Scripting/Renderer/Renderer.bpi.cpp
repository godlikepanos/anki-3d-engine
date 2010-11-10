#include "ScriptingCommon.h"
#include "Renderer.h"


WRAP(Renderer)
{
	class_<Renderer, noncopyable>("Renderer", no_init)
		.def("getFramesNum", &Renderer::getFramesNum)
		.def("getPps", &Renderer::getPps, return_value_policy<reference_existing_object>())
	;
}

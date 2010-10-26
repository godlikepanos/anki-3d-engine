#include "ScriptingCommon.h"
#include "Renderer.h"


WRAP(Renderer)
{
	class_<Renderer, noncopyable>("Renderer", no_init)
		.def("getFramesNum", &Renderer::getFramesNum)
		.def_readonly("pps", &Renderer::pps)
	;
}

#include "anki/script/ScriptCommon.h"
#include "anki/renderer/Renderer.h"


WRAP(Renderer)
{
	typedef Pps& (Renderer::* getPpsAccessor)();

	class_<Renderer, noncopyable>("Renderer", no_init)
		.def("getPps", (getPpsAccessor)(&Renderer::getPps),
			return_value_policy<reference_existing_object>())
		.def("getMsTime", &Renderer::getMsTime)
		.def("getIsTime", &Renderer::getIsTime)
		.def("getPpsTime", &Renderer::getPpsTime)
		.def("getBsTime", &Renderer::getBsTime)
		.def("getStagesProfilingEnabled",
			(bool (Renderer::*)() const)(&Renderer::getStagesProfilingEnabled))
		.def("setStagesProfilingEnabled", &Renderer::setStagesProfilingEnabled)
	;
}

#include "ScriptingCommon.h"
#include "Renderer/Renderer.h"


WRAP(Renderer)
{
	typedef Pps& (Renderer::* getPpsAccessor)();

	class_<Renderer, noncopyable>("Renderer", no_init)
		//BP_PROPERTY_BASIC_TYPE(uint, Renderer, framesNum, getter__, setter__)
		//.def("getFramesNum", &Renderer::getFramesNum)
		.def("getPps", (getPpsAccessor)(&Renderer::getPps), return_value_policy<reference_existing_object>())
	;
}

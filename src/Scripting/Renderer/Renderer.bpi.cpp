#include "ScriptingCommon.h"
#include "Renderer/Renderer.h"


WRAP(Renderer)
{
	typedef Pps& (Renderer::* getPpsAccessor)();

	class_<Renderer, noncopyable>("Renderer", no_init)
		.def("getPps", (getPpsAccessor)(&Renderer::getPps),
			return_value_policy<reference_existing_object>())
	;
}

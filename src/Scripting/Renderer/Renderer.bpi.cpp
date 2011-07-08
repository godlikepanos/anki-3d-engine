#include "ScriptingCommon.h"
#include "Renderer/Renderer.h"


WRAP(Renderer)
{
	typedef R::Pps& (R::Renderer::* getPpsAccessor)();

	class_<R::Renderer, noncopyable>("Renderer", no_init)
		.def("getPps", (getPpsAccessor)(&R::Renderer::getPps),
			return_value_policy<reference_existing_object>())
	;
}

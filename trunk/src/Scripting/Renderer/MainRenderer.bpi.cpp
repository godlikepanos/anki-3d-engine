#include "ScriptingCommon.h"
#include "Renderer/MainRenderer.h"
#include "Renderer/Dbg.h"


WRAP(MainRenderer)
{
	class_<R::MainRenderer, bases<R::Renderer>, noncopyable>("MainRenderer",
		no_init)
		.def("getDbg", (R::Dbg& (R::MainRenderer::*)())(
			&R::MainRenderer::getDbg),
			return_value_policy<reference_existing_object>())
	;
}

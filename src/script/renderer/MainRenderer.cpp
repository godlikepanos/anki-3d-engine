#include "ScriptCommon.h"
#include "renderer/MainRenderer.h"
#include "renderer/Dbg.h"


WRAP(MainRenderer)
{
	class_<MainRenderer, bases<Renderer>, noncopyable>("MainRenderer",
		no_init)
		.def("getDbg", (Dbg& (MainRenderer::*)())(
			&MainRenderer::getDbg),
			return_value_policy<reference_existing_object>())

		.def("getDbgTime", &MainRenderer::getDbgTime)
	;
}

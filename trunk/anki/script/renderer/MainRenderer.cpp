#include "anki/script/ScriptCommon.h"
#include "anki/renderer/MainRenderer.h"
#include "anki/renderer/Dbg.h"


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

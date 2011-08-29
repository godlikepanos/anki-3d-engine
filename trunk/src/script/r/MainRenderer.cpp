#include "Common.h"
#include "r/MainRenderer.h"
#include "r/Dbg.h"


WRAP(MainRenderer)
{
	using namespace r;

	class_<MainRenderer, bases<Renderer>, noncopyable>("r_MainRenderer",
		no_init)
		.def("getDbg", (Dbg& (MainRenderer::*)())(
			&MainRenderer::getDbg),
			return_value_policy<reference_existing_object>())

		.def("getDbgTime", &MainRenderer::getDbgTime)
	;
}

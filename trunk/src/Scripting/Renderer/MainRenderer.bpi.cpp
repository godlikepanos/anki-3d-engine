#include "ScriptingCommon.h"
#include "MainRenderer.h"
#include "Dbg.h"


WRAP(MainRenderer)
{
	class_<MainRenderer, bases<Renderer>, noncopyable>("MainRenderer", no_init)
		.def("getDbg", (Dbg& (MainRenderer::*)())(&MainRenderer::getDbg),
		     return_value_policy<reference_existing_object>())
	;
}


WRAP_SINGLETON(MainRendererSingleton)

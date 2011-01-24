#include "ScriptingCommon.h"
#include "App.h"
#include "Scene.h"
#include "MainRenderer.h"


WRAP(App)
{
	class_<App, noncopyable>("App", no_init)
		.def("quit", &App::quit)
	;
}

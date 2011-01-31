#include "ScriptingCommon.h"
#include "App.h"


WRAP(App)
{
	class_<App, noncopyable>("App", no_init)
		.def("quit", &App::quit)
	;
}


WRAP_SINGLETON(AppSingleton)

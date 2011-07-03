#include "ScriptingCommon.h"
#include "Core/App.h"


WRAP(App)
{
	class_<App, noncopyable>("App", no_init)
		.def("quit", &App::quit)
	;
}

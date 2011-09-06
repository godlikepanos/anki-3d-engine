#include "ScriptCommon.h"
#include "core/App.h"


WRAP(App)
{
	class_<App, noncopyable>("App", no_init)
		.def("quit", &App::quit)
	;
}

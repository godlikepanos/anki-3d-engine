#include "anki/script/Common.h"
#include "anki/core/App.h"

ANKI_WRAP(App)
{
	class_<App, noncopyable>("App", no_init)
		.def("quit", &App::quit)
	;
}

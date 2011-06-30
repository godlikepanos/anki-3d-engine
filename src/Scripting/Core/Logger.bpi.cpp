#include "ScriptingCommon.h"
#include "Core/Logger.h"


WRAP(Logger)
{
	class_<Logger, noncopyable>("Logger", no_init)
		.def("write", &Logger::write)
	;
}

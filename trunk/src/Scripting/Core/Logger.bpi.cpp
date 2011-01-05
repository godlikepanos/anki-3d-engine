#include "ScriptingCommon.h"
#include "Logger.h"


WRAP(Logger)
{
	class_<Logger, noncopyable>("Logger", no_init)
		.def("write", &Logger::write)
	;
}


WRAP_SINGLETON(LoggerSingleton)

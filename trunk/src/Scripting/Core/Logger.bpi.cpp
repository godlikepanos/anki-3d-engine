#include "ScriptingCommon.h"
#include "Logger.h"


WRAP(Logger)
{
	class_<Logger, noncopyable>("Logger", no_init)
		.def("getInstance", &Logger::getInstance, return_value_policy<reference_existing_object>())
		.staticmethod("getInstance")
		.def("write", &Logger::write)
	;
}

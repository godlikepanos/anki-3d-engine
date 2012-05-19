#include "anki/script/Common.h"
#include "anki/core/Logger.h"


WRAP(Logger)
{
	scope loggerScope(
	class_<Logger, noncopyable>("Logger", no_init)
		.def("write", &Logger::write)
	);

	enum_<Logger::MessageType>("MessageType")
		.value("MT_NORMAL", Logger::MT_NORMAL)
		.value("MT_ERROR", Logger::MT_ERROR)
		.value("MT_WARNING", Logger::MT_WARNING)
	;
}

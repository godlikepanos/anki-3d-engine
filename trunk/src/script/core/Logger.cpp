#include "anki/script/Common.h"
#include "anki/core/Logger.h"

ANKI_WRAP(Logger)
{
	scope loggerScope(
	class_<Logger, noncopyable>("Logger", no_init)
		.def("write", &Logger::write)
	);

	enum_<Logger::LoggerMessageType>("LoggerMessageType")
		.value("LMT_NORMAL", Logger::LMT_NORMAL)
		.value("LMT_ERROR", Logger::LMT_ERROR)
		.value("LMT_WARNING", Logger::LMT_WARNING)
	;
}

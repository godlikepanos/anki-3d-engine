#include "anki/core/Logger.h"
#include <cstring>

namespace anki {

//==============================================================================
void Logger::write(const char* file, int line, const char* func,
	LoggerMessageType type, const char* msg)
{
	mutex.lock();

	Info inf = {file, line, func, type, msg};
	ANKI_EMIT messageRecieved(inf);

	mutex.unlock();
}

} // end namespace

#ifndef ANKI_CORE_LOGGER_H
#define ANKI_CORE_LOGGER_H

#include "anki/util/Observer.h"
#include "anki/util/Singleton.h"
#include "anki/Config.h"
#include <array>
#include <mutex>
#include <sstream> // For the macros

namespace anki {

/// @addtogroup Core
/// @{

/// The logger singleton class. The logger cannot print errors or throw
/// exceptions, it has to recover somehow. Its thread safe
class Logger
{
public:
	/// Logger message type
	enum LoggerMessageType
	{
		LMT_NORMAL,
		LMT_ERROR,
		LMT_WARNING
	};

	/// Used as parammeter when emitting the signal
	struct Info
	{
		const char* file;
		int line;
		const char* func;
		LoggerMessageType type;
		const char* msg;
	};

	/// Send a message
	void write(const char* file, int line, const char* func,
		LoggerMessageType type, const char* msg);

	ANKI_SIGNAL(const Info&, messageRecieved)

private:
	std::mutex mutex; ///< For thread safety
};

typedef Singleton<Logger> LoggerSingleton;
/// @}

} // end namespace

//==============================================================================
// Macros                                                                      =
//==============================================================================

#define ANKI_LOGGER_MESSAGE(t, msg) \
	do \
	{ \
		std::stringstream ss; \
		ss << msg; \
		LoggerSingleton::get().write(ANKI_FILE, __LINE__, ANKI_FUNC, \
			t, ss.str().c_str()); \
	} while(false);

#define ANKI_LOGI(x) ANKI_LOGGER_MESSAGE(Logger::LMT_NORMAL, x)

#define ANKI_LOGW(x) ANKI_LOGGER_MESSAGE(Logger::LMT_WARNING, x)

#define ANKI_LOGE(x) ANKI_LOGGER_MESSAGE(Logger::LMT_ERROR, x)

#endif

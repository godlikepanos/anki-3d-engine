#ifndef ANKI_CORE_LOGGER_H
#define ANKI_CORE_LOGGER_H

#include "anki/util/Observer.h"
#include "anki/util/Singleton.h"
#include <array>
#include <mutex>

namespace anki {

/// @addtogroup Core
/// @{

/// The logger singleton class. The logger cannot print errors or throw
/// exceptions, it has to recover somehow. Its thread safe
class Logger
{
public:
	enum LoggerMessageType
	{
		LMT_NORMAL,
		LMT_ERROR,
		LMT_WARNING
	};

	/// XXX
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
		LoggerSingleton::get().write(__FILE__, __LINE__, __func__, \
			t, msg.str().c_str()); \
	} while(false);

#define ANKI_INFO(x) ANKI_LOGGER_MESSAGE(Logger::FMT_NORMAL, x)

#define ANKI_WARNING(x) ANKI_LOGGER_MESSAGE(Logger::FMT_WARNING, x)

#define ANKI_ERROR(x) ANKI_LOGGER_MESSAGE(Logger::FMT_ERROR, x)

#endif

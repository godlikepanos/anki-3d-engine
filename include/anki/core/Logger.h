#ifndef ANKI_CORE_LOGGER_H
#define ANKI_CORE_LOGGER_H

#include "anki/Config.h"
#include "anki/util/Observer.h"
#include "anki/util/Singleton.h"
#include <array>
#include <mutex>
#include <sstream> // For the macros

namespace anki {

/// @addtogroup Core
/// @{

/// The logger singleton class. The logger cannot print errors or throw
/// exceptions, it has to recover somehow. Its thread safe
/// To add a new signal: 
/// @code ANKI_CONNECT(&logger, messageRecieved, &instance, handle) @endcode
class Logger
{
public:
	ANKI_HAS_SLOTS(Logger)

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

	Logger();

	/// Send a message
	void write(const char* file, int line, const char* func,
		LoggerMessageType type, const char* msg);

	/// Send a formated message
	void writeFormated(const char* file, int line, const char* func,
		LoggerMessageType type, const char* fmt, ...);

	ANKI_SIGNAL(const Info&, messageRecieved)

private:
	std::mutex mutex; ///< For thread safety

	/// Depending on the OS implement a different handler. This one is the 
	/// default one
	void defaultSystemMessageHandler(const Info& info);
	ANKI_SLOT(defaultSystemMessageHandler, const Logger::Info&)
};

typedef Singleton<Logger> LoggerSingleton;
/// @}

} // end namespace

//==============================================================================
// Macros                                                                      =
//==============================================================================

#define ANKI_LOGGER_MESSAGE(t, ...) \
	do \
	{ \
		LoggerSingleton::get().writeFormated(ANKI_FILE, __LINE__, ANKI_FUNC, \
			t, __VA_ARGS__); \
	} while(false);

#define ANKI_LOGI(...) ANKI_LOGGER_MESSAGE(Logger::LMT_NORMAL, __VA_ARGS__)

#define ANKI_LOGW(...) ANKI_LOGGER_MESSAGE(Logger::LMT_WARNING, __VA_ARGS__)

#define ANKI_LOGE(...) ANKI_LOGGER_MESSAGE(Logger::LMT_ERROR, __VA_ARGS__)

#endif

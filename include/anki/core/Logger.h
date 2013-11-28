#ifndef ANKI_CORE_LOGGER_H
#define ANKI_CORE_LOGGER_H

#include "anki/Config.h"
#include "anki/util/Singleton.h"
#include "anki/util/File.h"
#include <mutex>

namespace anki {

/// @addtogroup Core
/// @{

/// The logger singleton class. The logger cannot print errors or throw
/// exceptions, it has to recover somehow. Its thread safe
/// To add a new signal: 
/// @code logger.addMessageHandler((void*)obj, &function) @endcode
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

	/// Logger init mask
	enum LoggerInitFlags
	{
		INIT_SYSTEM_MESSAGE_HANDLER = 1 << 0,
		INIT_LOG_FILE_MESSAGE_HANDLER  = 1 << 1
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

	/// The message handler callback
	using MessageHandlerCallback = void (*)(void*, const Info& info);

	/// Initialize the logger
	void init(U32 flags);

	/// Add a new message handler
	void addMessageHandler(void* data, MessageHandlerCallback callback);

	/// Send a message
	void write(const char* file, int line, const char* func,
		LoggerMessageType type, const char* msg);

	/// Send a formated message
	void writeFormated(const char* file, int line, const char* func,
		LoggerMessageType type, const char* fmt, ...);

private:
	std::mutex mutex; ///< For thread safety

	struct Handler
	{
		void* data;
		MessageHandlerCallback callback;
	};

	Vector<Handler> handlers;

	File logfile;
	
	static void defaultSystemMessageHandler(void*, const Info& info);
	static void logfileMessageHandler(void* vlogger, const Info& info);
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

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
	enum class MessageType: U8
	{
		NORMAL,
		ERROR,
		WARNING
	};

	/// Logger init mask
	enum class InitFlags: U8
	{
		NONE = 0,
		WITH_SYSTEM_MESSAGE_HANDLER = 1 << 0,
		WITH_LOG_FILE_MESSAGE_HANDLER  = 1 << 1
	};

	friend InitFlags operator|(InitFlags a, InitFlags b)
	{
		typedef std::underlying_type<InitFlags>::type Int;
		return 
			static_cast<InitFlags>(static_cast<Int>(a) | static_cast<Int>(b));
	}

	friend InitFlags operator&(InitFlags a, InitFlags b)
	{
		typedef std::underlying_type<InitFlags>::type Int;
		return 
			static_cast<InitFlags>(static_cast<Int>(a) & static_cast<Int>(b));
	}

	/// Used as parammeter when emitting the signal
	class Info
	{
	public:
		const char* m_file;
		I32 m_line;
		const char* m_func;
		MessageType m_type;
		const char* m_msg;
	};

	/// The message handler callback
	using MessageHandlerCallback = void (*)(void*, const Info& info);

	/// Initialize the logger
	void init(InitFlags flags, HeapAllocator<U8>& alloc);

	/// Add a new message handler
	void addMessageHandler(void* data, MessageHandlerCallback callback);

	/// Send a message
	void write(const char* file, int line, const char* func,
		MessageType type, const char* msg);

	/// Send a formated message
	void writeFormated(const char* file, int line, const char* func,
		MessageType type, const char* fmt, ...);

private:
	std::mutex m_mutex; ///< For thread safety

	class Handler
	{
	public:
		void* m_data;
		MessageHandlerCallback m_callback;
	};

	Vector<Handler> m_handlers;

	File m_logfile;
	
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

#define ANKI_LOGI(...) ANKI_LOGGER_MESSAGE(Logger::MessageType::NORMAL, \
	__VA_ARGS__)

#define ANKI_LOGW(...) ANKI_LOGGER_MESSAGE(Logger::MessageType::WARNING, \
	__VA_ARGS__)

#define ANKI_LOGE(...) ANKI_LOGGER_MESSAGE(Logger::MessageType::ERROR, \
	__VA_ARGS__)

#endif

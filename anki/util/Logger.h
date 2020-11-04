// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Config.h>
#include <anki/util/Singleton.h>
#include <anki/util/Thread.h>

namespace anki
{

// Forward
class File;

/// @addtogroup util_other
/// @{

/// Logger message type.
/// @memberof Logger
enum class LoggerMessageType : U8
{
	NORMAL,
	ERROR,
	WARNING,
	FATAL,
	COUNT
};

/// Used as parammeter when emitting the signal.
/// @memberof Logger
class LoggerMessageInfo
{
public:
	const char* m_file;
	I32 m_line;
	const char* m_func;
	LoggerMessageType m_type;
	const char* m_msg;
	const char* m_subsystem;
	ThreadId m_tid;
};

/// The message handler callback.
/// @memberof Logger
using LoggerMessageHandlerCallback = void (*)(void*, const LoggerMessageInfo& info);

/// The logger singleton class. The logger cannot print errors or throw exceptions, it has to recover somehow. It's
/// thread safe.
/// To add a new signal:
/// @code logger.addMessageHandler((void*)obj, &function) @endcode
class Logger
{
public:
	/// Initialize the logger and add the default message handler
	Logger();

	~Logger();

	/// Add a new message handler
	void addMessageHandler(void* data, LoggerMessageHandlerCallback callback);

	/// Remove a message handler.
	void removeMessageHandler(void* data, LoggerMessageHandlerCallback callback);

	/// Add file message handler.
	void addFileMessageHandler(File* file);

	/// Send a message
	void write(const char* file, int line, const char* func, const char* subsystem, LoggerMessageType type,
			   ThreadId tid, const char* msg);

	/// Send a formated message
	ANKI_CHECK_FORMAT(7, 8)
	void writeFormated(const char* file, int line, const char* func, const char* subsystem, LoggerMessageType type,
					   ThreadId tid, const char* fmt, ...);

private:
	class Handler
	{
	public:
		void* m_data = nullptr;
		LoggerMessageHandlerCallback m_callback = nullptr;

		Handler() = default;

		Handler(const Handler&) = default;

		Handler(void* data, LoggerMessageHandlerCallback callback)
			: m_data(data)
			, m_callback(callback)
		{
		}
	};

	Mutex m_mutex; ///< For thread safety
	Array<Handler, 4> m_handlers;
	U32 m_handlersCount = 0;

	static void defaultSystemMessageHandler(void*, const LoggerMessageInfo& info);
	static void fileMessageHandler(void* file, const LoggerMessageInfo& info);
};

using LoggerSingleton = Singleton<Logger>;

#define ANKI_LOG(subsystem_, t, ...) \
	do \
	{ \
		LoggerSingleton::get().writeFormated(ANKI_FILE, __LINE__, ANKI_FUNC, subsystem_, LoggerMessageType::t, \
											 Thread::getCurrentThreadId(), __VA_ARGS__); \
	} while(false);
/// @}

/// @addtogroup util_logging
/// @{

/// Log information message.
#define ANKI_LOGI(...) ANKI_LOG(nullptr, NORMAL, __VA_ARGS__)

/// Log warning message.
#define ANKI_LOGW(...) ANKI_LOG(nullptr, WARNING, __VA_ARGS__)

/// Log error message.
#define ANKI_LOGE(...) ANKI_LOG(nullptr, ERROR, __VA_ARGS__)

/// Log fatal message. It will will abort.
#define ANKI_LOGF(...) ANKI_LOG(nullptr, FATAL, __VA_ARGS__)
/// @}

} // end namespace anki

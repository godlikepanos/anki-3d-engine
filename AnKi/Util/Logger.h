// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Config.h>
#include <AnKi/Util/Singleton.h>
#include <AnKi/Util/Thread.h>
#include <format>

namespace anki {

// Forward
class File;

// Logger message type
enum class LoggerMessageType : U8
{
	kNormal,
	kVerbose,
	kError,
	kWarning,
	kFatal,
	kCount
};

inline constexpr Array<const Char*, U(LoggerMessageType::kCount)> kLoggerMessageTypeText = {"I", "V", "E", "W", "F"};

// Used as parameter when emitting the signal
class LoggerMessageInfo
{
public:
	const Char* m_file;
	I32 m_line;
	const Char* m_func;
	LoggerMessageType m_type;
	const Char* m_msg;
	const Char* m_subsystem;
	const Char* m_threadName;
};

// The message handler callback.
using LoggerMessageHandlerCallback = void (*)(void*, const LoggerMessageInfo& info);

// The logger singleton class. The logger cannot print errors or throw exceptions, it has to recover somehow. It's thread safe.
// To add a new signal: logger.addMessageHandler((void*)obj, &function)
class Logger : public MakeSingletonSimple<Logger>
{
	template<typename>
	friend class MakeSingletonSimple;

public:
	// Add a new message handler.
	void addMessageHandler(void* data, LoggerMessageHandlerCallback callback);

	// Remove a message handler.
	void removeMessageHandler(void* data, LoggerMessageHandlerCallback callback);

	// Add file message handler.
	void addFileMessageHandler(File* file)
	{
		addMessageHandler(file, fileMessageHandler);
	}

	void removeFileMessageHandler(File* file)
	{
		removeMessageHandler(file, fileMessageHandler);
	}

	// Send a message.
	void write(const Char* file, int line, const Char* func, const Char* subsystem, LoggerMessageType type, const Char* threadName, const Char* msg)
	{
		// Note: m_verbosityEnabled is not accessed in a thread-safe way. It doesn't really matter though
		if(type == LoggerMessageType::kVerbose && !m_verbosityEnabled)
		{
			return;
		}

		writeInternal(file, line, func, subsystem, type, threadName, msg);
	}

	// Send a message formatted with std::format syntax.
	template<typename... TArgs>
	void writeFormat(const Char* file, int line, const Char* func, const Char* subsystem, LoggerMessageType type, const Char* threadName,
					 std::format_string<TArgs...> fmt, TArgs&&... args)
	{
		// Note: m_verbosityEnabled is not accessed in a thread-safe way. It doesn't really matter though
		if(type == LoggerMessageType::kVerbose && !m_verbosityEnabled)
		{
			return;
		}

		// Format into a stack buffer first. format_to_n returns the full length even when it truncates
		Array<Char, 256> buffer;
		const PtrSize len = PtrSize(std::format_to_n(&buffer[0], buffer.getSize() - 1, fmt, nullSafeFormatArg(std::forward<TArgs>(args))...).size);
		if(len < buffer.getSize())
		{
			buffer[len] = '\0';
			writeInternal(file, line, func, subsystem, type, threadName, &buffer[0]);
		}
		else
		{
			// Doesn't fit the stack buffer, format again into a big enough one
			Char* newBuffer = static_cast<Char*>(malloc(len + 1));
			std::format_to(newBuffer, fmt, nullSafeFormatArg(std::forward<TArgs>(args))...);
			newBuffer[len] = '\0';
			writeInternal(file, line, func, subsystem, type, threadName, newBuffer);
			free(newBuffer);
		}
	}

	// Enable or disable logger verbosity.
	void enableVerbosity(Bool enable)
	{
		m_verbosityEnabled = enable;
	}

	Bool verbosityEnabled() const
	{
		return m_verbosityEnabled;
	}

private:
	class Handler
	{
	public:
		void* m_data = nullptr;
		LoggerMessageHandlerCallback m_callback = nullptr;
	};

	Mutex m_mutex;
	Array<Handler, 8> m_handlers;
	U32 m_handlersCount = 0;
	Bool m_verbosityEnabled = false;

	// Initialize the logger and add the default message handler
	Logger();

	~Logger();

	// A null C string crashes std::format. Swap it with "(null)", like printf does. Returns the same type so fmt still matches the arguments
	template<typename T>
	static T&& nullSafeFormatArg(T&& arg)
	{
		using U = std::remove_cvref_t<T>;
		if constexpr(std::is_same_v<U, Char*> || std::is_same_v<U, const Char*>)
		{
			if(arg == nullptr)
			{
				static U nullStr = const_cast<U>("(null)");
				return static_cast<T&&>(nullStr);
			}
		}

		return std::forward<T>(arg);
	}

	static void defaultSystemMessageHandler(void*, const LoggerMessageInfo& info);
	static void fileMessageHandler(void* file, const LoggerMessageInfo& info);

	void writeInternal(const Char* file, int line, const Char* func, const Char* subsystem, LoggerMessageType type, const Char* threadName,
					   const Char* msg);
};

#define ANKI_LOG(subsystem_, t, ...) \
	do \
	{ \
		Logger::getSingleton().writeFormat(ANKI_FILE, __LINE__, ANKI_FUNC, subsystem_, LoggerMessageType::t, Thread::getCurrentThreadName(), \
										   __VA_ARGS__); \
	} while(false)

// Log information message.
#define ANKI_LOGI(...) ANKI_LOG(nullptr, kNormal, __VA_ARGS__)

// Log verbose information message.
#define ANKI_LOGV(...) ANKI_LOG(nullptr, kVerbose, __VA_ARGS__)

// Log warning message.
#define ANKI_LOGW(...) ANKI_LOG(nullptr, kWarning, __VA_ARGS__)

// Log error message.
#define ANKI_LOGE(...) ANKI_LOG(nullptr, kError, __VA_ARGS__)

// Log fatal message. It will abort.
#define ANKI_LOGF(...) ANKI_LOG(nullptr, kFatal, __VA_ARGS__)

} // end namespace anki
